
// ConnectHandle.h
// 处理客户端链接
// 很多时间，做出来就是做出来了，做不出来，就是做不出来了。没有什么借口好讲。
// 今天是2009年的大年初一，我再继续着我在这的思维，我想把在蓝翼身上的东西，在这里完整的继承下来。
// 对于我，这可能有些难，但是我并不在乎。因为我知道，梦想的道路从不平坦，不怕重新开始，因为我能感到蓝翼对我的期望。
// 添加对链接流量，数据包数的管控。
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

	//重写继承方法
	virtual int open(void*);                                                //用户建立一个链接
	virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask);

	void Init(uint16 u2HandlerID);

	bool SendMessage(IBuffPacket* pBuffPacket);
	
	bool Close(int nIOCount = 1);
	bool ServerClose();

	const char* GetError();
	void        SetConnectID(uint32 u4ConnectID);
	uint32      GetConnectID();
	uint8       GetConnectState();                                           //得到链接状态
	uint8       GetSendBuffState();                                          //得到发送状态    
	uint8       GetIsClosing();                                              //链接是否应该关闭
	bool        CheckAlive();
	_ClientConnectInfo GetClientInfo();                                      //得到客户端信息 
	_ClientIPInfo      GetClientIPInfo();                                    //得到客户端IP信息

private:
	bool CheckMessage();                                                      //处理接收的数据
	bool PutSendPacket(ACE_Message_Block* pMbData);                           //发送数据

private:
	char                       m_szError[MAX_BUFF_500];
	ACE_INET_Addr              m_addrRemote;
	ACE_Time_Value             m_atvConnect;
	ACE_Time_Value             m_atvInput;
	ACE_Time_Value             m_atvOutput;
	ACE_Time_Value             m_atvSendAlive;

	uint8                      m_u1ConnectState;               //目前链接处理状态
	uint8                      m_u1SendBuffState;              //目前缓冲器是否有等待发送的数据
	uint8                      m_u1IsClosing;                  //是否应该关闭 0为否，1为是
	uint16                     m_u2SendQueueMax;               //发送队列最大长度
	uint16                     m_u2SendCount;                  //当前数据包的个数
	uint32                     m_u4HandlerID;                  //此Hander生成时的ID
	uint32                     m_u4ConnectID;                  //链接的ID
	uint32                     m_u4AllRecvCount;               //当前链接接收数据包的个数
	uint32                     m_u4AllSendCount;               //当前链接发送数据包的个数
	uint32                     m_u4AllRecvSize;                //当前链接接收字节总数
	uint32                     m_u4AllSendSize;                //当前链接发送字节总数
	uint16                     m_u2MaxConnectTime;             //最大时间链接判定
	uint32                     m_u4MaxPacketSize;              //单个数据包的最大长度
	
	ACE_Recursive_Thread_Mutex m_ThreadLock;

	uint32                     m_u4SendThresHold;              //发送阀值(消息包的个数)
	uint32                     m_u4SendCheckTime;              //发送检测时间的阀值
	int                        m_nIOCount;                     //当前IO操作的个数

	_TimerCheckID*             m_pTCClose;

	CBuffPacket                m_AlivePacket;                  //服务器生存包
	CPacketParse*              m_pPacketParse;                 //数据包解析类
	ACE_Message_Block*         m_pCurrMessage;                 //当前的MB对象
	uint32                     m_u4CurrSize;                   //当前MB缓冲字符长度
	_TimeConnectInfo           m_TimeConnectInfo;              //链接健康检测器
};

//管理所有已经建立的链接
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
	bool SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //异步发送
	bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //同步发送
	bool Close(uint32 u4ConnectID);                                    //客户单关闭
	bool CloseConnect(uint32 u4ConnectID);                             //服务器关闭
	void GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo);   //返回当前存活链接的信息

	_ClientIPInfo GetClientIPInfo(uint32 u4ConnectID);                 //得到指定链接信息

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
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;       //用于循环监控和断开链接时候的数据锁
	_TimerCheckID*              m_pTCTimeSendCheck;
	ACE_Time_Value              m_tvCheckConnect;
	bool                        m_blRun;              //线程是否在运行

private:
	//定义一个发送数据容器，用于异步发送队列
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

//链接ConnectHandler内存池
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
	mapHandle                   m_mapMessageUsed;                      //已使用的
	mapHandle                   m_mapMessageFree;                      //没有使用的
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;                     //控制多线程锁
	uint32                      m_u4CurrMaxCount;
};

typedef ACE_Singleton<CConnectManager, ACE_Recursive_Thread_Mutex> App_ConnectManager;
typedef ACE_Singleton<CConnectHandlerPool, ACE_Null_Mutex> App_ConnectHandlerPool;

#endif
