#include "MainConfig.h"

CMainConfig::CMainConfig(void)
{
	m_szError[0] = '\0';

	m_u4MsgHighMark         = 0;
	m_u4MsgLowMark          = 0;
	m_u4MsgThreadCount      = 0;
	m_u4MsgMaxQueue         = 0;
	m_nEncryptFlag          = 0;
	m_nEncryptOutFlag       = 0;
	m_u4ReactorCount        = 3;
	m_u4SendThresHold       = 0;
	m_u4RecvBuffSize        = 0;
	m_u2ThreadTimuOut       = 0;
	m_u2ThreadTimeCheck     = 0;
	m_u2PacketTimeOut       = 0;
	m_u2SendAliveTime       = 0;
	m_u2HandleCount         = 0;
	m_u2MaxHanderCount      = 0;
	m_u2MaxConnectTime      = 0;
	m_u1CommandAccount      = 0;

	m_u1ConsoleSupport      = 0;
	m_nConsolePort          = 0;

	m_u2ValidConnectCount   = MAX_CONNECT_COUNT;
	m_u1Valid               = 1;
	m_u4ValidPacketCount    = 0;
	m_u4ValidRecvSize       = 0;
	m_u2ForbiddenTime       = 0;
	m_u2RecvQueueTimeout    = MAX_QUEUE_TIMEOUT;
	m_u2SendQueueTimeout    = MAX_QUEUE_TIMEOUT;
	m_u2SendQueueCount      = SENDQUEUECOUNT;

	m_szServerName[0]       = '\0';
	m_szModulePath[0]       = '\0';
	m_szResourceName[0]     = '\0';
	m_szEncryptPass[0]      = '\0';
}

CMainConfig::~CMainConfig(void)
{
	OUR_DEBUG((LM_INFO, "[CMainConfig::~CMainConfig].\n"));
	m_vecServerInfo.clear();
	m_vecUDPServerInfo.clear();
	OUR_DEBUG((LM_INFO, "[CMainConfig::~CMainConfig]End.\n"));
}

const char* CMainConfig::GetError()
{
	return m_szError;
}

bool CMainConfig::Init(const char* szConfigPath)
{
  char* pData = NULL;
	OUR_DEBUG((LM_INFO, "[CMainConfig::Init]Filename = %s.\n", szConfigPath));
	if(false == m_MainConfig.Init(szConfigPath))
  {
    OUR_DEBUG((LM_INFO, "[CMainConfig::Init]File Read Error = %s.\n", szConfigPath));
    return false;
  }

	pData = m_MainConfig.GetData("ServerID", "id");
  if(NULL != pData)
  {
	  m_nServerID = ACE_OS::atoi(pData);
  }

	pData = m_MainConfig.GetData("ServerName", "name");
  if(NULL != pData)
  {
	  sprintf_safe(m_szServerName, MAX_BUFF_20, "%s", pData);
  }

	//获得监听端口信息
	_ServerInfo serverinfo;

	m_vecServerInfo.clear();
  TiXmlElement* pNextTiXmlElementIP = NULL;
  TiXmlElement* pNextTiXmlElementPort = NULL;
	while(true)
	{
		pData = m_MainConfig.GetData("TCPServerIP", "ip", pNextTiXmlElementIP);
    if(pData != NULL)
    {
		  sprintf_safe(serverinfo.m_szServerIP, MAX_BUFF_20, "%s", pData);
    }
    else
    {
      break;
    }

		pData = m_MainConfig.GetData("TCPServerIP", "port", pNextTiXmlElementPort);
    if(pData != NULL)
    {
      serverinfo.m_nPort = ACE_OS::atoi(pData);
    }
    else
    {
      break;
    }
		

		m_vecServerInfo.push_back(serverinfo);
	}

	//开始获得消息处理线程参数
	pData = m_MainConfig.GetData("Message", "Msg_High_mark");
  if(pData != NULL)
  {
	  m_u4MsgHighMark = (uint32)ACE_OS::atoi(pData);
  }
  pData = m_MainConfig.GetData("Message", "Msg_Low_mark");
  if(pData != NULL)
  {
	  m_u4MsgLowMark = (uint32)ACE_OS::atoi(pData);
  }
  pData = m_MainConfig.GetData("Message", "Msg_Thread");
  if(pData != NULL)
  {
    m_u4MsgThreadCount = (uint32)ACE_OS::atoi(pData);
  }

	
	pData = m_MainConfig.GetData("Message", "Msg_MaxQueue");
  if(pData != NULL)
  {
	  m_u4MsgMaxQueue = (uint32)ACE_OS::atoi(pData);
  }

	//开始获得UDP服务器相关参数
	m_vecUDPServerInfo.clear();
  pNextTiXmlElementIP   = NULL;
  pNextTiXmlElementPort = NULL;
	while(true)
	{
		pData = m_MainConfig.GetData("UDPServerIP", "uip", pNextTiXmlElementIP);
    if(pData != NULL)
    {
		  sprintf_safe(serverinfo.m_szServerIP, MAX_BUFF_20, "%s", pData);
    }
    else
    {
      break;
    }

		pData = m_MainConfig.GetData("UDPServerIP", "uport", pNextTiXmlElementPort);
    if(pData != NULL)
    {
		  serverinfo.m_nPort = ACE_OS::atoi(pData);
    }
    else
    {
      break;
    }

		m_vecUDPServerInfo.push_back(serverinfo);
	}

	//开始获得加载模块参数
	pData = m_MainConfig.GetData("Module", "ModulePath");
  if(pData != NULL)
  {
	  sprintf_safe(m_szModulePath, MAX_BUFF_200, "%s", pData);
  }

	pData = m_MainConfig.GetData("Module", "ModuleString");
  if(pData != NULL)
  {
	  sprintf_safe(m_szResourceName, MAX_BUFF_200, "%s", pData);
  }

	//开始获得发送和接受阀值
  pData = m_MainConfig.GetData("SendInfo", "SendThresHold");
  if(pData != NULL)
  {
	  m_u4SendThresHold = (int)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("RecvInfo", "RecvBuffSize");
  if(pData != NULL)
  {
	  m_u4RecvBuffSize = (int)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("SendInfo", "SendQueueMax");
  if(pData != NULL)
  {
	  m_u2SendQueueMax = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ThreadInfo", "ThreadTimeout");
  if(pData != NULL)
  {
	  m_u2ThreadTimuOut = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ThreadInfo", "ThreadTimeCheck");
  if(pData != NULL)
  {
	  m_u2ThreadTimeCheck = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ThreadInfo", "DisposeTimeout");
  if(pData != NULL)
  {
	  m_u2PacketTimeOut = (uint16)ACE_OS::atoi(pData);
  }
  pData = m_MainConfig.GetData("SendInfo", "SendAliveTime");
  if(pData != NULL)
  {
	  m_u2SendAliveTime = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ClientInfo", "HandlerCount");
  if(pData != NULL)
  {
	  m_u2HandleCount = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ClientInfo", "MaxHandlerCount");
  if(pData != NULL)
  {
	  m_u2MaxHanderCount = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ClientInfo", "MaxConnectTime");
  if(pData != NULL)
  {
	  m_u2MaxConnectTime = (uint16)ACE_OS::atoi((char*)pData);
  }
	pData = m_MainConfig.GetData("RecvInfo", "RecvQueueTimeout");
  if(pData != NULL)
  {
	  m_u2RecvQueueTimeout = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("SendInfo", "SendQueueTimeout");
  if(pData != NULL)
  {
	  m_u2SendQueueTimeout = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("SendInfo", "SendQueueCount");
  if(pData != NULL)
  {
	  m_u2SendQueueCount = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ClientInfo", "CommandAccount");
  if(pData != NULL)
  {
	  m_u1CommandAccount = (uint8)ACE_OS::atoi(pData);
  }

	//开始获得Console服务器相关配置信息
	pData = m_MainConfig.GetData("Console", "support");
  if(pData != NULL)
  {
	  m_u1ConsoleSupport = (uint8)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("Console", "sip");
  if(pData != NULL)
  {
	  sprintf_safe(m_szConsoleIP, MAX_BUFF_100, "%s", pData);
  }
	pData = m_MainConfig.GetData("Console", "sport");
  if(pData != NULL)
  {
	  m_nConsolePort = (int)ACE_OS::atoi(pData);
  }

  //获得Console可接受的客户端IP
	m_vecConsoleClientIP.clear();
  pNextTiXmlElementIP   = NULL;
	while(true)
	{
		_ConsoleClientIP ConsoleClientIP;
    pData = m_MainConfig.GetData("ConsoleClient", "cip", pNextTiXmlElementIP);
    if(NULL != pData)
    {
		  sprintf_safe(ConsoleClientIP.m_szServerIP, MAX_BUFF_20, "%s", pData);
    }
    else
    {
      break;
    }

		m_vecConsoleClientIP.push_back(ConsoleClientIP);
	}

	//开始获得ConnectValid对应的参数
	pData = m_MainConfig.GetData("ConnectValid", "ConnectCount");
  if(NULL != pData)
  {
	  m_u2ValidConnectCount = (uint16)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ConnectValid", "ConnectValid");
  if(NULL != pData)
  {
    m_u1Valid = (uint8)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ConnectValid", "ConnectPacketCount");
  if(NULL != pData)
  {
	  m_u4ValidPacketCount = (uint32)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ConnectValid", "ConnectRecvSize");
  if(NULL != pData)
  {
	  m_u4ValidRecvSize = (uint32)ACE_OS::atoi(pData);
  }
	pData = m_MainConfig.GetData("ConnectValid", "ForbiddenTime");
  if(NULL != pData)
  {
	  m_u2ForbiddenTime = (uint16)ACE_OS::atoi(pData);
  }
	return true;
}

void CMainConfig::Display()
{
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_nServerID = %d.\n", m_nServerID));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_szServerName = %s.\n", m_szServerName));

	for(int i = 0; i < (int)m_vecServerInfo.size(); i++)
	{
		OUR_DEBUG((LM_INFO, "[CMainConfig::Display]ServerIP%d = %s.\n", i, m_vecServerInfo[i].m_szServerIP));
		OUR_DEBUG((LM_INFO, "[CMainConfig::Display]ServerPort%d = %d.\n", i, m_vecServerInfo[i].m_nPort));
	}

	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4MsgHighMark = %d.\n", m_u4MsgHighMark));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4MsgLowMark = %d.\n", m_u4MsgLowMark));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4MsgThreadCount = %d.\n", m_u4MsgThreadCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4MsgMaxQueue = %d.\n", m_u4MsgMaxQueue));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_szModulePath = %s.\n", m_szModulePath));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_szResourceName = %s.\n", m_szResourceName));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_nEncryptFlag = %d.\n", m_nEncryptFlag));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_szEncryptPass = %s.\n", m_szEncryptPass));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_nEncryptOutFlag = %d.\n", m_nEncryptOutFlag));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4SendThresHold = %d.\n", m_u4SendThresHold));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4RecvBuffSize = %d.\n", m_u4RecvBuffSize));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2SendQueueMax = %d.\n", m_u2SendQueueMax));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2ThreadTimuOut = %d.\n", m_u2ThreadTimuOut));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2ThreadTimeCheck = %d.\n", m_u2ThreadTimeCheck));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2PacketTimeOut = %d.\n", m_u2PacketTimeOut));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2SendAliveTime = %d.\n", m_u2SendAliveTime));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2HandleCount = %d.\n", m_u2HandleCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2MaxHanderCount = %d.\n", m_u2MaxHanderCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2RecvQueueTimeout = %d.\n", m_u2RecvQueueTimeout));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2SendQueueTimeout = %d.\n", m_u2SendQueueTimeout));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2SendQueueCount = %d.\n", m_u2SendQueueCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u1CommandAccount = %d.\n", m_u1CommandAccount));
	
	for(int i = 0; i < (int)m_vecUDPServerInfo.size(); i++)
	{
		OUR_DEBUG((LM_INFO, "[CMainConfig::Display]ServerIP%d = %s.\n", i, m_vecUDPServerInfo[i].m_szServerIP));
		OUR_DEBUG((LM_INFO, "[CMainConfig::Display]ServerPort%d = %d.\n", i, m_vecUDPServerInfo[i].m_nPort));
	}

	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u1ConsoleSupport = %d.\n", m_u1ConsoleSupport));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_szConsoleIP = %s.\n", m_szConsoleIP));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_nConsolePort = %d.\n", m_nConsolePort));

	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2ValidConnectCount = %d.\n", m_u2ValidConnectCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u1Valid = %d.\n", m_u1Valid));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4ValidPacketCount = %d.\n", m_u4ValidPacketCount));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u4ValidRecvSize = %d.\n", m_u4ValidRecvSize));
	OUR_DEBUG((LM_INFO, "[CMainConfig::Display]m_u2ForbiddenTime = %d.\n", m_u2ForbiddenTime));
}

const char* CMainConfig::GetServerName()
{
	return m_szServerName;
}

uint16 CMainConfig::GetServerID()
{
	return (uint16)m_nServerID;
}

uint16 CMainConfig::GetServerPortCount()
{
	return (uint16)m_vecServerInfo.size();
}

uint16 CMainConfig::GetHandleCount()
{
	return m_u2HandleCount;
}

_ServerInfo* CMainConfig::GetServerPort(int nIndex)
{
	if(nIndex > (uint16)m_vecServerInfo.size())
	{
		return NULL;
	}

	return &m_vecServerInfo[nIndex];
}

uint32 CMainConfig::GetMgsHighMark()
{
	return m_u4MsgHighMark;
}

uint32 CMainConfig::GetMsgLowMark()
{
	return m_u4MsgLowMark;
}

uint32 CMainConfig::GetThreadCount()
{
	return m_u4MsgThreadCount;
}

uint32 CMainConfig::GetMsgMaxQueue()
{
	return m_u4MsgMaxQueue;
}

const char* CMainConfig::GetModulePath()
{
	return m_szModulePath;
}

const char* CMainConfig::GetModuleString()
{
	return m_szResourceName;
}

int CMainConfig::GetEncryptFlag()
{
	return m_nEncryptFlag;
}

const char* CMainConfig::GetEncryptPass() 
{
	return m_szEncryptPass;
}

int CMainConfig::GetEncryptOutFlag()
{
	return m_nEncryptOutFlag;
}

uint32 CMainConfig::GetReactorCount()
{
	return m_u4ReactorCount;
}

uint16 CMainConfig::GetUDPServerPortCount()
{
	return (uint16)m_vecUDPServerInfo.size();
}

_ServerInfo* CMainConfig::GetUDPServerPort(int nIndex)
{
	if(nIndex > (uint16)m_vecUDPServerInfo.size())
	{
		return NULL;
	}

	return &m_vecUDPServerInfo[nIndex];
}

uint32 CMainConfig::GetSendThresHold()
{
	return m_u4SendThresHold;
}

uint32 CMainConfig::GetRecvBuffSize()
{
	return m_u4RecvBuffSize;
}

uint16 CMainConfig::GetSendQueueMax()
{
	return m_u2SendQueueMax;
}

uint16 CMainConfig::GetThreadTimuOut()
{
	return m_u2ThreadTimuOut;
}

uint16 CMainConfig::GetThreadTimeCheck()
{
	return m_u2ThreadTimeCheck;
}

uint16 CMainConfig::GetPacketTimeOut()
{
	return m_u2PacketTimeOut;
}

uint16 CMainConfig::GetSendAliveTime()
{
	return m_u2SendAliveTime;
};

uint16 CMainConfig::GetMaxHandlerCount()
{
	return m_u2MaxHanderCount;
}

uint16 CMainConfig::GetMaxConnectTime()
{
	return m_u2MaxConnectTime;
}

uint8 CMainConfig::GetConsoleSupport()
{
	return m_u1ConsoleSupport;
}

int CMainConfig::GetConsolePort()
{
	return m_nConsolePort;
}

const char* CMainConfig::GetConsoleIP()
{
	return m_szConsoleIP;
}

uint16 CMainConfig::GetRecvQueueTimeout()
{
	return m_u2RecvQueueTimeout;
}

uint16 CMainConfig::GetSendQueueTimeout()
{
	return m_u2SendQueueTimeout;
}

uint16 CMainConfig::GetSendQueueCount()
{
	return m_u2SendQueueCount;
}

bool CMainConfig::CompareConsoleClinetIP(const char* pConsoleClientIP)
{
	for(int i = 0; i < (int)m_vecConsoleClientIP.size(); i++)
	{
		if(ACE_OS::strcmp(m_vecConsoleClientIP[i].m_szServerIP, pConsoleClientIP) == 0)
		{
			return true;
		}
	}

	return false;
}

uint16 CMainConfig::GetValidConnectCount()
{
	return m_u2ValidConnectCount;
}

uint8 CMainConfig::GetValid()
{
	return m_u1Valid;
}

uint32 CMainConfig::GetValidPacketCount()
{
	return m_u4ValidPacketCount;
}

uint32 CMainConfig::GetValidRecvSize()
{
	return m_u4ValidRecvSize;
}

uint16 CMainConfig::GetForbiddenTime()
{
	return m_u2ForbiddenTime;
}

uint8 CMainConfig::GetCommandAccount()
{
	return m_u1CommandAccount;
}


