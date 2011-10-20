
// ConnectHandle.h
// ����ͻ�������
// �ܶ�ʱ�䣬�����������������ˣ������������������������ˡ�û��ʲô��ںý���
// ������2009��Ĵ����һ�����ټ������������˼ά����������������ϵĶ����������������ļ̳�������
// �����ң��������Щ�ѣ������Ҳ����ں�����Ϊ��֪��������ĵ�·�Ӳ�ƽ̹���������¿�ʼ����Ϊ���ܸе�������ҵ�������
// ��Ӷ��������������ݰ����Ĺܿء�
// add by freeeyes
// 2008-12-22

#ifndef _CONNECTHANDLE_H
#define _CONNECTHANDLE_H

#include "define.h"

#include "ace/Reactor.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Reactor_Notification_Strategy.h"

#include "AceReactorManager.h"
#include "MessageService.h"
#include "IConnectManager.h"
#include "ThreadLock.h"
#include "MakePacket.h"
#include "MessageBlockManager.h"
#include "PacketParsePool.h"
#include "BuffPacketManager.h"
#include "ForbiddenIP.h"
#include "IPAccount.h"

#include <map>

using namespace std;

class CConnectHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
public:
	CConnectHandler(void);
	~CConnectHandler(void);

	//��д�̳з���
	virtual int open(void*);                                                //�û�����һ������
	virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask);

	void Init(uint16 u2HandlerID);

	bool SendMessage(IBuffPacket* pBuffPacket);
	
	bool Close(int nIOCount = 1);
	bool ServerClose();

	const char* GetError();
	void        SetConnectID(uint32 u4ConnectID);
	uint32      GetConnectID();
	uint8       GetConnectState();                                           //�õ�����״̬
	uint8       GetSendBuffState();                                          //�õ�����״̬    
	uint8       GetIsClosing();                                              //�����Ƿ�Ӧ�ùر�
	bool        CheckAlive();
	_ClientConnectInfo GetClientInfo();                                      //�õ��ͻ�����Ϣ 
	_ClientIPInfo      GetClientIPInfo();                                    //�õ��ͻ���IP��Ϣ

private:
	bool CheckMessage();                                                      //������յ�����
	bool PutSendPacket(ACE_Message_Block* pMbData);                           //��������

private:
	char                       m_szError[MAX_BUFF_500];
	ACE_INET_Addr              m_addrRemote;
	ACE_Time_Value             m_atvConnect;
	ACE_Time_Value             m_atvInput;
	ACE_Time_Value             m_atvOutput;
	ACE_Time_Value             m_atvSendAlive;

	uint8                      m_u1ConnectState;               //Ŀǰ���Ӵ���״̬
	uint8                      m_u1SendBuffState;              //Ŀǰ�������Ƿ��еȴ����͵�����
	uint8                      m_u1IsClosing;                  //�Ƿ�Ӧ�ùر� 0Ϊ��1Ϊ��
	uint16                     m_u2SendQueueMax;               //���Ͷ�����󳤶�
	uint16                     m_u2SendCount;                  //��ǰ���ݰ��ĸ���
	uint32                     m_u4HandlerID;                  //��Hander����ʱ��ID
	uint32                     m_u4ConnectID;                  //���ӵ�ID
	uint32                     m_u4AllRecvCount;               //��ǰ���ӽ������ݰ��ĸ���
	uint32                     m_u4AllSendCount;               //��ǰ���ӷ������ݰ��ĸ���
	uint32                     m_u4AllRecvSize;                //��ǰ���ӽ����ֽ�����
	uint32                     m_u4AllSendSize;                //��ǰ���ӷ����ֽ�����
	uint16                     m_u2MaxConnectTime;             //���ʱ�������ж�
	uint32                     m_u4MaxPacketSize;              //�������ݰ�����󳤶�
	
	ACE_Recursive_Thread_Mutex m_ThreadLock;

	uint32                     m_u4SendThresHold;              //���ͷ�ֵ(��Ϣ���ĸ���)
	uint32                     m_u4SendCheckTime;              //���ͼ��ʱ��ķ�ֵ
	int                        m_nIOCount;                     //��ǰIO�����ĸ���

	_TimerCheckID*             m_pTCClose;

	CBuffPacket                m_AlivePacket;                  //�����������
	CPacketParse*              m_pPacketParse;                 //���ݰ�������
	ACE_Message_Block*         m_pCurrMessage;                 //��ǰ��MB����
	uint32                     m_u4CurrSize;                   //��ǰMB�����ַ�����
	_TimeConnectInfo           m_TimeConnectInfo;              //���ӽ��������
};

//���������Ѿ�����������
class CConnectManager : public ACE_Task<ACE_MT_SYNCH>, public IConnectManager
{
public:
	CConnectManager(void);
	~CConnectManager(void);

	virtual int open(void* args = 0);
	virtual int svc (void);
	virtual int close (u_long);
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

	void CloseAll();
	bool AddConnect(CConnectHandler* pConnectHandler);
	bool SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //�첽����
	bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //ͬ������
	bool Close(uint32 u4ConnectID);                                    //�ͻ����ر�
	bool CloseConnect(uint32 u4ConnectID);                             //�������ر�
	void GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo);   //���ص�ǰ������ӵ���Ϣ

	_ClientIPInfo GetClientIPInfo(uint32 u4ConnectID);                 //�õ�ָ��������Ϣ

	bool StartTimer();
	bool KillTimer();

	int         GetCount();
	const char* GetError();

private:
	bool IsRun();
	
private:
	typedef map<uint32, CConnectHandler*> mapConnectManager;
	typedef vector<uint32> vecConnectManager;
	mapConnectManager           m_mapConnectManager;
	char                        m_szError[MAX_BUFF_500];
	uint32                      m_u4ConnectCurrID;
	uint32                      m_u4TimeCheckID;
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;       //����ѭ����غͶϿ�����ʱ���������
	_TimerCheckID*              m_pTCTimeSendCheck;
	ACE_Time_Value              m_tvCheckConnect;
	bool                        m_blRun;              //�߳��Ƿ�������

private:
	//����һ���������������������첽���Ͷ���
	struct _SendMessgae
	{
		uint32       m_u4ConnectID;
		IBuffPacket* m_pBuffPacket;

		~_SendMessgae()
		{
			if(m_pBuffPacket != NULL)
			{
				delete m_pBuffPacket;
			}
		}
	};
};

//����ConnectHandler�ڴ��
class CConnectHandlerPool
{
public:
	CConnectHandlerPool(void);
	~CConnectHandlerPool(void);

	void Init(int nObjcetCount);
	void Close();

	CConnectHandler* Create();
	bool Delete(CConnectHandler* pObject);

	int GetUsedCount();
	int GetFreeCount();

private:
	typedef map<CConnectHandler*, CConnectHandler*> mapHandle;
	mapHandle                   m_mapMessageUsed;                      //��ʹ�õ�
	mapHandle                   m_mapMessageFree;                      //û��ʹ�õ�
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;                     //���ƶ��߳���
	uint32                      m_u4CurrMaxCount;
};

typedef ACE_Singleton<CConnectManager, ACE_Recursive_Thread_Mutex> App_ConnectManager;
typedef ACE_Singleton<CConnectHandlerPool, ACE_Null_Mutex> App_ConnectHandlerPool;

#endif
