#ifndef _CLIENTCONNECTMANAGER_H
#define _CLIENTCONNECTMANAGER_H

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "TimerManager.h"
#include "IClientManager.h"
#include "ConnectClient.h"
#include "ReactorUDPClient.h"

#define RE_CONNECT_SERVER_TIMEOUT 100*1000

#include <map>

using namespace std;

typedef ACE_Connector<CConnectClient, ACE_SOCK_CONNECTOR> CConnectClientConnector;

class CReactorClientInfo
{
public:
    CReactorClientInfo();
    ~CReactorClientInfo();

    bool Init(int nServerID, const char* pIP, int nPort, uint8 u1IPType, CConnectClientConnector* pReactorConnect, IClientMessage* pClientMessage, ACE_Reactor* pReactor);  //��ʼ�����ӵ�ַ�Ͷ˿�
    bool Run(bool blIsReady, EM_Server_Connect_State emState = SERVER_CONNECT_RECONNECT);  //��ʼ����
    bool SendData(ACE_Message_Block* pmblk);                                               //��������
    bool ConnectError(int nError);                                                         //���Ӵ��󣬱���
    int  GetServerID();                                                                    //�õ�������ID
    bool Close();                                                                          //�رշ���������
    void SetConnectClient(CConnectClient* pConnectClient);                                 //��������״̬
    CConnectClient* GetConnectClient();                                                    //�õ�ProConnectClientָ��
    IClientMessage* GetClientMessage();                                                    //��õ�ǰ����Ϣ����ָ��
    ACE_INET_Addr GetServerAddr();                                                         //��÷������ĵ�ַ
    EM_Server_Connect_State GetServerConnectState();                                       //�õ���ǰ����״̬
    void SetServerConnectState(EM_Server_Connect_State objState);                          //���õ�ǰ����״̬

private:
    ACE_INET_Addr              m_AddrServer;             //Զ�̷������ĵ�ַ
    CConnectClient*            m_pConnectClient;         //��ǰ���Ӷ���
    CConnectClientConnector*   m_pReactorConnect;        //Connector���Ӷ���
    IClientMessage*            m_pClientMessage;         //�ص������࣬�ص����ش���ͷ������ݷ���
    int                        m_nServerID;              //Զ�̷�������ID
    ACE_Reactor*               m_pReactor;               //��¼ʹ�õķ�Ӧ��
    EM_Server_Connect_State    m_emConnectState;         //����״̬
};

class CClientReConnectManager : public ACE_Event_Handler, public IClientManager
{
public:
    CClientReConnectManager(void);
    ~CClientReConnectManager(void);

public:
    bool Init(ACE_Reactor* pReactor);
    bool Connect(int nServerID, const char* pIP, int nPort, uint8 u1IPType, IClientMessage* pClientMessage);                                   //���ӷ�����(TCP)
    bool ConnectUDP(int nServerID, const char* pIP, int nPort, uint8 u1IPType, IClientUDPMessage* pClientUDPMessage);                          //����һ��ָ��UDP�����ӣ�UDP��
    bool ReConnect(int nServerID);                                                                                             //��������һ��ָ���ķ�����(TCP)
    bool CloseByClient(int nServerID);                                                                                         //Զ�̱����ر�(TCP)
    bool Close(int nServerID);                                                                                                 //�ر�����
    bool CloseUDP(int nServerID);                                                                                              //�ر����ӣ�UDP��
    bool ConnectErrorClose(int nServerID);                                                                                     //���ڲ����������ʧ�ܣ���ProConnectClient����
    bool SendData(int nServerID, const char* pData, int nSize, bool blIsDelete = true);                                        //��������
    bool SendDataUDP(int nServerID, const char* pIP, int nPort, const char* pMessage, uint32 u4Len, bool blIsDelete = true);   //�������ݣ�UDP��
    bool SetHandler(int nServerID, CConnectClient* pConnectClient);                                                            //��ָ����CProConnectClient*�󶨸�nServerID
    IClientMessage* GetClientMessage(int nServerID);                                                                           //���ClientMessage����
    bool StartConnectTask(int nIntervalTime = CONNECT_LIMIT_RETRY);                                                            //�����Զ������Ķ�ʱ��
    void CancelConnectTask();                                                                                                  //�ر�������ʱ��
    void Close();                                                                                                              //�ر���������
    bool GetConnectState(int nServerID);                                                                                       //�õ�ָ�����ӵĵ�ǰ״̬
    ACE_INET_Addr GetServerAddr(int nServerID);                                                                                //�õ�ָ����������Զ�̵�ַ������Ϣ
    bool SetServerConnectState(int nServerID, EM_Server_Connect_State objState);                                               //����ָ�����ӵ�����״̬

    void GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo);      //���ص�ǰ������ӵ���Ϣ��TCP��
    void GetUDPConnectInfo(vecClientConnectInfo& VecClientConnectInfo);   //���ص�ǰ������ӵ���Ϣ��UDP��

    virtual int handle_timeout(const ACE_Time_Value& current_time, const void* act = 0);               //��ʱ��ִ��

private:
    typedef map<int, CReactorClientInfo*> mapReactorConnectInfo;
    typedef map<int, CReactorUDPClient*>  mapReactorUDPConnectInfo;

public:
    mapReactorConnectInfo       m_mapConnectInfo;              //TCP���Ӷ��������
    mapReactorUDPConnectInfo    m_mapReactorUDPConnectInfo;    //UDP���Ӷ��������
    CConnectClientConnector     m_ReactorConnect;              //Reactor���ӿͻ��˶���
    ACE_Recursive_Thread_Mutex  m_ThreadWritrLock;             //�߳���
    ActiveTimer                 m_ActiveTimer;                 //ʱ�������
    int                         m_nTaskID;                     //��ʱ��⹤��
    ACE_Reactor*                m_pReactor;                    //��ǰ�ķ�Ӧ��
    bool                        m_blReactorFinish;             //Reactor�Ƿ��Ѿ�ע��
    uint32                      m_u4ConnectServerTimeout;      //���Ӽ��ʱ��
};

typedef ACE_Singleton<CClientReConnectManager, ACE_Recursive_Thread_Mutex> App_ClientReConnectManager;
#endif
