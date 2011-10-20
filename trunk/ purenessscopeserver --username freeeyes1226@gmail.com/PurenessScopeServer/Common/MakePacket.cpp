#include "MakePacket.h"


CMakePacketPool::CMakePacketPool()
{
}

CMakePacketPool::~CMakePacketPool()
{
	Close();
}

void CMakePacketPool::Init(uint32 u4PacketCount)
{
	Close();

	for(int i = 0; i < (int)u4PacketCount; i++)
	{
		_MakePacket* pPacket = new _MakePacket();
		if(NULL != pPacket)
		{
			//添加到Free map里面
			mapPacket::iterator f = m_mapPacketFree.find(pPacket);
			if(f == m_mapPacketFree.end())
			{
				m_mapPacketFree.insert(mapPacket::value_type(pPacket, pPacket));
			}
		}
	}
}

void CMakePacketPool::Close()
{
	//清理所有已存在的指针
	mapPacket::iterator itorFreeB = m_mapPacketFree.begin();
	mapPacket::iterator itorFreeE = m_mapPacketFree.end();

	for(itorFreeB; itorFreeB != itorFreeE; itorFreeB++)
	{
		_MakePacket* pPacket = (_MakePacket* )itorFreeB->second;
		SAFE_DELETE(pPacket);
	}

	mapPacket::iterator itorUsedB = m_mapPacketUsed.begin();
	mapPacket::iterator itorUsedE = m_mapPacketUsed.end();

	for(itorUsedB; itorUsedB != itorUsedE; itorUsedB++)
	{
		_MakePacket* pPacket = (_MakePacket* )itorUsedB->second;
		SAFE_DELETE(pPacket);
	}

	m_mapPacketFree.clear();
	m_mapPacketUsed.clear();
}

int CMakePacketPool::GetUsedCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapPacketUsed.size();
}

int CMakePacketPool::GetFreeCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapPacketFree.size();
}

_MakePacket* CMakePacketPool::Create()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	//如果free池中已经没有了，则添加到free池中。
	if(m_mapPacketFree.size() <= 0)
	{
		_MakePacket* pPacket = new _MakePacket();

		if(pPacket != NULL)
		{
			//添加到Free map里面
			mapPacket::iterator f = m_mapPacketFree.find(pPacket);
			if(f == m_mapPacketFree.end())
			{
				m_mapPacketFree.insert(mapPacket::value_type(pPacket, pPacket));
			}
		}
		else
		{
			return NULL;
		}
	}

	//从free池中拿出一个,放入到used池中
	mapPacket::iterator itorFreeB = m_mapPacketFree.begin();
	_MakePacket* pPacket = (_MakePacket* )itorFreeB->second;
	m_mapPacketFree.erase(itorFreeB);
	//添加到used map里面
	mapPacket::iterator f = m_mapPacketUsed.find(pPacket);
	if(f == m_mapPacketUsed.end())
	{
		m_mapPacketUsed.insert(mapPacket::value_type(pPacket, pPacket));
	}

	return (_MakePacket* )pPacket;
}

bool CMakePacketPool::Delete(_MakePacket* pBuffPacket)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	_MakePacket* pBuff = (_MakePacket* )pBuffPacket;
	if(NULL == pBuff)
	{
		return false;
	}

	mapPacket::iterator f = m_mapPacketUsed.find(pBuff);
	if(f != m_mapPacketUsed.end())
	{
		m_mapPacketUsed.erase(f);

		//添加到Free map里面
		mapPacket::iterator f = m_mapPacketFree.find(pBuff);
		if(f == m_mapPacketFree.end())
		{
			m_mapPacketFree.insert(mapPacket::value_type(pBuff, pBuff));
		}
	}

	return true;
}


//*********************************************************************

CMakePacket::CMakePacket(void)
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

bool CMakePacket::IsRun()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_RunMutex);

	return m_blRun;
}


CMakePacket::~CMakePacket(void)
{
}

int CMakePacket::open(void* args)
{
	m_blRun = true;
	msg_queue()->high_water_mark(m_u4HighMask);
	msg_queue()->low_water_mark(m_u4LowMask);

	OUR_DEBUG((LM_INFO,"[CMakePacket::open] m_u4HighMask = [%d] m_u4LowMask = [%d]\n", m_u4HighMask, m_u4LowMask));
	if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, m_u4ThreadCount) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::open] activate error ThreadCount = [%d].", m_u4ThreadCount));
		m_blRun = false;
		return -1;
	}

	resume();

	return 0;
}

int CMakePacket::svc(void)
{
	uint32 u4threadCurNo = 0;
	ACE_Message_Block* mb = NULL;
	ACE_Time_Value xtime;

	m_rwMutex.acquire_write();
	uint32 u4ThreadID = (uint32)m_ThreadInfo.GetThreadCount();
	m_ThreadInfo.AddThreadInfo(u4ThreadID);
	OUR_DEBUG((LM_INFO,"[CMakePacket::svc]Create WorkThread=[%d]=[%d] ...\n", (uint32)m_ThreadInfo.GetThreadCount()));

	//获取线程的系统ID
	_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(u4ThreadID);
	if(NULL == pThreadInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::svc] pThreadInfo[%d] is not find.\n", u4threadCurNo));
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
		if(getq(mb, 0) == -1)
		{
			//OUR_DEBUG((LM_ERROR,"[CMakePacket::PutMessage] getq error errno = [%d].\n", errno));
			break;
		}
		if (mb == NULL)
		{
			continue;
		}
		_MakePacket* msg = *((_MakePacket**)mb->base());
		if (msg == NULL)
		{
			OUR_DEBUG((LM_ERROR,"[CMakePacket::svc] mb msg == NULL CurrthreadNo=[%d]!\n", u4threadCurNo));
			mb->release();
			continue;
		}

		//这里处理数据包请求
		ProcessMessageBlock(msg, u4ThreadID);
		mb->release();
	}

	OUR_DEBUG((LM_INFO,"[CMakePacket::svc] svc finish!\n"));
	return 0;
}

int CMakePacket::Close()
{
	KillTimer();
	m_blRun = false;
	msg_queue()->deactivate();
	msg_queue()->flush();
	OUR_DEBUG((LM_INFO,"[CMakePacket::close] close().\n"));
	return 0;
}

int CMakePacket::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	//这里进行线程自检
	for(uint32 i = 0; i < (int)m_u4ThreadCount; i++)
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
			//开始查看线程是否超时
			if(pThreadInfo->m_u4State == THREAD_RUNBEGIN && tvNow.sec() - pThreadInfo->m_tvUpdateTime.sec() > m_u2ThreadTimeOut)
			{
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMakePacket::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d] TimeOut > %d[%d].", 
					pThreadInfo->m_u4ThreadID, 
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime,
					m_u2ThreadTimeOut,
					tvNow.sec() - pThreadInfo->m_tvUpdateTime.sec());

				//记录各个缓冲池使用状态
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMakePacket::handle_timeout] MakePacketPool Used = %d, MakePacketPool Free = %d, PacketParsePool Used = %d, PacketParsePool Free = %d, MessagePool Used = %d, App_MessagePool Free = %d.", 
					m_MakePacketPool.GetUsedCount(),
					m_MakePacketPool.GetFreeCount(),
					App_PacketParsePool::instance()->GetUsedCount(),
					App_PacketParsePool::instance()->GetFreeCount(),
					App_MessagePool::instance()->GetUsedCount(),
					App_MessagePool::instance()->GetFreeCount());

				//发现阻塞线程，重启相应的线程
				ACE_OS::thr_kill(pThreadInfo->m_u4ThreadIndex, SIGALRM);
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMakePacket::handle_timeout]  pThreadInfo = [%d] ThreadID = [%d] Thread is reset.", pThreadInfo->m_u4ThreadID, pThreadInfo->m_u4ThreadIndex);
				m_ThreadInfo.CloseThread(pThreadInfo->m_u4ThreadID);
				ResumeThread(1);
			}
			else
			{
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMakePacket::handle_timeout] pThreadInfo = [%d] State = [%d] Time = [%04d-%02d-%02d %02d:%02d:%02d] PacketCount = [%d] LastCommand = [0x%x] PacketTime = [%d].", 
					pThreadInfo->m_u4ThreadID, 
					pThreadInfo->m_u4State, 
					dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), 
					pThreadInfo->m_u4RecvPacketCount,
					pThreadInfo->m_u2CommandID,
					pThreadInfo->m_u2PacketTime);

				//记录各个缓冲池使用状态
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_PACKETTHREAD, "[CMakePacket::handle_timeout] MakePacketPool Used = %d, MakePacketPool Free = %d, PacketParsePool Used = %d, PacketParsePool Free = %d, MessagePool Used = %d, App_MessagePool Free = %d.", 
					m_MakePacketPool.GetUsedCount(),
					m_MakePacketPool.GetFreeCount(),
					App_PacketParsePool::instance()->GetUsedCount(),
					App_PacketParsePool::instance()->GetFreeCount(),
					App_MessagePool::instance()->GetUsedCount(),
					App_MessagePool::instance()->GetFreeCount());
			}
		}
		else
		{
			OUR_DEBUG((LM_INFO,"[CMakePacket::handle_timeout] pThreadInfo = [%d] Not find.\n", i));
		}
	}

	return 0;
}

bool CMakePacket::ResumeThread(int nThreadCount)
{
	ACE_Thread::spawn_n(1, &ACE_Task_Base::svc_run, (void *)this);

	return true;
}

bool CMakePacket::Start()
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

void CMakePacket::Init(uint32 u4ThreadCount, uint32 u4MaxQueue, uint32 u4LowMask, uint32 u4HighMask)
{
	m_u4ThreadCount = u4ThreadCount;
	m_u4MaxQueue    = u4MaxQueue;
	m_u4HighMask    = u4HighMask;
	m_u4LowMask     = u4LowMask;

	m_MakePacketPool.Init(MAX_MP_POOL);
}

bool CMakePacket::StartTimer()
{
	OUR_DEBUG((LM_ERROR, "[CMakePacket::StartTimer]begin.\n"));

	m_u4TimerID = App_TimerManager::instance()->schedule(this, NULL, ACE_OS::gettimeofday() + ACE_Time_Value(MAX_MSG_STARTTIME), ACE_Time_Value(m_u2ThreadTimeCheck));
	if(0 == m_u4TimerID)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::StartTimer] Start thread time error.\n"));
		return false;
	}

	return true;
}

bool CMakePacket::KillTimer()
{
	if(m_u4TimerID > 0)
	{
		App_TimerManager::instance()->cancel(m_u4TimerID);
	}
	return true;
}

bool CMakePacket::PutUDPMessageBlock(const ACE_INET_Addr& AddrRemote, uint8 u1Option, CPacketParse* pPacketParse)
{
	ACE_Message_Block* pMb = App_MessageBlockManager::instance()->Create(sizeof(ACE_Message_Block*));
	if(pMb == NULL)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutMessageBlock] Get pMb is NULL.\n"));
		return false;
	}

	_MakePacket* pMakePacket = m_MakePacketPool.Create();
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutMessageBlock] Get pMakePacket is NULL.\n"));
		return false;
	}

	pMakePacket->m_u4ConnectID       = UDP_HANDER_ID;
	pMakePacket->m_u1Option          = u1Option;
	pMakePacket->m_pPacketParse      = pPacketParse;
	pMakePacket->m_AddrRemote        = AddrRemote;
	pMakePacket->m_PacketType        = PACKET_UDP;

	_MakePacket** ppMakePacket = (_MakePacket **)pMb->base();
	*ppMakePacket = pMakePacket;

	//判断队列是否是已经最大
	int nQueueCount = (int)msg_queue()->message_count();
	if(nQueueCount >= (int)m_u4MaxQueue)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));
		pMb->release();
		return false;
	}

	ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
	if(this->putq(pMb, &xtime) == -1)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
		pMb->release();
		return false;
	}

	return true;
}

bool CMakePacket::PutMessageBlock(uint32 u4ConnectID, uint8 u1Option, CPacketParse* pPacketParse)
{
	ACE_Message_Block* pMb = App_MessageBlockManager::instance()->Create(sizeof(ACE_Message_Block*));
	if(pMb == NULL)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutMessageBlock] Get pMb is NULL.\n"));
		return false;
	}

	_MakePacket* pMakePacket = m_MakePacketPool.Create();
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutMessageBlock] Get pMakePacket is NULL.\n"));
		return false;
	}

	pMakePacket->m_u4ConnectID       = u4ConnectID;
	pMakePacket->m_u1Option          = u1Option;
	pMakePacket->m_pPacketParse      = pPacketParse;

	_MakePacket** ppMakePacket = (_MakePacket **)pMb->base();
	*ppMakePacket = pMakePacket;

	//判断队列是否是已经最大
	int nQueueCount = (int)msg_queue()->message_count();
	if(nQueueCount >= (int)m_u4MaxQueue)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));
		pMb->release();
		return false;
	}

	ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
	if(this->putq(pMb, &xtime) == -1)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
		pMb->release();
		return false;
	}

	return true;
}

bool CMakePacket::ProcessMessageBlock(_MakePacket* pMakePacket, uint32 u4ThreadID)
{
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::ProcessMessageBlock] pMakePacket is NULL.\n"));
		return false;
	}

	//在这里进行线程自检代码
	_ThreadInfo* pThreadInfo = m_ThreadInfo.GetThreadInfo(u4ThreadID);
	if(NULL == pThreadInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::ProcessMessageBlock] pThreadInfo[%d] is not find.\n", 0));
		return false;
	}

	pThreadInfo->m_tvUpdateTime = ACE_OS::gettimeofday();
	pThreadInfo->m_u4State = THREAD_RUNBEGIN;

	//抛出掉链接建立和断开的命令，只计算逻辑处理包数
	if(pMakePacket->m_u1Option != PACKET_CONNECT && pMakePacket->m_u1Option != PACKET_CDISCONNECT && pMakePacket->m_u1Option != PACKET_SDISCONNECT)
	{
		pThreadInfo->m_u4RecvPacketCount++;
		pThreadInfo->m_u4CurrPacketCount++;
	}

	//根据操作OP，调用相应的方法。
	CMessage* pMessage = NULL;
	if(pMakePacket->m_u1Option == PACKET_PARSE)
	{
		if(pMakePacket->m_PacketType == 0)
		{
			//TCP数据包处理方法
			pMessage = SetMessage(pMakePacket->m_pPacketParse, pMakePacket->m_u4ConnectID);
		}
		else
		{
			//UDP数据包处理方法
			pMessage = SetMessage(pMakePacket->m_pPacketParse, pMakePacket->m_AddrRemote);
		}		
	}
	else if(pMakePacket->m_u1Option == PACKET_CONNECT)
	{
		pMessage = SetMessageConnect(pMakePacket->m_pPacketParse, pMakePacket->m_u4ConnectID);
	}
	else if(pMakePacket->m_u1Option == PACKET_CDISCONNECT)
	{
		pMessage = SetMessageCDisConnect(pMakePacket->m_pPacketParse, pMakePacket->m_u4ConnectID);
	}
	else if(pMakePacket->m_u1Option == PACKET_SDISCONNECT)
	{
		pMessage = SetMessageSDisConnect(pMakePacket->m_pPacketParse, pMakePacket->m_u4ConnectID);
	}

	if(NULL != pMessage)
	{
		//将要处理的消息放入消息处理线程
		if(false == App_MessageService::instance()->PutMessage(pMessage))
		{
			OUR_DEBUG((LM_ERROR, "[CMakePacket::ProcessMessageBlock] App_MessageService::instance()->PutMessage Error.\n"));
			App_MessagePool::instance()->Delete(pMessage);
			App_PacketParsePool::instance()->Delete(pMakePacket->m_pPacketParse);
			m_MakePacketPool.Delete(pMakePacket);
			return false;
		}
	}

	if(NULL != pMakePacket->m_pPacketParse)
	{
		App_PacketParsePool::instance()->Delete(pMakePacket->m_pPacketParse);
	}

	m_MakePacketPool.Delete(pMakePacket);

	pThreadInfo->m_u4State = THREAD_RUNEND;

	return true;
}

CMessage* CMakePacket::SetMessage(CPacketParse* pPacketParse, uint32 u4ConnectID)
{
	//创建新的Message对象
	CMessage* pMessage = App_MessagePool::instance()->Create();
	if(NULL == pMessage)
	{
		//写入接收包错误
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage is NULL.\n", u4ConnectID));
		return NULL;
	}

	if(NULL != pMessage->GetMessageBase())
	{
		//开始组装数据
		pMessage->GetMessageBase()->m_u4ConnectID = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd       = pPacketParse->GetPacketCommandID();
		pMessage->GetMessageBase()->m_u4MsgTime   = (uint32)ACE_OS::gettimeofday().sec();

		//将接受的数据缓冲放入CMessage对象
		pMessage->SetPacketHead(pPacketParse->GetMessageHead());
		pMessage->SetPacketBody(pPacketParse->GetMessageBody());

		return pMessage;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
		return NULL;
	}
}

CMessage* CMakePacket::SetMessage(CPacketParse* pPacketParse, const ACE_INET_Addr& AddrRemote)
{
	//创建新的Message对象
	CMessage* pMessage = App_MessagePool::instance()->Create();
	if(NULL == pMessage)
	{
		//写入接收包错误
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] UDP ConnectID, pMessage is NULL.\n"));
		return NULL;
	}

	if(NULL != pMessage->GetMessageBase())
	{
		//开始组装数据
		pMessage->GetMessageBase()->m_u4ConnectID  = UDP_HANDER_ID;
		pMessage->GetMessageBase()->m_u2Cmd        = pPacketParse->GetPacketCommandID();
		pMessage->GetMessageBase()->m_u4MsgTime    = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4Port       = (uint32)AddrRemote.get_port_number();
		pMessage->GetMessageBase()->m_u1PacketType = PACKET_UDP;
		sprintf_safe(pMessage->GetMessageBase()->m_szIP, MAX_BUFF_20, "%s", AddrRemote.get_host_addr());

		//将接受的数据缓冲放入CMessage对象
		pMessage->SetPacketHead(pPacketParse->GetMessageHead());
		pMessage->SetPacketBody(pPacketParse->GetMessageBody());
		
		return pMessage;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] UDP ConnectID, pMessage->GetMessageBase() is NULL.\n"));
		return NULL;
	}
}

CMessage* CMakePacket::SetMessageConnect(CPacketParse* pPacketParse, uint32 u4ConnectID)
{
	//创建新的Message对象
	CMessage* pMessage = App_MessagePool::instance()->Create();
	if(NULL == pMessage)
	{
		//写入接收包错误
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage is NULL.\n", u4ConnectID));
		return NULL;
	}

	if(NULL != pMessage->GetMessageBase())
	{
		//开始组装数据
		pMessage->GetMessageBase()->m_u4ConnectID = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd       = CLIENT_LINK_CONNECT;
		pMessage->GetMessageBase()->m_u4MsgTime   = (uint32)ACE_OS::gettimeofday().sec();

		//将接受的数据缓冲放入CMessage对象
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);

		return pMessage;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
		return NULL;
	}
}

CMessage* CMakePacket::SetMessageCDisConnect(CPacketParse* pPacketParse, uint32 u4ConnectID)
{
	//创建新的Message对象
	CMessage* pMessage = App_MessagePool::instance()->Create();
	if(NULL == pMessage)
	{
		//写入接收包错误
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage is NULL.\n", u4ConnectID));
		return NULL;
	}

	if(NULL != pMessage->GetMessageBase())
	{
		//开始组装数据
		pMessage->GetMessageBase()->m_u4ConnectID = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd       = CLIENT_LINK_CDISCONNET;
		pMessage->GetMessageBase()->m_u4MsgTime   = (uint32)ACE_OS::gettimeofday().sec();

		//将接受的数据缓冲放入CMessage对象
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);

		return pMessage;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
		return NULL;
	}
}

CMessage* CMakePacket::SetMessageSDisConnect(CPacketParse* pPacketParse, uint32 u4ConnectID)
{
	//创建新的Message对象
	CMessage* pMessage = App_MessagePool::instance()->Create();
	if(NULL == pMessage)
	{
		//写入接收包错误
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage is NULL.\n", u4ConnectID));
		return NULL;
	}

	if(NULL != pMessage->GetMessageBase())
	{
		//开始组装数据
		pMessage->GetMessageBase()->m_u4ConnectID = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd       = CLIENT_LINK_SDISCONNET;
		pMessage->GetMessageBase()->m_u4MsgTime   = (uint32)ACE_OS::gettimeofday().sec();

		//将接受的数据缓冲放入CMessage对象
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);

		return pMessage;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
		return NULL;
	}
}

CThreadInfo* CMakePacket::GetThreadInfo()
{
	return &m_ThreadInfo;
}
