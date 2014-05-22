#include "ClientReConnectManager.h"

CReactorClientInfo::CReactorClientInfo()
{
    m_pConnectClient    = NULL;
    m_pClientMessage    = NULL;
    m_pReactorConnect   = NULL;
    m_nServerID         = 0;
    m_emConnectState    = SERVER_CONNECT_READY;
}

CReactorClientInfo::~CReactorClientInfo()
{
    OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::~CReactorClientInfo].\n"));
}

bool CReactorClientInfo::Init(int nServerID, const char* pIP, int nPort, uint8 u1IPType, CConnectClientConnector* pReactorConnect, IClientMessage* pClientMessage, ACE_Reactor* pReactor)
{
    int nRet = 0;

    if (u1IPType == TYPE_IPV4)
    {
        nRet = m_AddrServer.set(nPort, pIP);
    }
    else
    {
        nRet = m_AddrServer.set(nPort, pIP, 1, PF_INET6);
    }

    if (-1 == nRet)
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Init]adrClient(%s:%d) error.\n", pIP, nPort));
        return false;
    }

    m_pClientMessage    = pClientMessage;
    m_pReactorConnect   = pReactorConnect;
    m_nServerID         = nServerID;
    m_pReactor          = pReactor;
    return true;
}

bool CReactorClientInfo::Run(bool blIsReady, EM_Server_Connect_State emState)
{
    if (NULL != m_pConnectClient)
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Run]Connect is exist.\n"));
        return false;
    }

    if (NULL == m_pReactorConnect)
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Run]m_pAsynchConnect is NULL.\n"));
        return false;
    }

    m_pConnectClient = new CConnectClient();

    if (NULL == m_pConnectClient)
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Run]pConnectClient new is NULL.\n"));
        return false;
    }

    m_pConnectClient->SetServerID(m_nServerID);
    m_pConnectClient->reactor(m_pReactor);
    m_emConnectState = emState;

    if (blIsReady == true)
    {
        if (m_pReactorConnect->connect(m_pConnectClient, m_AddrServer) == -1)
        {
            m_emConnectState = SERVER_CONNECT_FAIL;
            OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Run](%s:%d) connection fails(ServerID=%d) error(%d).\n", m_AddrServer.get_host_addr(), m_AddrServer.get_port_number(), m_nServerID, ACE_OS::last_error()));
            //��������ΪTrue��Ϊ�����Զ�����������
            return true;
        }
    }

    return true;
}

bool CReactorClientInfo::SendData(ACE_Message_Block* pmblk)
{
    if (NULL == m_pConnectClient)
    {
        //������Ӳ����ڣ��������ӡ�
        Run(true);

        if (NULL != pmblk)
        {
            pmblk->release();
        }

        //�����Ϣ�д����ӿڣ��򷵻�ʧ�ܽӿ�
        if (NULL != m_pClientMessage)
        {
            //�������Ѿ��Ͽ�����Ҫ�ȴ��������ӵĽ��
            m_pClientMessage->ConnectError(101);
        }

        return false;
    }
    else
    {
        //��������
        return m_pConnectClient->SendData(pmblk);
    }
}

int CReactorClientInfo::GetServerID()
{
    return m_nServerID;
}

bool CReactorClientInfo::Close()
{
    //OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Close]Begin.\n"));
    if (NULL == m_pConnectClient)
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Close]End 1.\n", m_pClientMessage));
        m_pClientMessage = NULL;
        return false;
    }
    else
    {
        OUR_DEBUG((LM_ERROR, "[CReactorClientInfo::Close]End 2.\n"));
        m_pClientMessage = NULL;
        m_pConnectClient->SetClientMessage(NULL);
        m_pConnectClient->ClinetClose();
        return true;
    }
}

void CReactorClientInfo::SetConnectClient(CConnectClient* pConnectClient)
{
    m_pConnectClient = pConnectClient;
}

CConnectClient* CReactorClientInfo::GetConnectClient()
{
    return m_pConnectClient;
}

IClientMessage* CReactorClientInfo::GetClientMessage()
{
    //���������Ƿ��������������ж�
    if (m_emConnectState == SERVER_CONNECT_RECONNECT && NULL != m_pClientMessage)
    {
        //֪ͨ�ϲ�ĳһ�������Ѿ��ָ�
        m_pClientMessage->ReConnect(m_nServerID);
    }

    m_emConnectState = SERVER_CONNECT_OK;
    return m_pClientMessage;
}

ACE_INET_Addr CReactorClientInfo::GetServerAddr()
{
    return m_AddrServer;
}

EM_Server_Connect_State CReactorClientInfo::GetServerConnectState()
{
    return m_emConnectState;
}

void CReactorClientInfo::SetServerConnectState(EM_Server_Connect_State objState)
{
    m_emConnectState = objState;
}

CClientReConnectManager::CClientReConnectManager(void)
{
    m_nTaskID         = -1;
    m_pReactor        = NULL;
    m_blReactorFinish = false;
}

CClientReConnectManager::~CClientReConnectManager(void)
{
    OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::~CClientReConnectManager].\n"));
    //Close();
}

bool CClientReConnectManager::Init(ACE_Reactor* pReactor)
{
    if (-1 == m_ReactorConnect.open(pReactor))
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Init]open error(%d).", ACE_OS::last_error()));
        return false;
    }

    m_u4ConnectServerTimeout = App_MainConfig::instance()->GetConnectServerTimeout() * 1000; //ת��Ϊ΢��

    if (m_u4ConnectServerTimeout == 0)
    {
        m_u4ConnectServerTimeout = RE_CONNECT_SERVER_TIMEOUT;
    }

    m_pReactor        = pReactor;
    m_blReactorFinish = true;
    return true;
}

bool CClientReConnectManager::Connect(int nServerID, const char* pIP, int nPort, uint8 u1IPType, IClientMessage* pClientMessage)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f != m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Connect]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }

    //��ʼ��������Ϣ
    CReactorClientInfo* pClientInfo = new CReactorClientInfo();

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Connect]pClientInfo is NULL.\n"));
        return false;
    }

    //�����Ѿ����������ӽ�map
    m_mapConnectInfo[nServerID] = pClientInfo;

    if (false == pClientInfo->Init(nServerID, pIP, nPort, u1IPType, &m_ReactorConnect, pClientMessage, m_pReactor))
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Connect]pClientInfo Init Error.\n"));
        delete pClientInfo;
        pClientInfo = NULL;
        Close(nServerID);
        return false;
    }

    //��ʼ����
    if (false == pClientInfo->Run(m_blReactorFinish, SERVER_CONNECT_FIRST))
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Connect]Run Error.\n"));
        delete pClientInfo;
        pClientInfo = NULL;
        Close(nServerID);
        return false;
    }

    OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Connect]nServerID =(%d) connect is OK.\n", nServerID));
    return true;
}

bool CClientReConnectManager::ConnectUDP(int nServerID, const char* pIP, int nPort, uint8 u1IPType, IClientUDPMessage* pClientUDPMessage)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorUDPConnectInfo::iterator f = m_mapReactorUDPConnectInfo.find(nServerID);

    if (f != m_mapReactorUDPConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::ConnectUDP]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }

    CReactorUDPClient* pReactorUDPClient = new CReactorUDPClient();

    if (NULL == pReactorUDPClient)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::ConnectUDP]nServerID =(%d) pProactorUDPClient is NULL.\n", nServerID));
        return false;
    }

    m_mapReactorUDPConnectInfo[nServerID] = pReactorUDPClient;
    ACE_INET_Addr AddrLocal;
    int nErr = 0;

    if (u1IPType == TYPE_IPV4)
    {
        nErr = AddrLocal.set(nPort, pIP);
    }
    else
    {
        nErr = AddrLocal.set(nPort, pIP, 1, PF_INET6);
    }

    if (nErr != 0)
    {
        OUR_DEBUG((LM_INFO, "[CClientReConnectManager::ConnectUDP](%d)UDP set_address error[%d].\n", nServerID, errno));
        SAFE_DELETE(pReactorUDPClient);
        return false;
    }

    if (0 != pReactorUDPClient->OpenAddress(AddrLocal, App_ReactorManager::instance()->GetAce_Reactor(REACTOR_UDPDEFINE), pClientUDPMessage))
    {
        OUR_DEBUG((LM_INFO, "[CClientReConnectManager::ConnectUDP](%d)UDP OpenAddress error.\n", nServerID));
        SAFE_DELETE(pReactorUDPClient);
        return false;
    }

    return true;
}

bool CClientReConnectManager::SetHandler(int nServerID, CConnectClient* pConnectClient)
{
    if (NULL == pConnectClient)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::SetHandler]pProConnectClient is NULL.\n"));
        return false;
    }

    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��������ӵ��Ѿ����ڵĿͻ���map������
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::SetHandler]nServerID =(%d) is not exist.\n", nServerID));
        return false;
    }

    return true;
}

bool CClientReConnectManager::Close(int nServerID)
{
    //�������Ϊ�������Ͽ�����ֻɾ��ProConnectClient��ָ��
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is not exist.\n", nServerID));
        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    //�ر����Ӷ���
    if (NULL != pClientInfo->GetConnectClient())
    {
        pClientInfo->GetConnectClient()->ClinetClose();
    }

    pClientInfo->SetConnectClient(NULL);
    SAFE_DELETE(pClientInfo);
    //��map����ɾ����ǰ���ڵĶ���
    m_mapConnectInfo.erase(f);
    return true;
}

bool CClientReConnectManager::CloseUDP(int nServerID)
{
    //�������Ϊ�������Ͽ�����ֻɾ��ProConnectClient��ָ��
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorUDPConnectInfo::iterator f = m_mapReactorUDPConnectInfo.find(nServerID);

    if (f == m_mapReactorUDPConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::CloseUDP]nServerID =(%d) is not exist.\n", nServerID));
        return false;
    }

    CReactorUDPClient* pClientInfo = (CReactorUDPClient*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::CloseUDP]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    pClientInfo->Close();
    SAFE_DELETE(pClientInfo);
    //��map����ɾ����ǰ���ڵĶ���
    return true;
}

bool CClientReConnectManager::ConnectErrorClose(int nServerID)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::ConnectErrorClose]nServerID =(%d) is not exist.\n", nServerID));
        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::ConnectErrorClose]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    //��map����ɾ����ǰ���ڵĶ���
    m_mapConnectInfo.erase(f);
    SAFE_DELETE(pClientInfo);
    return true;
}

IClientMessage* CClientReConnectManager::GetClientMessage(int nServerID)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        return NULL;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL != pClientInfo)
    {
        return pClientInfo->GetClientMessage();
    }

    return NULL;
}

bool CClientReConnectManager::SendData(int nServerID, const char* pData, int nSize, bool blIsDelete)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CProConnectManager::SendData]nServerID =(%d) is not exist.\n", nServerID));

        if (true == blIsDelete)
        {
            SAFE_DELETE_ARRAY(pData);
        }

        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;
    ACE_Message_Block* pmblk = App_MessageBlockManager::instance()->Create(nSize);

    if (NULL == pmblk)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::SendData]nServerID =(%d) pmblk is NULL.\n", nServerID));

        if (true == blIsDelete)
        {
            SAFE_DELETE_ARRAY(pData);
        }

        return false;
    }

    ACE_OS::memcpy(pmblk->wr_ptr(), pData, nSize);
    pmblk->wr_ptr(nSize);

    if (true == blIsDelete)
    {
        SAFE_DELETE_ARRAY(pData);
    }

    //��������
    return pClientInfo->SendData(pmblk);
}

bool CClientReConnectManager::SendDataUDP(int nServerID, const char* pIP, int nPort, const char* pMessage, uint32 u4Len, bool blIsDelete)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorUDPConnectInfo::iterator f = m_mapReactorUDPConnectInfo.find(nServerID);

    if (f == m_mapReactorUDPConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CProConnectManager::Close]nServerID =(%d) is not exist.\n", nServerID));

        if (true == blIsDelete)
        {
            SAFE_DELETE_ARRAY(pMessage);
        }

        return false;
    }

    CReactorUDPClient* pClientInfo = (CReactorUDPClient*)f->second;
    //��������
    bool blSendRet = pClientInfo->SendMessage(pMessage, u4Len, pIP, nPort);

    if (true == blIsDelete)
    {
        SAFE_DELETE_ARRAY(pMessage);
    }

    return blSendRet;
}

bool CClientReConnectManager::StartConnectTask(int nIntervalTime)
{
    CancelConnectTask();
    m_nTaskID = m_ActiveTimer.schedule(this, (void*)NULL, ACE_OS::gettimeofday() + ACE_Time_Value(nIntervalTime), ACE_Time_Value(nIntervalTime));

    if (m_nTaskID == -1)
    {
        OUR_DEBUG((LM_ERROR, "[CProConnectManager::StartConnectTask].StartConnectTask is fail, time is (%d).\n", nIntervalTime));
        return false;
    }

    m_ActiveTimer.activate();
    return true;
}

void CClientReConnectManager::CancelConnectTask()
{
    if (m_nTaskID != -1)
    {
        //ɱ��֮ǰ�Ķ�ʱ�������¿����µĶ�ʱ��
        m_ActiveTimer.cancel(m_nTaskID);
        m_nTaskID = -1;
    }
}

void CClientReConnectManager::Close()
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]Begin.\n"));
    //����ж�ʱ������ɾ����ʱ��
    CancelConnectTask();

    //�ر������Ѵ��ڵ�����
    for (mapReactorConnectInfo::iterator b = m_mapConnectInfo.begin(); b != m_mapConnectInfo.end(); b++)
    {
        CReactorClientInfo* pClientInfo = (CReactorClientInfo*)b->second;
        pClientInfo->Close();
        SAFE_DELETE(pClientInfo);
    }

    for (mapReactorUDPConnectInfo::iterator ub = m_mapReactorUDPConnectInfo.begin(); ub != m_mapReactorUDPConnectInfo.end(); ub++)
    {
        CReactorUDPClient* pClientInfo = (CReactorUDPClient*)ub->second;
        pClientInfo->Close();
        SAFE_DELETE(pClientInfo);
    }

    m_mapConnectInfo.clear();
    m_mapReactorUDPConnectInfo.clear();
    OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]End.\n"));
}

int CClientReConnectManager::handle_timeout(const ACE_Time_Value& tv, const void* arg)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);

    if (arg != NULL)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::handle_timeout] arg is not NULL, tv = %d.\n", tv.sec()));
    }

    for (mapReactorConnectInfo::iterator b = m_mapConnectInfo.begin(); b != m_mapConnectInfo.end(); b++)
    {
        //int nServerID = (int)b->first;
        CReactorClientInfo* pClientInfo = (CReactorClientInfo*)b->second;

        if (NULL == pClientInfo->GetConnectClient())
        {
            //������Ӳ����ڣ������½�������
            pClientInfo->Run(m_blReactorFinish);
            //�Զ�����0.1��
            ACE_Time_Value tvSleep(0, m_u4ConnectServerTimeout);
            ACE_OS::sleep(tvSleep);
        }
    }

    return 0;
}

void CClientReConnectManager::GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    VecClientConnectInfo.clear();

    for (mapReactorConnectInfo::iterator b = m_mapConnectInfo.begin(); b != m_mapConnectInfo.end(); b++)
    {
        CReactorClientInfo* pClientInfo = (CReactorClientInfo*)b->second;

        if (NULL != pClientInfo)
        {
            if (NULL != pClientInfo->GetConnectClient())
            {
                _ClientConnectInfo ClientConnectInfo = pClientInfo->GetConnectClient()->GetClientConnectInfo();
                ClientConnectInfo.m_addrRemote = pClientInfo->GetServerAddr();
                VecClientConnectInfo.push_back(ClientConnectInfo);
            }
            else
            {
                _ClientConnectInfo ClientConnectInfo;
                ClientConnectInfo.m_blValid    = false;
                ClientConnectInfo.m_addrRemote = pClientInfo->GetServerAddr();
                VecClientConnectInfo.push_back(ClientConnectInfo);
            }
        }
    }
}

void CClientReConnectManager::GetUDPConnectInfo(vecClientConnectInfo& VecClientConnectInfo)
{
    VecClientConnectInfo.clear();

    for (mapReactorUDPConnectInfo::iterator b = m_mapReactorUDPConnectInfo.begin(); b != m_mapReactorUDPConnectInfo.end(); b++)
    {
        CReactorUDPClient* pClientInfo = (CReactorUDPClient*)b->second;

        if (NULL != pClientInfo)
        {
            _ClientConnectInfo ClientConnectInfo = pClientInfo->GetClientConnectInfo();
            VecClientConnectInfo.push_back(ClientConnectInfo);
        }
    }
}

bool CClientReConnectManager::CloseByClient(int nServerID)
{
    //�������ΪԶ�����ӶϿ�����ֻɾ��ProConnectClient��ָ��
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    pClientInfo->SetConnectClient(NULL);
    pClientInfo->SetServerConnectState(SERVER_CONNECT_FAIL);
    return true;
}

bool CClientReConnectManager::GetConnectState(int nServerID)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    if (NULL == pClientInfo->GetConnectClient())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool CClientReConnectManager::ReConnect(int nServerID)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }

    CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;

    if (NULL == pClientInfo)
    {
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) pClientInfo is NULL.\n", nServerID));
        return false;
    }

    if (NULL == pClientInfo->GetConnectClient())
    {
        //������Ӳ����ڣ������½�������
        pClientInfo->Run(m_blReactorFinish);
        //�Զ�����0.1��
        ACE_Time_Value tvSleep(0, m_u4ConnectServerTimeout);
        ACE_OS::sleep(tvSleep);
        return true;
    }
    else
    {
        return true;
    }
}

ACE_INET_Addr CClientReConnectManager::GetServerAddr(int nServerID)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    ACE_INET_Addr remote_addr;
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is exist.\n", nServerID));
        return remote_addr;
    }
    else
    {
        CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;
        remote_addr = pClientInfo->GetServerAddr();
        return remote_addr;
    }
}

bool CClientReConnectManager::SetServerConnectState(int nServerID, EM_Server_Connect_State objState)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_ThreadWritrLock);
    mapReactorConnectInfo::iterator f = m_mapConnectInfo.find(nServerID);

    if (f == m_mapConnectInfo.end())
    {
        //�����������Ѿ����ڣ��򲻴����µ�����
        OUR_DEBUG((LM_ERROR, "[CClientReConnectManager::Close]nServerID =(%d) is exist.\n", nServerID));
        return false;
    }
    else
    {
        CReactorClientInfo* pClientInfo = (CReactorClientInfo*)f->second;
        pClientInfo->SetServerConnectState(objState);
        return true;
    }
}