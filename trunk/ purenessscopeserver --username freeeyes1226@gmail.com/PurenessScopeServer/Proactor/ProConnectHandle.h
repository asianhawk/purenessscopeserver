// ConnectHandle.h
// 处理客户端链接
// 不得不说，今天是一个艰苦的考验，经过几个月完善优化的reactor架构，竟然不能适应多于1024个的链接请求
// 对于ACE架构的理解，使我重新的认识，当然，好在的是，替换架构的部分，只有发送和接受部分。其他部分都可以保留。
// 费尽心机做出来的东西，竟然被自己否定，是一个很痛苦的过程，当然，如果想做的更好，这是必须的代价。
// 坚信自己能够做的更好，发现问题，解决问题即可。
// 加油吧，你能做到的。
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

	//重写继承方法
	virtual void open(ACE_HANDLE h, ACE_Message_Block&);                                 //用户建立一个链接
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
	uint8              GetConnectState();                                     //得到链接状态
	uint8              GetSendBuffState();                                    //得到发送状态    
	bool               GetIsClosing();                                        //链接是否应该关闭
	_ClientConnectInfo GetClientInfo();                                       //得到客户端信息
	_ClientIPInfo      GetClientIPInfo();                                     //得到客户端IP信息

private:
	bool RecvClinetPacket(uint32 u4PackeLen);                                 //接受数据包
	bool CheckMessage();                                                      //处理接收的数据
	bool PutSendPacket(ACE_Message_Block* pMbData);                           //将发送数据放入队列

private:
	char             m_szError[MAX_BUFF_500];
	ACE_INET_Addr    m_addrRemote;
	ACE_Time_Value   m_atvConnect;
	ACE_Time_Value   m_atvInput;
	ACE_Time_Value   m_atvOutput;
	ACE_Time_Value   m_atvSendAlive;

	CBuffPacket        m_AlivePacket;                  //服务器生存包
	uint8              m_u1ConnectState;               //目前链接处理状态
	uint8              m_u1SendBuffState;              //目前缓冲器是否有等待发送的数据
	uint16             m_u2SendQueueMax;               //发送队列最大长度
	uint16             m_u2MaxConnectTime;             //最大链接时间判定
	uint32             m_u4MaxPacketSize;              //单个数据包的最大长度
	uint32             m_u4HandlerID;                  //此Hander生成时的ID
	uint32             m_u4ConnectID;                  //当前Connect的流水号
	uint32             m_u4AllRecvCount;               //当前链接接收数据包的个数
	uint32             m_u4AllSendCount;               //当前链接发送数据包的个数
	uint32             m_u4AllRecvSize;                //当前链接接收字节总数
	uint32             m_u4AllSendSize;                //当前链接发送字节总数 
	CPacketParse*      m_pPacketParse;                 //数据包解析类

	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;

	uint32           m_u4SendThresHold;              //发送阀值(消息包的个数)
	uint32           m_u4SendCheckTime;              //发送检测时间的阀值
	bool             m_blCanWrite;                   //上一个数据包是否发送结束
	bool             m_blTimeClose;                  //是否正在关闭

	int              m_u4RecvPacketCount;            //接受包的个数
	int              m_nIOCount;                     //当前IO操作的个数
	_TimeConnectInfo m_TimeConnectInfo;              //链接健康检测器

	Fast_Asynch_Read_Stream  m_Reader;
	Fast_Asynch_Write_Stream m_Writer;
};

//管理所有已经建立的链接
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
	bool SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //同步发送
	bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket);    //异步发送
	bool Close(uint32 u4ConnectID);                                    //客户端关闭
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
	typedef map<uint32, CProConnectHandle*> mapConnectManager;
	typedef vector<uint32> vecConnectManager;
	mapConnectManager           m_mapConnectManager;
	char                        m_szError[MAX_BUFF_500];
	uint32                      m_u4ConnectCurrID;
	uint32                      m_u4TimeCheckID;
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;       //用于循环监控和断开链接时候的数据锁
	ACE_Time_Value              m_tvCheckConnect;
	bool                        m_blRun;              //线程是否在运行

private:
	//定义一个发送数据容器，用于异步发送队列
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

//链接ConnectHandler内存池
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
	mapHandle                   m_mapMessageUsed;                      //已使用的
	mapHandle                   m_mapMessageFree;                      //没有使用的
	ACE_Recursive_Thread_Mutex  m_ThreadWriteLock;                     //控制多线程锁
	uint32                      m_u4CurrMaxCount;
};

typedef ACE_Singleton<CProConnectManager, ACE_Null_Mutex> App_ProConnectManager; 
typedef ACE_Singleton<CProConnectHandlerPool, ACE_Null_Mutex> App_ProConnectHandlerPool;

#endif
