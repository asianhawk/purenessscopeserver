#include "ServerManager.h"

#include "Frame_Logging_Strategy.h"

CServerManager::CServerManager(void)
{
	m_pFileLogger           = NULL;
	m_pFrameLoggingStrategy = NULL;
}

CServerManager::~CServerManager(void)
{
	OUR_DEBUG((LM_INFO, "[CServerManager::~CServerManager].\n"));
}

bool CServerManager::Init()
{

	if(App_MainConfig::instance()->GetDebugTrunOn() == 1)
	{
		m_pFrameLoggingStrategy = new Frame_Logging_Strategy();

		//�Ƿ��ACE_DEBUG�ļ��洢
		Logging_Config_Param objParam;

		sprintf_safe(objParam.m_strLogFile, 256, "%s", App_MainConfig::instance()->GetDebugFileName());
		objParam.m_iChkInterval    = App_MainConfig::instance()->GetChkInterval();
		objParam.m_iLogFileMaxCnt  = App_MainConfig::instance()->GetLogFileMaxCnt();
		objParam.m_iLogFileMaxSize = App_MainConfig::instance()->GetLogFileMaxSize();
		sprintf_safe(objParam.m_strLogLevel, 128, "%s", App_MainConfig::instance()->GetDebugLevel());

		m_pFrameLoggingStrategy->InitLogStrategy(objParam);
	}

	int nServerPortCount    = App_MainConfig::instance()->GetServerPortCount();
	int nReactorCount       = App_MainConfig::instance()->GetReactorCount();
	bool blState = false;
	//��ʼ����ֹIP�б�
	App_ForbiddenIP::instance()->Init(FORBIDDENIP_FILE);

	//��ʼ��������
	if (!App_ConnectAcceptorManager::instance()->InitConnectAcceptor(nServerPortCount))
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Init]%s.\n", App_ConnectAcceptorManager::instance()->GetError()));
		return false;
	}

	OUR_DEBUG((LM_INFO, "[CServerManager::Init]nReactorCount=%d.\n", nReactorCount));

	//��ʼ����Ӧ��
	for (int i = 0 ; i < nReactorCount; i++)
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Init()]... i=[%d].\n", i));

		if (App_MainConfig::instance()->GetNetworkMode() == NETWORKMODE_RE_SELECT)
		{
			blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_Select, 1);
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_Select.\n"));
		}
		else if (App_MainConfig::instance()->GetNetworkMode() == NETWORKMODE_RE_TPSELECT)
		{
			blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_TP, 1);
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_TP.\n"));
		}
		else if (App_MainConfig::instance()->GetNetworkMode() == NETWORKMODE_RE_EPOLL)
		{
			blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_DEV_POLL, 1, App_MainConfig::instance()->GetHandleCount());
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_DEV_POLL.\n"));
		}
		else if (App_MainConfig::instance()->GetNetworkMode() == NETWORKMODE_RE_EPOLL_ET)
		{
			blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_DEV_POLL_ET, 1, App_MainConfig::instance()->GetHandleCount());
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_DEV_POLL_ET.\n"));
		}
		else
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewProactor NETWORKMODE Error.\n"));
			return false;
		}

		if (!blState)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]AddNewReactor [%d] Error.\n", i));
			OUR_DEBUG((LM_INFO, "[CServerManager::Init]Error=%s.\n", App_ReactorManager::instance()->GetError()));
			return false;
		}
	}

	//��ʼ����־ϵͳ�߳�
	m_pFileLogger = new CFileLogger();

	if (NULL == m_pFileLogger)
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Init]pFileLogger new is NULL.\n"));
		return false;
	}

	m_pFileLogger->Init();
	AppLogManager::instance()->Init();

	if (0 != AppLogManager::instance()->RegisterLog(m_pFileLogger))
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Init]AppLogManager::instance()->RegisterLog error.\n"));
		return false;
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Init]AppLogManager is OK.\n"));
	}

	//��ʼ��������ϵͳ
	App_IPAccount::instance()->Init(App_MainConfig::instance()->GetIPAlert()->m_u4IPMaxCount, App_MainConfig::instance()->GetTrackIPCount());

	App_ConnectAccount::instance()->Init(App_MainConfig::instance()->GetConnectAlert()->m_u4ConnectMin,
		App_MainConfig::instance()->GetConnectAlert()->m_u4ConnectMax,
		App_MainConfig::instance()->GetConnectAlert()->m_u4DisConnectMin,
		App_MainConfig::instance()->GetConnectAlert()->m_u4DisConnectMax);	

	//��ʼ��BuffPacket�����.Ĭ�϶��ǵ�ǰ�����������2��
	App_BuffPacketManager::instance()->Init(BUFFPACKET_MAX_COUNT, App_MainConfig::instance()->GetByteOrder());

	//��ʼ��PacketParse�����
	App_PacketParsePool::instance()->Init(MAX_PACKET_PARSE);

	//��ʼ��MakePacket�����
	App_MakePacket::instance()->Init();

	//��ʼ��Message�����
	App_MessagePool::instance()->Init(MAX_MESSAGE_POOL);

	//��ʼ��ConnectHandler�����
	if (App_MainConfig::instance()->GetHandleCount() <= 0)
	{
		App_ConnectHandlerPool::instance()->Init(MAX_HANDLE_POOL);
	}
	else
	{
		App_ConnectHandlerPool::instance()->Init(App_MainConfig::instance()->GetHandleCount());
	}

	//��ʼ�����ӹ�����
	App_ConnectManager::instance()->Init(App_MainConfig::instance()->GetSendQueueCount());
	//��ʼ����Ϣ�����߳�
	App_MessageServiceGroup::instance()->Init(App_MainConfig::instance()->GetThreadCount(), App_MainConfig::instance()->GetMsgMaxQueue(), App_MainConfig::instance()->GetMgsHighMark(), App_MainConfig::instance()->GetMsgLowMark());
	//��ʼ����DLL�Ķ���ӿ�
	App_ServerObject::instance()->SetMessageManager((IMessageManager*)App_MessageManager::instance());
	App_ServerObject::instance()->SetLogManager((ILogManager*)AppLogManager::instance());
	App_ServerObject::instance()->SetConnectManager((IConnectManager*)App_ConnectManager::instance());
	App_ServerObject::instance()->SetPacketManager((IPacketManager*)App_BuffPacketManager::instance());
	App_ServerObject::instance()->SetClientManager((IClientManager*)App_ClientReConnectManager::instance());
	App_ServerObject::instance()->SetUDPConnectManager((IUDPConnectManager*)App_ReUDPManager::instance());
	App_ServerObject::instance()->SetTimerManager((ActiveTimer*)App_TimerManager::instance());
	App_ServerObject::instance()->SetModuleMessageManager((IModuleMessageManager*)App_ModuleMessageManager::instance());
	App_ServerObject::instance()->SetControlListen((IControlListen*)App_ControlListen::instance());
	return true;
}

bool CServerManager::Start()
{
	if (0 != App_SigHandler::instance()->RegisterSignal(ACE_Reactor::instance()))
	{
		return false;
	}	

	//����TCP����
	int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();

	for (int i = 0 ; i < nServerPortCount; i++)
	{
		ACE_INET_Addr listenAddr;
		_ServerInfo* pServerInfo = App_MainConfig::instance()->GetServerPort(i);

		if (NULL == pServerInfo)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]pServerInfo [%d] is NULL.\n", i));
			return false;
		}

		//�ж�IPv4����IPv6
		int nErr = 0;

		if (pServerInfo->m_u1IPType == TYPE_IPV4)
		{
			nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP);
		}
		else
		{
			nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP, 1, PF_INET6);
		}

		if (nErr != 0)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start](%d)set_address error[%d].\n", i, errno));
			return false;
		}

		//�õ�������
		ConnectAcceptor* pConnectAcceptor = App_ConnectAcceptorManager::instance()->GetConnectAcceptor(i);

		if (NULL == pConnectAcceptor)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]pConnectAcceptor[%d] is NULL.\n", i));
			return false;
		}

		int nRet = pConnectAcceptor->open2(listenAddr, App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE), ACE_NONBLOCK, (int)App_MainConfig::instance()->GetBacklog());

		if (-1 == nRet)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start] pConnectAcceptor->open[%d] is error.\n", i));
			OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] error(%d).\n", listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
			return false;
		}

		OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
	}

	//����UDP����
	int nUDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();

	for (int i = 0 ; i < nUDPServerPortCount; i++)
	{
		ACE_INET_Addr listenAddr;
		_ServerInfo* pServerInfo = App_MainConfig::instance()->GetUDPServerPort(i);

		if (NULL == pServerInfo)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]UDP pServerInfo [%d] is NULL.\n", i));
			return false;
		}

		int nErr = 0;

		if (pServerInfo->m_u1IPType == TYPE_IPV4)
		{
			nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP);
		}
		else
		{
			nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP, 1, PF_INET6);
		}

		if (nErr != 0)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]UDP (%d)set_address error[%d].\n", i, errno));
			return false;
		}

		//�õ�������
		CReactorUDPHander* pReactorUDPHandler = App_ReUDPManager::instance()->Create();

		if (NULL == pReactorUDPHandler)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]UDP pReactorUDPHandler[%d] is NULL.\n", i));
			return false;
		}

		int nRet = pReactorUDPHandler->OpenAddress(listenAddr, App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE));

		if (-1 == nRet)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start] UDP Listen from [%s:%d] error(%d).\n", listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
			return false;
		}

		OUR_DEBUG((LM_INFO, "[CServerManager::Start] UDP Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
	}

	//������̨����˿ڼ���
	if (App_MainConfig::instance()->GetConsoleSupport() == CONSOLE_ENABLE)
	{
		ACE_INET_Addr listenConsoleAddr;
		int nErr = 0;

		if (App_MainConfig::instance()->GetConsoleIPType() == TYPE_IPV4)
		{
			nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), App_MainConfig::instance()->GetConsoleIP());
		}
		else
		{
			nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), App_MainConfig::instance()->GetConsoleIP(), 1, PF_INET6);
		}

		if (nErr != 0)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start]listenConsoleAddr set_address error[%d].\n", errno));
			return false;
		}

		int nRet = m_ConnectConsoleAcceptor.open(listenConsoleAddr, App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE), ACE_NONBLOCK);

		if (-1 == nRet)
		{
			OUR_DEBUG((LM_INFO, "[CServerManager::Start] pConnectAcceptor->open is error.\n"));
			OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] error(%d).\n", listenConsoleAddr.get_host_addr(), listenConsoleAddr.get_port_number(), errno));
			return false;
		}
	}

	//������־�����߳�
	if (0 != AppLogManager::instance()->Start())
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CServerManager::Init]AppLogManager is ERROR.");
	}
	else
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CServerManager::Init]AppLogManager is OK.");
	}

	//������ʱ��
	if (0 != App_TimerManager::instance()->activate())
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_TimerManager::instance()->Start() is error.\n"));
		return false;
	}

	//�����м���������ӹ�����
	App_ClientReConnectManager::instance()->Init(App_ReactorManager::instance()->GetAce_Reactor(REACTOR_POSTDEFINE));
	App_ClientReConnectManager::instance()->StartConnectTask(App_MainConfig::instance()->GetConnectServerCheck());

	//��ʼ��ģ����أ���Ϊ������ܰ������м���������Ӽ���
	bool blState = App_ModuleLoader::instance()->LoadModule(App_MainConfig::instance()->GetModulePath(), App_MainConfig::instance()->GetModuleString());

	if (false == blState)
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Start]LoadModule is error.\n"));
		return false;
	}

	//��ʼ��Ϣ�����߳�
	App_MessageServiceGroup::instance()->Start();
	//��ʼ�������ӷ��Ͷ�ʱ��
	App_ConnectManager::instance()->StartTimer();

	//��������Reactor����
	if (!App_ReactorManager::instance()->StartReactor())
	{
		OUR_DEBUG((LM_INFO, "[CServerManager::Start]App_ReactorManager::instance()->StartReactor is error.\n"));
		return false;
	}    

	//���������Ӧ��
	OUR_DEBUG((LM_INFO, "[CServerManager::Start]App_ReactorManager::instance()->StartReactorDefault begin....\n"));

	ACE_Thread_Manager::instance()->wait();

	return true;
}

bool CServerManager::Close()
{
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close begin....\n"));
	App_ConnectAcceptorManager::instance()->Close();
	m_ConnectConsoleAcceptor.close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_TimerManager OK.\n"));
	App_TimerManager::instance()->deactivate();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ReUDPManager OK.\n"));
	App_ReUDPManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ModuleLoader OK.\n"));
	App_ClientReConnectManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ClientReConnectManager OK.\n"));
	App_ModuleLoader::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_MessageManager OK.\n"));
	App_MessageManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ConnectManager OK.\n"));
	App_MessageServiceGroup::instance()->Close();
	OUR_DEBUG((LM_INFO, "[App_MessageServiceGroup::Close]Close App_MessageServiceGroup OK.\n"));
	App_ConnectManager::instance()->CloseAll();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ConnectManager OK.\n"));
	AppLogManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]AppLogManager OK\n"));
	App_BuffPacketManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]BuffPacketManager OK\n"));
	App_ReactorManager::instance()->StopReactor();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ReactorManager OK.\n"));	
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close end....\n"));


	m_pFrameLoggingStrategy->EndLogStrategy();
	SAFE_DELETE(m_pFrameLoggingStrategy);

	return true;
}
