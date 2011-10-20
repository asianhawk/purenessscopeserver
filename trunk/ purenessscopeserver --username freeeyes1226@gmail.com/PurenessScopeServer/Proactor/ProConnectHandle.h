// ConnectHandle.h
// ����ͻ�������
// ���ò�˵��������һ�����Ŀ��飬���������������Ż���reactor�ܹ�����Ȼ������Ӧ����1024������������
// ����ACE�ܹ�����⣬ʹ�����µ���ʶ����Ȼ�����ڵ��ǣ��滻�ܹ��Ĳ��֣�ֻ�з��ͺͽ��ܲ��֡��������ֶ����Ա�����
// �Ѿ��Ļ��������Ķ�������Ȼ���Լ��񶨣���һ����ʹ��Ĺ��̣���Ȼ����������ĸ��ã����Ǳ���Ĵ��ۡ�
// �����Լ��ܹ����ĸ��ã��������⣬������⼴�ɡ�
// ���Ͱɣ����������ġ�
// add by freeeyes
// 2009-08-23

#ifndef _PROCONNECTHANDLE_H
#define _PROCONNECTHANDLE_H

#include "ace/Svc_Handler.h"
#include "ace/Synch.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Asynch_IO.h"
#include "ace/Asynch_Acceptor.h"
#include "ace/Proactor.h"

#include "AceProactorManager.h"
#include "IConnectManager.h"
#include "TimerManager.h"
#include "MakePacket.h"
#include "PacketParsePool.h"
#include "BuffPacketManager.h"
#include "Fast_Asynch_Read_Stream.h"
#include "ForbiddenIP.h"
#include "IPAccount.h"

#include <map>
#include <vector>

class CProConnectHandle : public ACE_Service_Handler
{
public:
	CProConnectHandle(void);
	~CProConnectHandle(void);

	//��д�̳з���
	virtual void open(ACE_HANDLE h, ACE_Message_Block&);                                 //�û�����һ������
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result &result);
	virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result &result);
	virtual void addresses(const ACE_INET_Addr &remote_address, const ACE_INET_Addr &local_address);

	void Init(uint16 u2HandlerID);

	bool CheckAlive();
	bool SendMessage(IBuffPacket* pBuffPacket);
	bool Close(int nIOCount = 1);
	bool ServerClose();

	const char*        GetError();
	void               SetConnectID(uint32 u4ConnectID);
	uint32             GetConnectID();
	uint8              GetConnectState();                                     //�õ�����״̬
	uint8              GetSendBuffState();                                    //�õ�����״̬    
	bool               GetIsClosing();                                        //�����Ƿ�Ӧ�ùر�
	_ClientConnectInfo GetClientInfo();                                       //�õ��ͻ�����Ϣ
	_ClientIPInfo      GetClientIPInfo();                                     //�õ��ͻ���IP��Ϣ

private:
	bool RecvClinetPacket(uint32 u4PackeLen);                                 //�������ݰ�
	bool CheckMessage();                                                      //������յ�����
	bool PutSendPacket(ACE_Message_Block* pMbData);                           //���������ݷ������

private:
	char             m_szError[MAX_BUFF_500];
	ACE_INET_Addr    m_addrRemote;
	ACE_Time_Value   m_atvConnect;
	ACE_Time_Value   m_atvInput;
	ACE_Time_Value   m_atvOutput;
	ACE_Time_Value   m_atvSendAlive;

	CBuffPacket        m_AlivePacket;                  //�����������
	uint8              m_u1ConnectState;               //Ŀǰ���Ӵ���״̬
	uint8              m_u1SendBuffState;              //Ŀǰ�������Ƿ��еȴ����͵�����
	uint16             m_u2SendQueueMax;               //���Ͷ�����󳤶�
	uint16             m_u2MaxConnectTime;             //�������ʱ���ж�
	uint32             m_u4MaxPacketSize;              //�������ݰ�����󳤶�
	uint32             m_u4HandlerID;                  //��Hander����ʱ��ID
	uint32             m_u4ConnectID;                  //��ǰConnect����ˮ��
	uint32             m_u4AllRecvCount;               //��ǰ���ӽ������ݰ��ĸ���
	uint32             m_u4AllSendCount;               //��ǰ���ӷ������ݰ��ĸ���
	uint32             m_u4AllRecvSize;                //��ǰ���ӽ����ֽ�����
	uint32             m_u4AllSendSize;                //��ǰ���ӷ����ֽ����� 
	CPacketParse*      m_pPacketParse;                 //���ݰ�������

	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;

	uint32           m_u4SendThresHold;              //���ͷ�ֵ(��Ϣ���ĸ���)
	uint32           m_u4SendCheckTime;              //���ͼ��ʱ��ķ�ֵ
	bool             m_blCanWrite;                   //��һ�����ݰ��Ƿ��ͽ���
	bool             m_blTimeClose;                  //�Ƿ����ڹر�

	int              m_u4RecvPacketCount;            //���ܰ��ĸ���
	int              m_nIOCount;                     //��ǰIO�����ĸ���
	_TimeConnectInfo m_TimeConnectInfo;              //���ӽ��������

	Fast_Asynch_Read_Stream  m_Reader;
	Fast_Asynch_Write_Stream m_Writer;
};

//���������Ѿ�����������
class CProConnectManager : public ACE_Task<ACE_MT_SYNCH>, public IConnectManager
{
public:
	CProConnectManager(void);
	~CProConnectManager(void);

	virtual int open(void* args = 0);
	virtual int svc (void);
	virtual int close (u_long);
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

	void CloseAll();
	bool AddConnect(CProConnectHandle* pConnectHandler);
	bool SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //ͬ������
	bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //�첽����
	bool Close(uint32 u4ConnectID);                                    //�ͻ��˹ر�
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
	typedef map<uint32, CProConnectHandle*> mapConnectManager;
	typedef vector<uint32> vecConnectManager;
	mapConnectManager           m_mapConnectManager;
	char                        m_szError[MAX_BUFF_500];
	uint32                      m_u4ConnectCurrID;
	uint32                      m_u4TimeCheckID;
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;       //����ѭ����غͶϿ�����ʱ���������
	ACE_Time_Value              m_tvCheckConnect;
	bool                        m_blRun;              //�߳��Ƿ�������

private:
	//����һ���������������������첽���Ͷ���
	struct _SendMessgae
	{
		uint32       m_u4ConnectID;
		IBuffPacket* m_pBuffPacket;

		_SendMessgae()
		{
			m_pBuffPacket = NULL; 
		}

		~_SendMessgae()
		{
		}
	};
};

//����ConnectHandler�ڴ��
class CProConnectHandlerPool
{
public:
	CProConnectHandlerPool(void);
	~CProConnectHandlerPool(void);

	void Init(int nObjcetCount);
	void Close();

	CProConnectHandle* Create();
	bool Delete(CProConnectHandle* pObject);

	int GetUsedCount();
	int GetFreeCount();

private:
	typedef map<CProConnectHandle*, CProConnectHandle*> mapHandle;
	mapHandle                   m_mapMessageUsed;                      //��ʹ�õ�
	mapHandle                   m_mapMessageFree;                      //û��ʹ�õ�
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;                     //���ƶ��߳���
	uint32                      m_u4CurrMaxCount;
};

typedef ACE_Singleton<CProConnectManager, ACE_Null_Mutex> App_ProConnectManager; 
typedef ACE_Singleton<CProConnectHandlerPool, ACE_Null_Mutex> App_ProConnectHandlerPool;

#endif
