// MessageService.h
// ������Ϣ���߳��࣬Connect�ὫҪ�����CMessage���������Щ�߳��д���
// ��Ϊ��Щ���£���Ϊ�����˾ͻ�������ȴ���֣������ȴ�����ң������ģ����±����������
// add by freeeyes
// 2009-01-26

#include "MessageService.h"

Mutex_Allocator _msg_service_mb_allocator; 

CMessageService::CMessageService(void)
{
	m_u4ThreadID      = 0;
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

void CMessageService::Init(uint32 u4ThreadID, uint32 u4MaxQueue, uint32 u4LowMask, uint32 u4HighMask)
{
	m_u4MaxQueue    = u4MaxQueue;
	m_u4HighMask    = u4HighMask;
	m_u4LowMask     = u4LowMask;

	//OUR_DEBUG((LM_INFO, "[CMessageService::Init]ID=%d,m_u4State=%d.\n", m_u4ThreadID = u4ThreadID, m_ThreadInfo.m_u4State));

	//����߳���Ϣ
	m_u4ThreadID = u4ThreadID;
	m_ThreadInfo.m_u4ThreadID   = u4ThreadID;

	m_u4WorkQueuePutTime = App_MainConfig::instance()->GetWorkQueuePutTime() * 1000;
	
	//��ʼ���߳�AI
	m_WorkThreadAI.Init(App_MainConfig::instance()->GetWTAI(), 
		App_MainConfig::instance()->GetPacketTimeOut(),
		App_MainConfig::instance()->GetWTCheckTime(),
		App_MainConfig::instance()->GetWTTimeoutCount(),
		App_MainConfig::instance()->GetWTStopTime(),
		App_MainConfig::instance()->GetWTReturnDataType(),
		App_MainConfig::instance()->GetWTReturnData());
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
	if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, MAX_MSG_THREADCOUNT) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageService::open] activate error ThreadCount = [%d].", MAX_MSG_THREADCOUNT));
		m_blRun = false;
		return -1;
	}

	resume();

	return 0;
}

int CMessageService::svc(void)
{
	ACE_Message_Block* mb = NULL;

	//��΢��Ϣһ�£���һ�������߳�������ѭ��
	ACE_Time_Value tvSleep(0, MAX_MSG_SENDCHECKTIME*MAX_BUFF_1000);
	ACE_OS::sleep(tvSleep);

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
			OUR_DEBUG((LM_ERROR,"[CMessageService::svc] mb msg == NULL CurrthreadNo=[%d]!\n", m_u4ThreadID));
			mb->release();
			continue;
		}

		this->ProcessMessage(msg, m_u4ThreadID);
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
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] [%d]pMessage is NULL.\n", u4ThreadID));
		return false;
	}

	if(NULL == pMessage->GetMessageBase())
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage] [%d]pMessage->GetMessageBase() is NULL.\n", u4ThreadID));
		App_MessagePool::instance()->Delete(pMessage);
		return false;
	}

	//����������߳��Լ����
	m_ThreadInfo.m_tvUpdateTime = ACE_OS::gettimeofday();
	m_ThreadInfo.m_u4State = THREAD_RUNBEGIN;
	//OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage]1 [%d],m_u4State=%d, commandID=%d.\n", u4ThreadID, m_ThreadInfo.m_u4State,  pMessage->GetMessageBase()->m_u2Cmd));

	//�׳������ӽ����ͶϿ���ֻ�����߼����ݰ�
	bool blIsDead = false;
	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		m_ThreadInfo.m_u4RecvPacketCount++;
		m_ThreadInfo.m_u4CurrPacketCount++;

		blIsDead = m_WorkThreadAI.CheckCurrTimeout(pMessage->GetMessageBase()->m_u2Cmd, (uint32)m_ThreadInfo.m_tvUpdateTime.sec());
		if(blIsDead == true)
		{
			OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage]Command(%d) is Delele.\n", pMessage->GetMessageBase()->m_u2Cmd));
			//ֱ�ӷ���Ӧ�����ݸ��ͻ��ˣ����ڵ��߼���ȥ����
#ifdef WIN32
			App_ProConnectManager::instance()->PostMessage(pMessage->GetMessageBase()->m_u4ConnectID,
														   m_WorkThreadAI.GetReturnData(),
														   m_WorkThreadAI.GetReturnDataLength(), 
														   SENDMESSAGE_NOMAL,
														   (uint16)COMMAND_RETURN_BUSY, 
														   PACKET_SEND_IMMEDIATLY,
														   PACKET_IS_SELF_RECYC);
#else
			App_ConnectManager::instance()->PostMessage(pMessage->GetMessageBase()->m_u4ConnectID,
														m_WorkThreadAI.GetReturnData(),
														m_WorkThreadAI.GetReturnDataLength(), 
														SENDMESSAGE_NOMAL,
														(uint16)COMMAND_RETURN_BUSY, 
														PACKET_SEND_IMMEDIATLY,
														PACKET_IS_SELF_RECYC);
#endif
			App_MessagePool::instance()->Delete(pMessage);
			return true;
		}
	}

	//��Ҫ��������ݷŵ��߼�����ĵط�ȥ
	uint16 u2CommandID = 0;          //���ݰ���CommandID

	u2CommandID = pMessage->GetMessageBase()->m_u2Cmd;

	DisposeTime.Start();

	//�ڰ������ù����߳�ID
	pMessage->GetMessageBase()->m_u4WorkThreadID = m_u4ThreadID;
	uint32 u4TimeCost = 0;
	App_MessageManager::instance()->DoMessage(pMessage, u2CommandID, u4TimeCost);

	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		m_WorkThreadAI.SaveTimeout(pMessage->GetMessageBase()->m_u2Cmd, u4TimeCost);
	}

	m_ThreadInfo.m_u4State = THREAD_RUNEND;
	//OUR_DEBUG((LM_ERROR,"[CMessageService::ProcessMessage]2 [%d],m_u4State=%d,CommandID=%d.\n", u4ThreadID, m_ThreadInfo.m_u4State, pMessage->GetMessageBase()->m_u2Cmd));

	if(pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CONNECT && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_CDISCONNET && pMessage->GetMessageBase()->m_u2Cmd != CLIENT_LINK_SDISCONNET)
	{
		m_ThreadInfo.m_u2CommandID   = u2CommandID;
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

	//��ʼ�������ݰ������ʱ��
	uint64 u8DisposeCost = DisposeTime.Stop();

	uint16 u2DisposeTime = (uint16)(u8DisposeCost / 1000000);
	if(m_ThreadInfo.m_u2PacketTime == 0)
	{
		m_ThreadInfo.m_u2PacketTime = u2DisposeTime;
	}
	else
	{
		//�������ݰ���ƽ������ʱ��
		m_ThreadInfo.m_u2PacketTime = (uint16)((m_ThreadInfo.m_u2PacketTime + u2DisposeTime)/2);
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
	ACE_Time_Value tvNow(ACE_OS::gettimeofday());
	ACE_Date_Time dt(m_ThreadInfo.m_tvUpdateTime);

	//��ʼ�鿴�߳��Ƿ�ʱ
	//OUR_DEBUG((LM_INFO, "[CMessageService::SaveThreadInfoData]ID=%d,m_u4State=%d,m_u2ThreadTimeOut=%d,cost=%d.\n", m_ThreadInfo.m_u4ThreadID, m_ThreadInfo.m_u4State, m_u2ThreadTimeOut, tvNow.sec() - m_ThreadInfo.m_tvUpdateTime.sec()));
	if(m_ThreadInfo.m_u4State == THREAD_RUNBEGIN && tvNow.sec() - m_ThreadInfo.m_tvUpdateTime.sec() > m_u2ThreadTimeOut)
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] TimeOut > %d[%d] CurrPacketCount = [%d] QueueCount = [%d] BuffPacketUsed = [%d] BuffPacketFree = [%d].", 
			m_ThreadInfo.m_u4ThreadID,
			m_ThreadInfo.m_u4State, 
			dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
			m_ThreadInfo.m_u4RecvPacketCount,
			m_ThreadInfo.m_u2CommandID,
			m_ThreadInfo.m_u2PacketTime,
			m_u2ThreadTimeOut,
			tvNow.sec() - m_ThreadInfo.m_tvUpdateTime.sec(),
			m_ThreadInfo.m_u4CurrPacketCount,
			(int)msg_queue()->message_count(),
			App_BuffPacketManager::instance()->GetBuffPacketUsedCount(),
			App_BuffPacketManager::instance()->GetBuffPacketFreeCount());

		//���������̣߳���Ҫ������Ӧ���߳�
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] ThreadID = [%d] Thread is reset.", m_u4ThreadID);
		return false;
	}
	else
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_WORKTHREAD, "[CMessageService::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] CurrPacketCount = [%d] QueueCount = [%d] BuffPacketUsed = [%d] BuffPacketFree = [%d].", 
			m_ThreadInfo.m_u4ThreadID, 
			m_ThreadInfo.m_u4State, 
			dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
			m_ThreadInfo.m_u4RecvPacketCount,
			m_ThreadInfo.m_u2CommandID,
			m_ThreadInfo.m_u2PacketTime,
			m_ThreadInfo.m_u4CurrPacketCount,
			(int)msg_queue()->message_count(),
			App_BuffPacketManager::instance()->GetBuffPacketUsedCount(),
			App_BuffPacketManager::instance()->GetBuffPacketFreeCount());

		m_ThreadInfo.m_u4CurrPacketCount = 0;
		return true;
		}

	return true;
}

_ThreadInfo* CMessageService::GetThreadInfo()
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
				//�����ö���ʧЧ
				//m_ThreadWriteLock.acquire();
				m_vecMessageService[i] = NULL;
				//m_ThreadWriteLock.release();

				//��õ�ǰ�߳�ID
				uint32 u4ThreadID = pMessageService->GetThreadInfo()->m_u4ThreadID;

				//ɱ����ǰ�����߳�
#ifdef WIN32
				ACE_hthread_t hthread = 0; 
				int grp_id = pMessageService->grp_id(); 
				if (ACE_Thread_Manager::instance()->hthread_grp_list(grp_id, &hthread, 1) == 1)
				{
					int ret = ::TerminateThread (hthread, -1); 
					ACE_Thread_Manager::instance()->wait_grp (grp_id); 
					OUR_DEBUG((LM_DEBUG, "[CMessageServiceGroup::handle_timeout]kill return %d, %d\n", ret, GetLastError())); 
				}
#else
				int grp_id = pMessageService->grp_id(); 
				int ret = ACE_Thread_Manager::instance()->kill_grp(grp_id, SIGUSR1);
				OUR_DEBUG((LM_DEBUG, "[CMessageServiceGroup::handle_timeout]kill return %d OK.\n", ret)); 
#endif

				//��Ҫ���������̣߳��ȹرյ�ǰ�Ĺ����߳�
				pMessageService->Close();
				delete pMessageService;

				//����һ���µĹ����̣߳�����ֵ��ȥ
				pMessageService = new CMessageService();
				if(NULL != pMessageService)
				{
					pMessageService->Init(u4ThreadID, m_u4MaxQueue, m_u4LowMask, m_u4HighMask);
					pMessageService->Start();

					//m_ThreadWriteLock.acquire();
					m_vecMessageService[i] = pMessageService;
					//m_ThreadWriteLock.release();

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
		uint32 u4CurrCpu    = (uint32)GetProcessCPU_Idel();
		uint32 u4CurrMemory = (uint32)GetProcessMemorySize();
#else
		uint32 u4CurrCpu    = (uint32)GetProcessCPU_Idel_Linux();
		uint32 u4CurrMemory = (uint32)GetProcessMemorySize_Linux();
#endif
		if(u4CurrCpu > App_MainConfig::instance()->GetCpuMax() || u4CurrMemory > App_MainConfig::instance()->GetMemoryMax())
		{
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_MONITOR, "[Monitor] CPU Rote=%d,Memory=%d.", u4CurrCpu, u4CurrMemory);
		}

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
		pMessageService->Init(i, u4MaxQueue, u4LowMask, u4HighMask);
		m_vecMessageService.push_back(pMessageService);
	}

	//��ʼ�������Χ����UDPЭ�����Ҫ
	m_objRandomNumber.SetRange(0, u4ThreadCount - 1);

	return true;
}

bool CMessageServiceGroup::PutMessage(CMessage* pMessage)
{
	//�ж��Ƿ�ΪTCP�������������ConnectID���֡�UDP���������һ��
	uint32 u4ThreadID = 0;

	if(pMessage->GetMessageBase()->m_u1PacketType == PACKET_TCP)
	{
		u4ThreadID = pMessage->GetMessageBase()->m_u4ConnectID % (uint32)m_vecMessageService.size();
	}
	else
	{
		//�����UDPЭ�飬���ȡ��ǰ�����ֵ
		u4ThreadID = m_objRandomNumber.GetRandom();
	}
	
	//m_ThreadWriteLock.acquire();
	CMessageService* pMessageService = (CMessageService* )m_vecMessageService[u4ThreadID];
	if(NULL != pMessageService)
	{
		pMessageService->PutMessage(pMessage);
	}
	//m_ThreadWriteLock.release();

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
			_ThreadInfo* pThreadInfo = pMessageService->GetThreadInfo();
			if(NULL != pThreadInfo)
			{
				m_objAllThreadInfo.AddThreadInfo(i, pThreadInfo);
			}
		}
	}

	return &m_objAllThreadInfo;
}

uint32 CMessageServiceGroup::GetWorkThreadCount()
{
	return (uint32)m_vecMessageService.size();
}

uint32 CMessageServiceGroup::GetWorkThreadIDByIndex(uint32 u4Index)
{
	if(u4Index >= m_vecMessageService.size())
	{
		return (uint32)0;
	}
	else
	{
		return m_vecMessageService[u4Index]->GetThreadInfo()->m_u4ThreadID;
	}
}

