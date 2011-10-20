// MessageService.h
// ������Ϣ���߳��࣬Connect�ὫҪ�����CMessage���������Щ�߳��д���
// ��Ϊ��Щ���£���Ϊ�����˾ͻ�������ȴ���֣������ȴ�����ң������ģ����±����������
// add by freeeyes
// 2009-01-26

#include "MessageService.h"

Mutex_Allocator _msg_service_mb_allocator; 

CMessageService::CMessageService(void)
{
	m_u4ThreadNo      = 0;
	m_u4ThreadCount   = MAX_MSG_THREADCOUNT;
	m_u4MaxQueue      = MAX_MSG_THREADQUEUE;
	m_u4TimerID       = 0;
	m_blRun           = false;
	m_u4HighMask      = 0;
	m_u4LowMask       = 0;

	uint16 u2ThreadTimeOut = App_MainConfig::instance()->GetThreadTimuOut();
	if(u2ThreadTimeOut <= 0)
	{
		m_u2ThreadTimeOut = MAX_MSG_THREADTIMEOUT;
	}
	else
	{
		m_u2ThreadTimeOut = u2ThreadTimeOut;
	}

	uint16 u2ThreadTimeCheck = App_MainConfig::instance()->GetThreadTimeCheck();
	if(u2ThreadTimeOut <= 0)
	{
		m_u2ThreadTimeCheck = MAX_MSG_TIMEDELAYTIME;
	}
	else
	{
		m_u2ThreadTimeCheck = u2ThreadTimeCheck;
	}

	uint16 u2PacketTimeOut = App_MainConfig::instance()->GetPacketTimeOut();
	if(u2PacketTimeOut <= 0)
	{
		m_u2PacketTimeOut = MAX_MSG_PACKETTIMEOUT;
	}
	else
	{
		m_u2PacketTimeOut = u2PacketTimeOut;
	}

}

CMessageService::~CMessageService(void)
{
}

void CMessageService::Init(uint32 u4ThreadCount, uint32 u4MaxQueue, uint32 u4LowMask, uint32 u4HighMask)
{
	m_u4ThreadCount = u4ThreadCount;
	m_u4MaxQueue    = u4MaxQueue;
	m_u4HighMask    = u4HighMask;
	m_u4LowMask     = u4LowMask;
}

bool CMessageService::Start()
{
	if(false == StartTimer())
	{
		return false;
	}

	if(0 != open())
	{
		return false;
	}

	return true;
}

bool CMessageService::IsRun()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_RunMutex);

	return m_blRun;
}

int CMessageService::open(void* args)
{
	m_blRun = true;
	msg_queue()->high_water_mark(m_u4HighMask);
	msg_queue()->low_water_mark(m_u4LowMask);

	OUR_DEBUG((LM_INFO,"[CMessageService::open] m_u4HighMask = [%d] m_u4LowMask = [%d]\n", m_u4HighMask, m_u4LowMask));
	if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, m_u4ThreadCount) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageService::open] activate error ThreadCount = [%d].", m_u4ThreadCount));
		m_blRun = false;
		return -1;
	}

	resume();

	return 0;
}

int CMessageService::svc(void)
{
	uint32 u4threadCurNo = 0;
	ACE_Message_Block* mb = NULL;
	ACE_Time_Value xtime;

	m_rwMutex.acquire_write();
	uint32 u4ThreadID = (uint32)m_ThreadInfo.GetThreadCount();
	m_ThreadInfo.AddThreadInfo(u4ThreadID);
	
	OUR_DEBUG((LM_INFO,"[CMessageService::svc] Create WorkThread=[%d] ...\n", m_ThreadInfo.GetThreadCount()));

	//��ȡ�̵߳�ϵͳID
	_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(u4ThreadID);
	if(NULL == pThreadInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::svc] pThreadInfo[%d] is not find.\n", u4ThreadID));
		m_rwMutex.release();
		return true;
	}

	if(pThreadInfo->m_u4ThreadIndex == 0)
	{
		pThreadInfo->m_u4ThreadIndex = ACE_OS::thr_self();
	}

	m_rwMutex.release();

	ACE_OS::sleep(1);

	while(IsRun())
	{
		mb = NULL;
		//xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(getq(mb, 0) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::PutMessage] getq error errno = [%d].\n", errno));
			continue;
		}
		if (mb == NULL)
		{
			continue;
		}
		CMessage* msg = *((CMessage**)mb->base());
		if (! msg)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::svc] mb msg == NULL CurrthreadNo=[%d]!\n", u4threadCurNo));
			mb->release();
			continue;
		}

		this->ProcessMessage(msg, u4ThreadID);
		mb->release();
	}

	OUR_DEBUG((LM_INFO,"[CMessageService::svc] svc finish!\n"));
	return 0;
}

bool CMessageService::PutMessage(CMessage* pMessage)
{
	ACE_Message_Block* mb = NULL;

	ACE_NEW_MALLOC_NORETURN(mb, 
		static_cast<ACE_Message_Block*>( _msg_service_mb_allocator.malloc(sizeof(ACE_Message_Block))),
		ACE_Message_Block(sizeof(CMessage*), // size
		ACE_Message_Block::MB_DATA, // type
		0,
		0,
		&_msg_service_mb_allocator, // allocator_strategy
		0, // locking strategy
		ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
		ACE_Time_Value::zero,
		ACE_Time_Value::max_time,
		&_msg_service_mb_allocator,
		&_msg_service_mb_allocator
		));

	if(NULL != mb)
	{
		CMessage** ppMessage = (CMessage **)mb->base();
		*ppMessage = pMessage;

		//�ж϶����Ƿ����Ѿ����
		int nQueueCount = (int)msg_queue()->message_count();
		if(nQueueCount >= (int)m_u4MaxQueue)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));
			mb->release();
			return false;
		}

		ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(this->putq(mb, &xtime) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
			mb->release();
			return false;
		}
	}
	else
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::PutMessage] mb new error.\n"));
		return false;
	}

	return true;
}

bool CMessageService::ProcessMessage(CMessage* pMessage, uint32 u4ThreadID)
{
	if(NULL == pMessage)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] pMessage is NULL.\n"));
		return false;
	}

	if(NULL == pMessage->GetMessageBase())
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] pMessage->GetMessageBase() is NULL.\n"));
		App_MessagePool::instance()->Delete(pMessage);
		return false;
	}

	//����������߳��Լ����
	_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(u4ThreadID);
	if(NULL == pThreadInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] pThreadInfo[%d] is not find.\n", u4ThreadID));
		App_MessagePool::instance()->Delete(pMessage);
		return false;
	}

	pThreadInfo->m_tvUpdateTime = ACE_OS::gettimeofday();
	pThreadInfo->m_u4State = THREAD_RUNBEGIN;

	//�׳������ӽ����ͶϿ���ֻ�����߼����ݰ�
	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		pThreadInfo->m_u4RecvPacketCount++;
		pThreadInfo->m_u4CurrPacketCount++;
	}

	//��Ҫ��������ݷŵ��߼�����ĵط�ȥ
	uint32 u4Len       = 0;          //���ݰ��ĳ���
	uint16 u2CommandID = 0;          //���ݰ���CommandID

	_MessageBase* pMessageBase = pMessage->GetMessageBase();
	if(NULL == pMessageBase)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] pMessageBase is NULL.\n", u4ThreadID));
		App_MessagePool::instance()->Delete(pMessage);
		return false;
	}
	else
	{
		u2CommandID = pMessageBase->m_u2Cmd;
	}

	ACE_Time_Value tvDisposeBegin(ACE_OS::gettimeofday());
	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		pThreadInfo->m_u2CommandID   = u2CommandID;
	}

	App_MessageManager::instance()->DoMessage(pMessage, u4Len, u2CommandID);
	pThreadInfo->m_u4State = THREAD_RUNEND;

	//��ʼ�������ݰ������ʱ��
	ACE_Time_Value tvDisposeEnd(ACE_OS::gettimeofday());
	ACE_Time_Value tvDisposeTime(tvDisposeEnd - tvDisposeBegin);

	uint16 u2DisposeTime = (uint16)tvDisposeTime.msec();
	if(pThreadInfo->m_u2PacketTime == 0)
	{
		pThreadInfo->m_u2PacketTime = u2DisposeTime;
	}
	else
	{
		//�������ݰ���ƽ������ʱ��
		pThreadInfo->m_u2PacketTime = (uint16)((pThreadInfo->m_u2PacketTime + u2DisposeTime)/2);
	}


	//�鿴���ݰ������Ƿ�ʱ
	if(u2DisposeTime > m_u2PacketTimeOut)
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTIME, "[PacketTimeout]CommandID = [0x%x] CheckTime = [%d] < DisposeTime = [%d].", u2CommandID, m_u2PacketTimeOut, u2DisposeTime);
	}

	return true;
}

int CMessageService::Close()
{
	KillTimer();
	m_blRun = false;
	msg_queue()->deactivate();
	msg_queue()->flush();
	OUR_DEBUG((LM_INFO, "[CMessageService::close] Close().\n"));
	return 0;
}

int CMessageService::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	return SaveThreadInfoData();
}

int CMessageService::SaveThreadInfoData()
{
	//��������߳��Լ�
	for(uint32 i = 0; i < m_u4ThreadCount; i++)
	{
		_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(i);
		if(NULL == pThreadInfo)
		{
			continue;
		}

		ACE_Time_Value tvNow(ACE_OS::gettimeofday());
		ACE_Date_Time dt(pThreadInfo->m_tvUpdateTime);
		if(NULL != pThreadInfo)
		{
			//��ʼ�鿴�߳��Ƿ�ʱ
			if(pThreadInfo->m_u4State == THREAD_RUNBEGIN && tvNow.sec() - pThreadInfo->m_tvUpdateTime.sec() > m_u2ThreadTimeOut)
			{
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] TimeOut > %d[%d] CurrPacketCount = [%d] QueueCount = [%d].", 
					pThreadInfo->m_u4ThreadID,
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime,
					m_u2ThreadTimeOut,
					tvNow.sec() - pThreadInfo->m_tvUpdateTime.sec(),
					pThreadInfo->m_u4CurrPacketCount,
					(int)msg_queue()->message_count());

				//���������̣߳�������Ӧ���߳�
				ACE_OS::thr_kill(pThreadInfo->m_u4ThreadIndex, SIGALRM);
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout]  pThreadInfo = [%d] ThreadID = [%d] Thread is reset.", pThreadInfo->m_u4ThreadID, pThreadInfo->m_u4ThreadIndex);
				m_ThreadInfo.CloseThread(pThreadInfo->m_u4ThreadID);
				ResumeThread(1);
			}
			else
			{
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] CurrPacketCount = [%d] QueueCount = [%d].", 
					pThreadInfo->m_u4ThreadID, 
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime,
					pThreadInfo->m_u4CurrPacketCount,
					(int)msg_queue()->message_count());
			}
			pThreadInfo->m_u4CurrPacketCount = 0;
		}
		else
		{
			OUR_DEBUG((LM_INFO,"[CMessageService::handle_timeout] pThreadInfo = [%d] Not find.\n", i));
		}
	}

	return 0;
}

bool CMessageService::ResumeThread(int nThreadCount)
{
	ACE_Thread::spawn_n(1, &ACE_Task_Base::svc_run, (void *)this);

	return true;
}

bool CMessageService::StartTimer()
{
	OUR_DEBUG((LM_ERROR, "CMessageService::StartTimer()-->begin....\n"));

	m_u4TimerID = App_TimerManager::instance()->schedule(this, NULL, ACE_OS::gettimeofday() + ACE_Time_Value(MAX_MSG_STARTTIME), ACE_Time_Value(m_u2ThreadTimeCheck));
	if(0 == m_u4TimerID)
	{
		OUR_DEBUG((LM_ERROR, "CMessageService::StartTimer()--> Start thread time error.\n"));
		return false;
	}

	return true;
}

bool CMessageService::KillTimer()
{
	if(m_u4TimerID > 0)
	{
		App_TimerManager::instance()->cancel(m_u4TimerID);
	}
	return true;
}

CThreadInfo* CMessageService::GetThreadInfo()
{
	return &m_ThreadInfo;
}

