// LogManager.h
// Log�Ĺ����࣬����Log��־�Ķ������
// һ��ʼ�������Ϊһ��DLL����ʵ�֣����ǿ���һ�£����Ƿ��ڿ������ʵ�ֱȽϺá�
// add by freeeyes
// 2009-04-04

#include "LogManager.h"

Mutex_Allocator _log_service_mb_allocator; 

CLogBlockPool::CLogBlockPool()
{
	//��ʼ������־��
	m_pLogBlockInfo = NULL;

	m_u4MaxBlockSize = 0;
	m_u4PoolCount    = 0;
	m_u4CurrIndex    = 0;
}

CLogBlockPool::~CLogBlockPool()
{
	Close();
}

void CLogBlockPool::Init(uint32 u4BlockSize, uint32 u4PoolCount)
{
	//��ʼ������־��
	if(NULL != m_pLogBlockInfo)
	{
		Close();
	}

	m_pLogBlockInfo = new _LogBlockInfo[u4PoolCount];
	if(NULL == m_pLogBlockInfo)
	{
		OUR_DEBUG((LM_INFO, "[CLogBlockPool::CLogBlockPool]CLogBlockPool new (%d) fail.\n", u4PoolCount));
	}

	for(uint32 i = 0; i < u4PoolCount; i++)
	{
		m_pLogBlockInfo[i].m_pBlock = new char[u4BlockSize];
	}

	m_u4MaxBlockSize = u4BlockSize;
	m_u4PoolCount    = u4PoolCount;
	m_u4CurrIndex    = 0;

}

void CLogBlockPool::Close()
{
	for(uint32 i = 0; i < m_u4PoolCount; i++)
	{
		SAFE_DELETE_ARRAY(m_pLogBlockInfo[i].m_pBlock);
	}

	SAFE_DELETE_ARRAY(m_pLogBlockInfo);
}

_LogBlockInfo* CLogBlockPool::GetLogBlockInfo()
{
	if(NULL == m_pLogBlockInfo)
	{
		return NULL;
	}

	_LogBlockInfo* pLogBlockInfo = NULL;
	if(m_u4CurrIndex  == m_u4PoolCount - 1)
	{
		m_u4CurrIndex = 0;
	}

	pLogBlockInfo = &m_pLogBlockInfo[m_u4CurrIndex];

	if(pLogBlockInfo->m_blIsUsed == false)
	{
		pLogBlockInfo->m_blIsUsed = true;
	}
	else
	{
		OUR_DEBUG((LM_ERROR,"[CLogBlockPool::GetLogBlockInfo]***CLogBlockPool is all used!***\n"));
		return NULL;
	}

	m_u4CurrIndex++;

	memset(pLogBlockInfo->m_pBlock, 0, m_u4MaxBlockSize);
	return pLogBlockInfo;
}

void CLogBlockPool::ReturnBlockInfo(_LogBlockInfo* pLogBlockInfo)
{
	memset(pLogBlockInfo->m_pBlock, 0, m_u4MaxBlockSize);
	pLogBlockInfo->m_blIsUsed = false;
}

uint32 CLogBlockPool::GetBlockSize()
{
	return m_u4MaxBlockSize;
}
//******************************************************************

CLogManager::CLogManager(void)
{
	m_blRun         = false;
	m_nThreadCount  = 1;
	m_nQueueMax     = MAX_MSG_THREADQUEUE;
	m_pServerLogger = NULL;
}

CLogManager::~CLogManager(void)
{
	OUR_DEBUG((LM_ERROR,"[CLogManager::~CLogManager].\n"));
	SAFE_DELETE(m_pServerLogger);
	OUR_DEBUG((LM_ERROR,"[CLogManager::~CLogManager]End.\n"));
}

int CLogManager::open(void *args)
{
	if(args != NULL)
	{
		OUR_DEBUG((LM_ERROR,"[CLogManager::open]args is not NULL.\n"));
	}
	
	if(activate(THR_NEW_LWP | THR_DETACHED, m_nThreadCount) == -1)
	{
		m_blRun = false;
		OUR_DEBUG((LM_ERROR,"[CLogManager::open] activate is error[%d].", errno));
		return -1;
	}

	m_blRun = true;
	return 0;
}

int CLogManager::svc(void)
{
	OUR_DEBUG((LM_INFO,"[CLogManager::svc] svc run.\n"));
	ACE_Message_Block* mb = NULL;
	//ACE_Time_Value     xtime;
	while(m_blRun)
	{
		mb = NULL;
		//xtime=ACE_OS::gettimeofday()+ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(getq(mb, 0) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CLogManager::svc] get error errno = [%d].\n", errno));
			m_blRun = false;
			break;
		}

		if (mb == NULL)
		{
			continue;
		}

		_LogBlockInfo* pLogBlockInfo = *((_LogBlockInfo**)mb->base());
		if (!pLogBlockInfo)
		{
			OUR_DEBUG((LM_ERROR,"[CLogManager::svc] CLogManager mb log == NULL!\n"));
			mb->release();
			continue;
		}

		ProcessLog(pLogBlockInfo);
		mb->release();
		//OUR_DEBUG((LM_ERROR,"[CLogManager::svc] delete pstrLogText BEGIN!\n"));
		//������־��
		m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
		//OUR_DEBUG((LM_ERROR,"[CLogManager::svc] delete pstrLogText END!\n"));

	}
	OUR_DEBUG((LM_INFO,"[CLogManager::svc] CLogManager::svc finish!\n"));
	return 0;
}

int CLogManager::Close()
{
	msg_queue()->deactivate();
	msg_queue()->flush();
	return 0;
}

void CLogManager::Init(int nThreadCount, int nQueueMax)
{
	m_nThreadCount = nThreadCount;
	m_nQueueMax    = nQueueMax;
}

int CLogManager::Start()
{
	if(m_blRun == true)
	{
		return 0;
	}
	else
	{
		SetReset(false);
		return open();
	}
	
}

int CLogManager::Stop()
{
	m_blRun = false;
	return 0;
}

bool CLogManager::IsRun()
{
	return m_blRun;
}

int CLogManager::PutLog(_LogBlockInfo* pLogBlockInfo)
{
	ACE_Message_Block* mb = NULL;

	//����������¼���
	if(m_blIsNeedReset == true)
	{
		//������־��
		m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
		return 0;
	}

	ACE_NEW_MALLOC_NORETURN (mb,
		static_cast<ACE_Message_Block*>(_log_service_mb_allocator.malloc (sizeof (ACE_Message_Block))),
		ACE_Message_Block
		(	sizeof(ACE_TString *), // size
		0,
		0,
		0,
		&_log_service_mb_allocator, // allocator_strategy
		0, // locking strategy
		ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
		ACE_Time_Value::zero,
		ACE_Time_Value::max_time,
		&_log_service_mb_allocator,
		&_log_service_mb_allocator
		)
		);		

	if(mb)
	{
		_LogBlockInfo** loadin = (_LogBlockInfo **)mb->base();
		*loadin = pLogBlockInfo;

		int msgcount = (int)msg_queue()->message_count();
		if (msgcount >= m_nQueueMax) 
		{
			OUR_DEBUG((LM_INFO,"[CLogManager::PutLog] CLogManager queue is full!\n"));
			//������־��
			m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
			mb->release();
			return 1;
		}
		ACE_Time_Value xtime;
		xtime = ACE_OS::gettimeofday();
		if(this->putq(mb, &xtime) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CLogManager::PutLog] CLogManager putq error(%s)!\n", pLogBlockInfo->m_pBlock));
			//������־��
			m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
			mb->release();
			return -1;
		}
		return 0;
	}

	OUR_DEBUG((LM_ERROR,"[CLogManager::PutLog] CLogManager new ACE_Message_Block error!\n"));	
	return -1;
}

int CLogManager::RegisterLog(CServerLogger* pServerLogger)
{
	if(pServerLogger == NULL)
	{
		return -1;
	}

	//������־���Ͷ���
	if(m_pServerLogger != NULL)
	{
		SAFE_DELETE(m_pServerLogger);
	}

	m_pServerLogger = pServerLogger;

	//��ʼ����־��
	OUR_DEBUG((LM_ERROR,"[CLogManager::RegisterLog] GetBlockSize=%d, GetPoolCount=%d!\n", pServerLogger->GetBlockSize(), pServerLogger->GetPoolCount()));	
	m_objLogBlockPool.Init(pServerLogger->GetBlockSize(), pServerLogger->GetPoolCount());

	return 0;
}

int CLogManager::UnRegisterLog()
{
	if(m_pServerLogger != NULL)
	{
		SAFE_DELETE(m_pServerLogger);
	}

	return 0;
}

int CLogManager::ProcessLog(_LogBlockInfo* pLogBlockInfo)
{
	if(NULL == m_pServerLogger)
	{
		return -1;
	}

	//m_Logger_Mutex.acquire();
	m_pServerLogger->DoLog((int)pLogBlockInfo->m_u4LogID, pLogBlockInfo);
	//m_Logger_Mutex.release();
	return 0;
}

//*****************************************************************************

int CLogManager::WriteLog(int nLogType, const char* fmt, ...)
{
	int  nRet = 0;
	//����־��������ҵ�һ��������־��
	m_Logger_Mutex.acquire();
	_LogBlockInfo* pLogBlockInfo = m_objLogBlockPool.GetLogBlockInfo();
	

	if(NULL == pLogBlockInfo)
	{
		OUR_DEBUG((LM_ERROR,"[CLogManager::WriteLog] m_objLogBlockPool is full!\n"));
		m_Logger_Mutex.release();
		return -1;
	}

	va_list ap;
	va_start(ap, fmt);
	ACE_OS::vsnprintf(pLogBlockInfo->m_pBlock, m_objLogBlockPool.GetBlockSize() - 1, fmt, ap);
	va_end(ap);

	pLogBlockInfo->m_u4Length = (uint32)strlen(pLogBlockInfo->m_pBlock);
	pLogBlockInfo->m_u4LogID  = (uint32)nLogType;

	if (IsRun()) 
	{
		nRet = PutLog(pLogBlockInfo);
	} 
	else 
	{
		m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
	}

	m_Logger_Mutex.release();
	return 0;
}


int CLogManager::WriteLogBinary(int nLogType, const char* pData, int nLen)
{
	int nRet = 0;
	//����־��������ҵ�һ��������־��
	m_Logger_Mutex.acquire();
	_LogBlockInfo* pLogBlockInfo = m_objLogBlockPool.GetLogBlockInfo();

	if(NULL == pLogBlockInfo)
	{
		OUR_DEBUG((LM_ERROR,"[ILogManager::WriteLogBinary] m_objLogBlockPool is full!\n"));	
		m_Logger_Mutex.release();
		return -1;
	}

	//�Ѷ�����ת�������Ĵ洢
	if((uint32)(nLen * 5) >= m_objLogBlockPool.GetBlockSize())
	{
		OUR_DEBUG((LM_ERROR,"[ILogManager::WriteLogBinary] write length is more than BlockSize!\n"));
		m_Logger_Mutex.release();
		return -1;
	}

	pLogBlockInfo->m_u4LogID = nLogType;
	char szLog[10]  = {'\0'};
	for(int i = 0; i < nLen; i++)
	{
		sprintf_safe(szLog, 10, "0x%02X ", (unsigned char)pData[i]);
		sprintf_safe(pLogBlockInfo->m_pBlock + 5*i, m_objLogBlockPool.GetBlockSize() - 5*i, "%s", szLog);
	}

	pLogBlockInfo->m_u4Length = (uint32)(nLen * 5);

	if (IsRun()) 
	{
		nRet = PutLog(pLogBlockInfo);
		if (nRet) 
		{
			m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
		}
	} 
	else 
	{
		m_objLogBlockPool.ReturnBlockInfo(pLogBlockInfo);
	}

	m_Logger_Mutex.release();
	return nRet;
}

void CLogManager::SetReset(bool blReset)
{
	m_blIsNeedReset = blReset;
}

void CLogManager::ResetLogData(uint16 u2LogLevel)
{
	//����������־�ȼ���������־�ļ�
	SetReset(true);

	//����ȴ�һ��ʱ�䣬�ȴ�������־ȫ��д����ɣ���������־ģ�顣
	//�������ټ�һ����
	ACE_Time_Value tvSleep(0, 1000);
	ACE_OS::sleep(tvSleep);

	m_pServerLogger->ReSet(u2LogLevel);
	SetReset(false);
}

uint32 CLogManager::GetLogCount()
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetLogTypeCount();
	}
	else
	{
		return (uint32)0;
	}
}

uint32 CLogManager::GetCurrLevel()
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetCurrLevel();
	}
	else
	{
		return (uint32)0;
	}
}

uint16 CLogManager::GetLogID(uint16 u2Index)
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetLogID(u2Index);
	}
	else
	{
		return (uint16)0;
	}
}

char* CLogManager::GetLogInfoByServerName(uint16 u2LogID)
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetLogInfoByServerName(u2LogID);
	}
	else
	{
		return NULL;
	}
}

char* CLogManager::GetLogInfoByLogName(uint16 u2LogID)
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetLogInfoByLogName(u2LogID);
	}
	else
	{
		return NULL;
	}
}

int CLogManager::GetLogInfoByLogDisplay(uint16 u2LogID)
{
	if(m_pServerLogger != NULL)
	{
		return m_pServerLogger->GetLogType(u2LogID);
	}
	else
	{
		return 0;
	}
}
