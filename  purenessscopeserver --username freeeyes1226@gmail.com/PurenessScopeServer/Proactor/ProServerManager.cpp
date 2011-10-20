#include "ProServerManager.h"

CProServerManager::CProServerManager(void)
{
}

CProServerManager::~CProServerManager(void)
{
}

bool CProServerManager::Init()
{
	int nServerPortCount    = App_MainConfig::instance()->GetServerPortCount();
	int nUDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();
	int nReactorCount       = App_MainConfig::instance()->GetReactorCount();

	bool blState = false;

	//��ʼ����ֹIP�б�
	App_ForbiddenIP::instance()->Init(FORBIDDENIP_FILE);

	//��ʼ��������
	if(!m_ConnectAcceptorManager.InitConnectAcceptor(nServerPortCount))
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Init]%s.\n", m_ConnectAcceptorManager.GetError()));
		return false;
	}

	//��ʼ����Ӧ��
	for(int i = 0 ; i < nReactorCount + 1; i++)
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Init()]... i=[%d].\n",i));
		if(i == 0)
		{
			//�������ֲ���ϵͳ�汾��ʹ�ò�ͬ�ķ�Ӧ��
#ifdef WIN32
			blState = App_ProactorManager::instance()->AddNewProactor(REACTOR_CLIENTDEFINE, Proactor_WIN32, 0);
			OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewProactor REACTOR_CLIENTDEFINE = Proactor_WIN32.\n"));
#else
			blState = App_ProactorManager::instance()->AddNewProactor(REACTOR_CLIENTDEFINE, Proactor_POSIX, 0);
			OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewProactor REACTOR_CLIENTDEFINE = Proactor_POSIX.\n"));	
#endif		
			if(!blState)
			{
				OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE Error.\n"));
				return false;
			}
		}
		else
		{
#ifdef WIN32
			blState = App_ProactorManager::instance()->AddNewProactor(i, Proactor_WIN32, 1);
			OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Proactor_WIN32.\n"));
#else
			blState = App_ProactorManager::instance()->AddNewProactor(i, Proactor_POSIX, 1);
			OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewReactor REACTOR_CLIENTDEFINE = Proactor_POSIX.\n"));
#endif
			if(!blState)
			{
				OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewReactor [%d] Error.\n", i));
				return false;
			}
		}
	}

	//��ʼ����־ϵͳ�߳�
	CFileLogger* pFileLogger = new CFileLogger();
	if(NULL == pFileLogger)
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Init]pFileLogger new is NULL.\n"));
		return false;
	}

	pFileLogger->Init();
	AppLogManager::instance()->Init();
	if(0 != AppLogManager::instance()->RegisterLog(pFileLogger))
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AppLogManager::instance()->RegisterLog error.\n"));
		return false;
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AppLogManager is OK.\n"));
	}

	//��ʼ��BuffPacket�����
	App_BuffPacketManager::instance()->Init(BUFFPACKET_MAX_COUNT);

	//��ʼ��PacketParse�����
	App_PacketParsePool::instance()->Init(MAX_PACKET_PARSE);

	//��ʼ��MakePacket�����
	App_MakePacket::instance()->Init();

	//��ʼ��Message�����
	App_MessagePool::instance()->Init(MAX_MESSAGE_POOL);

	//��ʼ��ProConnectHandler�����
	if(App_MainConfig::instance()->GetHandleCount() <= 0)
	{
		App_ProConnectHandlerPool::instance()->Init(MAX_HANDLE_POOL);
	}
	else
	{
		App_ProConnectHandlerPool::instance()->Init(App_MainConfig::instance()->GetHandleCount());
	}

	//��ʼ����Ϣ�����߳�
	App_MessageService::instance()->Init(App_MainConfig::instance()->GetThreadCount(), App_MainConfig::instance()->GetMsgMaxQueue(), App_MainConfig::instance()->GetMgsHighMark(), App_MainConfig::instance()->GetMsgLowMark());

	//��ʼ����DLL�Ķ���ӿ�
	App_ServerObject::instance()->SetMessageManager((IMessageManager* )App_MessageManager::instance());
	App_ServerObject::instance()->SetLogManager((ILogManager*  )AppLogManager::instance());
	App_ServerObject::instance()->SetConnectManager((IConnectManager* )App_ProConnectManager::instance());
	App_ServerObject::instance()->SetPacketManager((IPacketManager* )App_BuffPacketManager::instance());
	App_ServerObject::instance()->SetClientManager((IClientManager* )App_ClientProConnectManager::instance());
	App_ServerObject::instance()->SetUDPConnectManager((IUDPConnectManager* )App_ProUDPManager::instance());

	//��ʼ��ģ�����
	blState = App_ModuleLoader::instance()->LoadModule(App_MainConfig::instance()->GetModulePath(), App_MainConfig::instance()->GetModuleString());
	if(false == blState)
	{
		return false;
	}

	return true;
}

bool CProServerManager::Start()
{
	//ע���ź���
	//if(0 != App_SigHandler::instance()->RegisterSignal(NULL))
	//{
	//	return false;
	//}

	//����TCP����
	int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();
	bool blState = false;

	for(int i = 0 ; i < nServerPortCount; i++)
	{
		ACE_INET_Addr listenAddr;

		_ServerInfo* pServerInfo = App_MainConfig::instance()->GetServerPort(i);
		if(NULL == pServerInfo)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]pServerInfo [%d] is NULL.\n", i));
			return false;
		}

		int nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP);
		if(nErr != 0)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start](%d)set_address error[%d].\n", i, errno));
			return false;
		}

		//�õ�������
		ProConnectAcceptor* pConnectAcceptor = m_ConnectAcceptorManager.GetConnectAcceptor(i);

		if(NULL == pConnectAcceptor)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]pConnectAcceptor[%d] is NULL.\n", i));
			return false;
		}

		ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);
		if(NULL == pProactor)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
			return false;
		}

		int nRet = pConnectAcceptor->open(listenAddr, 0, 1, MAX_ASYNCH_BACKLOG, 1, pProactor);
		if(-1 == nRet)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] pConnectAcceptor->open[%d] is error.\n", i));
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] error(%d).\n",listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
			return false;
		}
		OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
	}

	//����UDP����
	int nUDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();
	for(int i = 0 ; i < nUDPServerPortCount; i++)
	{
		ACE_INET_Addr listenAddr;

		_ServerInfo* pServerInfo = App_MainConfig::instance()->GetUDPServerPort(i);
		if(NULL == pServerInfo)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]UDP pServerInfo [%d] is NULL.\n", i));
			return false;
		}


		CProactorUDPHandler* pProactorUDPHandler = App_ProUDPManager::instance()->Create();
		if(NULL == pProactorUDPHandler)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] pProactorUDPHandler is NULL[%d] is error.\n", i));
			return false;
		}
		else
		{
			int nErr = listenAddr.set(pServerInfo->m_nPort, pServerInfo->m_szServerIP);
			if(nErr != 0)
			{
				OUR_DEBUG((LM_INFO, "[CProServerManager::Start](%d)UDP set_address error[%d].\n", i, errno));
				return false;
			}

			ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);
			if(NULL == pProactor)
			{
				OUR_DEBUG((LM_INFO, "[CProServerManager::Start]UDP App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
				return false;
			}

			if(0 != pProactorUDPHandler->OpenAddress(listenAddr, pProactor))
			{
				OUR_DEBUG((LM_INFO, "[CProServerManager::Start] UDP Listen from [%s:%d] error(%d).\n",listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
				return false;
			}

			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] UDP Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
		}
	}

	//������̨����˿ڼ���
	if(App_MainConfig::instance()->GetConsoleSupport() == CONSOLE_ENABLE)
	{
		ACE_INET_Addr listenConsoleAddr;

		int nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), App_MainConfig::instance()->GetConsoleIP());
		if(nErr != 0)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]listenConsoleAddr set_address error[%d].\n", errno));
			return false;
		}

		ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);
		if(NULL == pProactor)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
			return false;
		}

		int nRet = m_ProConsoleConnectAcceptor.open(listenConsoleAddr, 0, 1, MAX_ASYNCH_BACKLOG, 1, pProactor, true);
		if(-1 == nRet)
		{
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] m_ProConsoleConnectAcceptor.open is error.\n"));
			OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] error(%d).\n",listenConsoleAddr.get_host_addr(), listenConsoleAddr.get_port_number(), errno));
			return false;
		}
	}

	//������־�����߳�
	if(0 != AppLogManager::instance()->Start())
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CProServerManager::Init]AppLogManager is ERROR.");
	}
	else
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CProServerManager::Init]AppLogManager is OK.");
	}
	

	//������ʱ��
	if(0 != App_TimerManager::instance()->activate())
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_TimerManager::instance()->Start() is error.\n"));
		return false;
	}

	//������������Proactor���������ԭʼ��Proactor����Ϊԭʼ�Ļ�����̣߳������������һ�¡�
	if(!App_ProactorManager::instance()->StartProactor())
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->StartProactor is error.\n"));
		return false;
	}

	//�����м���������ӹ�����
	App_ClientProConnectManager::instance()->Init(App_ProactorManager::instance()->GetAce_Proactor(REACTOR_POSTDEFINE));
	App_ClientProConnectManager::instance()->StartConnectTask(CONNECT_LIMIT_RETRY);

	//��ʼ��Ϣ���ֽ��߳�
	App_MakePacket::instance()->Start();

	//��ʼ��Ϣ�����߳�
	App_MessageService::instance()->Start();

	//��ʼ�������ӷ��Ͷ�ʱ��
	App_ProConnectManager::instance()->StartTimer();

	//���������Ӧ��
	OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->StartProactorDefault begin....\n"));
	if(!App_ProactorManager::instance()->StartProactorDefault())
	{
		OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->StartProactorDefault is error.\n"));
		return false;
	}

	return true;
}

bool CProServerManager::Close()
{
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close begin....\n"));
	App_ProUDPManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close UDP OK.\n"));
	App_MakePacket::instance()->Close();
	OUR_DEBUG((LM_INFO, "[App_MessageService::Close]Close App_MakePacket OK.\n"));
	App_MessageService::instance()->Close();
	OUR_DEBUG((LM_INFO, "[App_MessageService::Close]Close App_MessageService OK.\n"));
	App_TimerManager::instance()->close();
	App_ProConnectManager::instance()->CloseAll();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ConnectManager OK.\n"));
	App_ClientProConnectManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ClientReConnectManager OK.\n"));
	AppLogManager::instance()->Close();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]AppLogManager OK\n"));
	App_ProactorManager::instance()->StopProactor();
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ReactorManager OK.\n"));	
	OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close end....\n"));

	return true;
}
