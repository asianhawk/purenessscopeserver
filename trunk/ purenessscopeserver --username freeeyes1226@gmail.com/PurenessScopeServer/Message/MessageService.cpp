// MessageService.h
// ������Ϣ���߳��࣬Connect�ὫҪ������CMessage���������Щ�߳��д�����
// ��Ϊ��Щ���£���Ϊ�����˾ͻ�������ȴ���֣�������ȴ�����ң������ģ����±����������
// add by freeeyes
// 2009-01-26

#include "MessageService.h"

Mutex_Allocator _msg_service_mb_allocator; 

CMessageService::CMessageService(void)
{
	m_u4ThreadNo      = 0;
	m_u4ThreadCount   = MAX_MSG_THREADCOUNT;
	m_u4MaxQueue      = MAX_MSG_THREADQUEUE;
	m_blRun           = false;
	m_u4HighMask      = 0;
	m_u4LowMask       = 0;
	m_u8TimeCost      = 0;
	m_u4Count         = 0;

	uint16 u2ThreadTimeOut = App_MainConfig::instance()->GetThreadTimuOut();
	if(u2ThreadTimeOut <= 0)
	{
		m_u2ThreadTimeOut = MAX_MSG_THREADTIMEOUT;
	}
	else
	{
		m_u2ThreadTimeOut = u2ThreadTimeOut;
	}
}

CMessageService::~CMessageService(void)
{
	OUR_DEBUG((LM_INFO, "[CMessageService::~CMessageService].\n"));
}

void CMessageService::Init(uint32 u4ThreadCount, uint32 u4MaxQueue, uint32 u4LowMask, uint32 u4HighMask)
{
	m_u4ThreadCount = u4ThreadCount;
	m_u4MaxQueue    = u4MaxQueue;
	m_u4HighMask    = u4HighMask;
	m_u4LowMask     = u4LowMask;

	m_u4WorkQueuePutTime = App_MainConfig::instance()->GetWorkQueuePutTime() * 1000;
}

bool CMessageService::Start()
{
	if(0 != open())
	{
		return false;
	}

	return true;
}

bool CMessageService::IsRun()
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_RunMutex);

	return m_blRun;
}

int CMessageService::open(void* args)
{
	if(args != NULL)
	{
		OUR_DEBUG((LM_INFO,"[CMessageService::open]args is not NULL.\n"));
	}

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

	uint32 u4ThreadID = (uint32)m_ThreadInfo.GetThreadCount();
	m_ThreadInfo.AddThreadInfo(u4ThreadID);

	OUR_DEBUG((LM_INFO,"[CMessageService::svc] Create WorkThread=[%d] ...\n", m_ThreadInfo.GetThreadCount()));

	//��ȡ�̵߳�ϵͳID
	_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(u4ThreadID);
	if(NULL == pThreadInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::svc] pThreadInfo[%d] is not find.\n", u4ThreadID));
		return true;
	}

	if(pThreadInfo->m_u4ThreadIndex == 0)
	{
		pThreadInfo->m_u4ThreadIndex = ACE_OS::thr_self();
	}

	ACE_OS::sleep(1);

	while(IsRun())
	{
		mb = NULL;
		//xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(getq(mb, 0) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::svc] PutMessage error errno = [%d].\n", errno));
			m_blRun = false;
			break;
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

		ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, m_u4WorkQueuePutTime);
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
	CProfileTime DisposeTime;
	//uint32 u4Cost = (uint32)(pMessage->GetMessageBase()->m_ProfileTime.Stop());

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

	//��Ҫ���������ݷŵ��߼������ĵط�ȥ
	uint16 u2CommandID = 0;          //���ݰ���CommandID

	u2CommandID = pMessage->GetMessageBase()->m_u2Cmd;

	DisposeTime.Start();
	App_MessageManager::instance()->DoMessage(pMessage, u2CommandID);

	pThreadInfo->m_u4State = THREAD_RUNEND;

	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		pThreadInfo->m_u2CommandID   = u2CommandID;
	}

	//�����windows��������Ĭ����App_ProConnectManager������������Ҫ�ֶ��޸�һ��
	//��ʱ���ã���ע�͵�
	/*
	#ifdef WIN32
	{
	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
	//��ʵ�о��������岻����Ϊ��windows�µ�ʱ�Ӿ��ȵ�����΢�룬�������ֵ���п���û�����塣
	App_ProConnectManager::instance()->SetRecvQueueTimeCost(pMessage->GetMessageBase()->m_u4ConnectID, u4Cost);
	}
	}
	#else
	{
	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
	//linux���Ծ�ȷ��΢�룬���ֵ�ͱ����������
	App_ConnectManager::instance()->SetRecvQueueTimeCost(pMessage->GetMessageBase()->m_u4ConnectID, u4Cost);
	}
	}
	#endif
	*/

	//��ʼ�������ݰ�������ʱ��
	uint64 u8DisposeCost = DisposeTime.Stop();

	uint16 u2DisposeTime = (uint16)(u8DisposeCost / 1000000);
	if(pThreadInfo->m_u2PacketTime == 0)
	{
		pThreadInfo->m_u2PacketTime = u2DisposeTime;
	}
	else
	{
		//�������ݰ���ƽ������ʱ��
		pThreadInfo->m_u2PacketTime = (uint16)((pThreadInfo->m_u2PacketTime + u2DisposeTime)/2);
	}

	return true;
}

int CMessageService::Close()
{
	m_blRun = false;
	msg_queue()->deactivate();
	msg_queue()->flush();
	OUR_DEBUG((LM_INFO, "[CMessageService::close] Close().\n"));
	return 0;
}

bool CMessageService::SaveThreadInfo()
{
	return SaveThreadInfoData();
}

bool CMessageService::SaveThreadInfoData()
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
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] TimeOut > %d[%d] CurrPacketCount = [%d] QueueCount = [%d] BuffPacketUsed = [%d] BuffPacketFree = [%d].", 
					pThreadInfo->m_u4ThreadID,
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime,
					m_u2ThreadTimeOut,
					tvNow.sec() - pThreadInfo->m_tvUpdateTime.sec(),
					pThreadInfo->m_u4CurrPacketCount,
					(int)msg_queue()->message_count(),
					App_BuffPacketManager::instance()->GetBuffPacketUsedCount(),
					App_BuffPacketManager::instance()->GetBuffPacketFreeCount());

				//���������̣߳���Ҫ������Ӧ���߳�
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout]  pThreadInfo = [%d] ThreadID = [%d] Thread is reset.", pThreadInfo->m_u4ThreadID, pThreadInfo->m_u4ThreadIndex);
				return false;
			}
			else
			{
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] CurrPacketCount = [%d] QueueCount = [%d] BuffPacketUsed = [%d] BuffPacketFree = [%d].", 
					pThreadInfo->m_u4ThreadID, 
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime,
					pThreadInfo->m_u4CurrPacketCount,
					(int)msg_queue()->message_count(),
					App_BuffPacketManager::instance()->GetBuffPacketUsedCount(),
					App_BuffPacketManager::instance()->GetBuffPacketFreeCount());

				pThreadInfo->m_u4CurrPacketCount = 0;
				return true;
			}
		}
		else
		{
			OUR_DEBUG((LM_INFO,"[CMessageService::handle_timeout] pThreadInfo = [%d] Not find.\n", i));
		}
	}

	return true;
}

CThreadInfo* CMessageService::GetThreadInfo()
{
	return &m_ThreadInfo;
}


CMessageServiceGroup::CMessageServiceGroup( void )
{
	m_u4TimerID = 0;

	uint16 u2ThreadTimeCheck = App_MainConfig::instance()->GetThreadTimeCheck();
	if(u2ThreadTimeCheck <= 0)
	{
		m_u2ThreadTimeCheck = MAX_MSG_TIMEDELAYTIME;
	}
	else
	{
		m_u2ThreadTimeCheck = u2ThreadTimeCheck;
	}
}

CMessageServiceGroup::~CMessageServiceGroup( void )
{

}

int CMessageServiceGroup::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	if(arg != NULL)
	{
		OUR_DEBUG((LM_ERROR,"[CMessageServiceGroup::handle_timeout] arg is not NULL, time is (%d).\n", tv.sec()));
	}

	for(uint32 i = 0; i < (uint32)m_vecMessageService.size(); i++)
	{
		CMessageService* pMessageService = (CMessageService* )m_vecMessageService[i];
		if(NULL != pMessageService)
		{
			bool blFlag = pMessageService->SaveThreadInfo();
			if(false == blFlag)
			{
				//��Ҫ���������̣߳��ȹرյ�ǰ�Ĺ����߳�
				pMessageService->Close();
				delete pMessageService;

				//����һ���µĹ����̣߳�����ֵ��ȥ
				pMessageService = new CMessageService();
				if(NULL != pMessageService)
				{
					pMessageService->Init(1, m_u4MaxQueue, m_u4LowMask, m_u4HighMask);
					pMessageService->Start();

					m_vecMessageService[i] = pMessageService;

				}
				else
				{
					OUR_DEBUG((LM_ERROR,"[CMessageServiceGroup::handle_timeout] reset workthread is NULL (%d).\n", i));
				}
			}
		}
	}

	//��¼PacketParse��ͳ�ƹ���
	AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMessageService::handle_timeout] UsedCount = %d, FreeCount= %d.", App_PacketParsePool::instance()->GetUsedCount(), App_PacketParsePool::instance()->GetFreeCount());

	if(App_MainConfig::instance()->GetMonitor() == 1)
	{
#ifdef WIN32
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_MONITOR, "[Monitor] CPU Rote=%d,Memory=%d.", GetProcessCPU_Idel(), GetProcessMemorySize());
#else
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_MONITOR, "[Monitor] CPU Rote=%d,Memory=%d.", GetProcessCPU_Idel_Linux(), GetProcessMemorySize_Linux());
#endif
	}

	return 0;
}

bool CMessageServiceGroup::Init(uint32 u4ThreadCount, uint32 u4MaxQueue, uint32 u4LowMask, uint32 u4HighMask)
{
	//ɾ����ǰ������CMessageService����
	Close();

	//��¼��ǰ����
	m_u4MaxQueue = u4MaxQueue;
	m_u4HighMask = u4HighMask;
	m_u4LowMask  = u4LowMask;

	//��ʼ�����е�Message����
	for(uint32 i = 0; i < u4ThreadCount; i++)
	{
		CMessageService* pMessageService = new CMessageService();
		if(NULL == pMessageService)
		{
			OUR_DEBUG((LM_ERROR, "[CMessageServiceGroup::Init](%d)pMessageService is NULL.\n", i));
			return false;
		}
		pMessageService->Init(1, u4MaxQueue, u4LowMask, u4HighMask);
		m_vecMessageService.push_back(pMessageService);
	}

	return true;
}

bool CMessageServiceGroup::PutMessage(CMessage* pMessage)
{
	uint32 u4ThreadID = pMessage->GetMessageBase()->m_u4ConnectID % (uint32)m_vecMessageService.size();
	CMessageService* pMessageService = (CMessageService* )m_vecMessageService[u4ThreadID];
	if(NULL != pMessageService)
	{
		pMessageService->PutMessage(pMessage);
	}

	return true;
}

void CMessageServiceGroup::Close()
{
	KillTimer();

	for(uint32 i = 0; i < (uint32)m_vecMessageService.size(); i++)
	{
		CMessageService* pMessageService = (CMessageService* )m_vecMessageService[i];
		if(NULL != pMessageService)
		{
			pMessageService->Close();
		}
	}

	m_vecMessageService.clear();
}

bool CMessageServiceGroup::Start()
{
	if(StartTimer() == false)
	{
		return false;
	}

	for(uint32 i = 0; i < (uint32)m_vecMessageService.size(); i++)
	{
		CMessageService* pMessageService = (CMessageService* )m_vecMessageService[i];
		if(NULL != pMessageService)
		{
			pMessageService->Start();
		}

		OUR_DEBUG((LM_INFO,"[CMessageServiceGroup::Start](%d)WorkThread is OK.\n", i));
	}

	return true;
}

bool CMessageServiceGroup::StartTimer()
{
	OUR_DEBUG((LM_ERROR, "[CMessageService::StartTimer] begin....\n"));

	m_u4TimerID = App_TimerManager::instance()->schedule(this, NULL, ACE_OS::gettimeofday() + ACE_Time_Value(MAX_MSG_STARTTIME), ACE_Time_Value(m_u2ThreadTimeCheck));
	if(0 == m_u4TimerID)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageService::StartTimer] Start thread time error.\n"));
		return false;
	}

	return true;
}

bool CMessageServiceGroup::KillTimer()
{
	if(m_u4TimerID > 0)
	{
		App_TimerManager::instance()->cancel(m_u4TimerID);
	}
	return true;
}

CThreadInfo* CMessageServiceGroup::GetThreadInfo()
{
	m_objAllThreadInfo.Close();

	for(uint32 i = 0; i < (uint32)m_vecMessageService.size(); i++)
	{
		CMessageService* pMessageService = (CMessageService* )m_vecMessageService[i];
		if(NULL != pMessageService)
		{
			CThreadInfo* pThreadInfo = pMessageService->GetThreadInfo();
			if(NULL != pThreadInfo)
			{
				_ThreadInfo* pThreadInfoData = pThreadInfo->GetThreadInfo();
				m_objAllThreadInfo.AddThreadInfo(i, pThreadInfoData);
			}
		}
	}

	return &m_objAllThreadInfo;
}