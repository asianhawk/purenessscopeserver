#ifndef _PROCONNECTMANAGER_H
#define _PROCONNECTMANAGER_H

//��������Զ�̷������Ĺ�����
//��������һ���ṹ��������ConectHandlerָ��
//add by freeeyes 2010-12-27

#include "ace/INET_Addr.h"
#include "ace/Guard_T.h"

#include "TimerManager.h"
#include "ProAsynchConnect.h"
#include "IClientManager.h"
#include "ProactorUDPClient.h"

#include <map>

using namespace std;

class CProactorClientInfo
{
public:
	CProactorClientInfo();
	~CProactorClientInfo();

	bool Init(const char* pIP, int nPort, CProAsynchConnect* pProAsynchConnect, IClientMessage* pClientMessage);  //��ʼ�����ӵ�ַ�Ͷ˿�
	bool Run();                                                     //��ʼ����
	bool SendData(ACE_Message_Block* pmblk);                        //��������
	bool ConnectError(int nError);                                  //���Ӵ��󣬱���
	bool SetServerID(int nServerID);                                //���÷�����ID
	int  GetServerID();                                             //�õ�������ID
	bool Close();                                                   //�رշ���������
	void SetProConnectClient(CProConnectClient* pProConnectClient); //����ProConnectClientָ��
	CProConnectClient* GetProConnectClient();                       //�õ�ProConnectClientָ��
	IClientMessage* GetClientMessage();                             //��õ�ǰ����Ϣ����ָ��

private:
	ACE_INET_Addr      m_AddrServer;             //Զ�̷������ĵ�ַ
	CProConnectClient* m_pProConnectClient;      //��ǰ���Ӷ���
	CProAsynchConnect* m_pProAsynchConnect;      //�첽���Ӷ���
	IClientMessage*    m_pClientMessage;         //�ص������࣬�ص����ش���ͷ������ݷ���
};

//�����������ӵ������������Ĺ�����
class CClientProConnectManager : public ACE_Task<ACE_MT_SYNCH>, public IClientManager
{
public:
	CClientProConnectManager(void);
	~CClientProConnectManager(void);

	bool Init(ACE_Proactor* pProactor);                                                                //��ʼ��������
	bool Connect(int nServerID, const char* pIP, int nPort, IClientMessage* pClientMessage);           //����ָ���ķ�������TCP��
	bool ConnectUDP(int nServerID, const char* pIP, int nPort, IClientUDPMessage* pClientUDPMessage);  //����һ��ָ��UDP�����ӣ�UDP��
	bool Close(int nServerID);                                                                         //�ر����ӣ�TCP��
	bool CloseUDP(int nServerID);                                                                      //�ر����ӣ�UDP��
	bool ConnectErrorClose(int nServerID);                                                             //���ڲ����������ʧ�ܣ���ProConnectClient����
	bool SendData(int nServerID, const char* pData, int nSize);                                        //�������ݣ�TCP��
	bool SendDataUDP(int nServerID,const char* pIP, int nPort, const char* pMessage, uint32 u4Len);    //�������ݣ�UDP��
	bool SetHandler(int nServerID, CProConnectClient* pProConnectClient);                              //��ָ����CProConnectClient*�󶨸�nServerID
	IClientMessage* GetClientMessage(int nServerID);                                                   //���ClientMessage����
	bool StartConnectTask(int nIntervalTime = CONNECT_LIMIT_RETRY);                                    //�����Զ������Ķ�ʱ��
	void CancelConnectTask();                                                                          //�ر�������ʱ��
	void Close();

	void GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo);      //���ص�ǰ������ӵ���Ϣ��TCP��
	void GetUDPConnectInfo(vecClientConnectInfo& VecClientConnectInfo);   //���ص�ǰ������ӵ���Ϣ��UDP��

	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);                       //��ʱ���

private:
	typedef map<int, CProactorClientInfo*> mapProactorClientInfo;              //���������Ѿ����ڵĿͻ���������Ϣ��TCP��
	typedef map<int, CProactorUDPClient*>  mapProactorUDPClientInfo;           //���������Ѿ����ڵĿͻ���������Ϣ��UDP��

private:
	 CProAsynchConnect           m_ProAsynchConnect;
	 mapProactorClientInfo       m_mapClientInfo;               //TCP�ͻ�������
	 mapProactorUDPClientInfo    m_mapProactorUDPClientInfo;    //UDP�ͻ�������
	 ACE_Recursive_Thread_Mutex  m_ThreadWritrLock;             //�߳���
	 ActiveTimer                 m_ActiveTimer;                 //ʱ�������
	 int                         m_nTaskID;                     //��ʱ��⹤��
};

typedef ACE_Singleton<CClientProConnectManager, ACE_Recursive_Thread_Mutex> App_ClientProConnectManager;

#endif 
