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
	OUR_DEBUG((LM_INFO, "[CMainConfig::Init]Filename = %s.\n", szConfigPath));
	if(!m_AppConfig.ReadConfig(szConfigPath))
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "%s", m_AppConfig.GetError());
		return false;
	}

	ACE_TString strValue;

	//获得反应器个数(目前这个读取废弃)
	//m_AppConfig.GetValue("ReactorCount", strValue, "\\REACTOR");
	//m_u4ReactorCount = ACE_OS::atoi((char*)strValue.c_str());

	m_AppConfig.GetValue("ServerID", strValue, "\\SERVER");
	m_nServerID = ACE_OS::atoi((char*)strValue.c_str());

	m_AppConfig.GetValue("ServerName", strValue, "\\SERVER");
	sprintf_safe(m_szServerName, MAX_BUFF_20, "%s", strValue.c_str());

	m_AppConfig.GetValue("ListenPortCount", strValue, "\\SERVER");
	int nServerCount = ACE_OS::atoi((char*)strValue.c_str());

	//获得监听端口信息
	char szName1[MAX_BUFF_20] = {'\0'};
	char szName2[MAX_BUFF_20] = {'\0'};
	_ServerInfo serverinfo;

	m_vecServerInfo.clear();
	for(int i = 0; i < nServerCount; i++)
	{
		sprintf_safe(szName1, MAX_BUFF_20, "ServerIP%d", i);
		sprintf_safe(szName2, MAX_BUFF_20, "ServerPort%d", i);
		m_AppConfig.GetValue(szName1, strValue, "\\SERVER");
		sprintf_safe(serverinfo.m_szServerIP, MAX_BUFF_20, "%s", strValue.c_str());

		m_AppConfig.GetValue(szName2, strValue, "\\SERVER");
		serverinfo.m_nPort = ACE_OS::atoi((char*)strValue.c_str());

		m_vecServerInfo.push_back(serverinfo);
	}

	//开始获得消息处理线程参数
	m_AppConfig.GetValue("Msg_High_mark", strValue, "\\SERVER");
	m_u4MsgHighMark = (uint32)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("Msg_Low_mark", strValue, "\\SERVER");
	m_u4MsgLowMark = (uint32)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("Msg_Thread", strValue, "\\SERVER");
	m_u4MsgThreadCount = (uint32)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("Msg_MaxQueue", strValue, "\\SERVER");
	m_u4MsgMaxQueue = (uint32)ACE_OS::atoi((char*)strValue.c_str());

	//开始获得UDP服务器相关参数
	m_AppConfig.GetValue("ListenPortCount", strValue, "\\UDPSERVER");
	int nUDPServerCount = ACE_OS::atoi((char*)strValue.c_str());

	m_vecUDPServerInfo.clear();
	for(int i = 0; i < nUDPServerCount; i++)
	{
		sprintf_safe(szName1, MAX_BUFF_20, "ServerIP%d", i);
		sprintf_safe(szName2, MAX_BUFF_20, "ServerPort%d", i);
		m_AppConfig.GetValue(szName1, strValue, "\\UDPSERVER");
		sprintf_safe(serverinfo.m_szServerIP, MAX_BUFF_20, "%s", strValue.c_str());

		m_AppConfig.GetValue(szName2, strValue, "\\UDPSERVER");
		serverinfo.m_nPort = ACE_OS::atoi((char*)strValue.c_str());

		m_vecUDPServerInfo.push_back(serverinfo);
	}

	//开始获得加载模块参数
	m_AppConfig.GetValue("ModulePath", strValue, "\\SERVER");
	sprintf_safe(m_szModulePath, MAX_BUFF_200, "%s", strValue.c_str());
	m_AppConfig.GetValue("ModuleString", strValue, "\\SERVER");
	sprintf_safe(m_szResourceName, MAX_BUFF_200, "%s", strValue.c_str());

	//开始获得加密模式参数
	m_AppConfig.GetValue("EncryptFlag", strValue, "\\SERVER");
	m_nEncryptFlag = (int)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("EncryptPass", strValue, "\\SERVER");
	sprintf_safe(m_szEncryptPass, MAX_BUFF_9, "%s", strValue.c_str());
	m_AppConfig.GetValue("EncryptOutFlag", strValue, "\\SERVER");
	m_nEncryptOutFlag = (int)ACE_OS::atoi((char*)strValue.c_str());

	//开始获得发送和接受阀值
	m_AppConfig.GetValue("SendThresHold", strValue, "\\SERVER");
	m_u4SendThresHold = (int)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("RecvBuffSize", strValue, "\\SERVER");
	m_u4RecvBuffSize = (int)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("SendQueueMax", strValue, "\\SERVER");
	m_u2SendQueueMax = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ThreadTimeout", strValue, "\\SERVER");
	m_u2ThreadTimuOut = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ThreadTimeCheck", strValue, "\\SERVER");
	m_u2ThreadTimeCheck = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("DisposeTimeout", strValue, "\\SERVER");
	m_u2PacketTimeOut = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("SendAliveTime", strValue, "\\SERVER");
	m_u2SendAliveTime = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("HandlerCount", strValue, "\\SERVER");
	m_u2HandleCount = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("MaxHandlerCount", strValue, "\\SERVER");
	m_u2MaxHanderCount = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("MaxConnectTime", strValue, "\\SERVER");
	m_u2MaxConnectTime = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("RecvQueueTimeout", strValue, "\\SERVER");
	m_u2RecvQueueTimeout = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("SendQueueTimeout", strValue, "\\SERVER");
	m_u2SendQueueTimeout = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("SendQueueCount", strValue, "\\SERVER");
	m_u2SendQueueCount = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("CommandAccount", strValue, "\\SERVER");
	m_u1CommandAccount = (uint8)ACE_OS::atoi((char*)strValue.c_str());

	//开始获得Console服务器相关配置信息
	m_AppConfig.GetValue("ConsoleSupport", strValue, "\\SERVER");
	m_u1ConsoleSupport = (uint8)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ConsoleIP", strValue, "\\SERVER");
	sprintf_safe(m_szConsoleIP, MAX_BUFF_100, "%s", strValue.c_str());
	m_AppConfig.GetValue("ConsolePort", strValue, "\\SERVER");
	m_nConsolePort = (int)ACE_OS::atoi((char*)strValue.c_str());

	m_AppConfig.GetValue("ConsoleIPCount", strValue, "\\SERVER");
	int nConsoleClientIPCount = (int)ACE_OS::atoi((char*)strValue.c_str());

	m_vecConsoleClientIP.clear();
	for(int i = 0; i < nConsoleClientIPCount; i++)
	{
		_ConsoleClientIP ConsoleClientIP;
		sprintf_safe(szName1, MAX_BUFF_20, "ConsoleIP%d", i);
		m_AppConfig.GetValue(szName1, strValue, "\\SERVER");
		sprintf_safe(ConsoleClientIP.m_szServerIP, MAX_BUFF_20, "%s", strValue.c_str());

		m_vecConsoleClientIP.push_back(ConsoleClientIP);
	}

	//开始获得ConnectValid对应的参数
	m_AppConfig.GetValue("ConnectCount", strValue, "\\CONNECTVALID");
	m_u2ValidConnectCount = (uint16)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ConnectValid", strValue, "\\CONNECTVALID");
	m_u1Valid = (uint8)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ConnectPacketCount", strValue, "\\CONNECTVALID");
	m_u4ValidPacketCount = (uint32)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ConnectRecvSize", strValue, "\\CONNECTVALID");
	m_u4ValidRecvSize = (uint32)ACE_OS::atoi((char*)strValue.c_str());
	m_AppConfig.GetValue("ConnectRecvSize", strValue, "\\CONNECTVALID");
	m_u2ForbiddenTime = (uint16)ACE_OS::atoi((char*)strValue.c_str());
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


