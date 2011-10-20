#ifndef MAKEPACKET_H
#define MAKEPACKET_H

//专门处理数据解包操作
//把数据包的拼接和解包放在一起
//add by freeeyes
//2010-06-11
#pragma once

#include "define.h"
#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Malloc_T.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include "ace/Date_Time.h"

#include "PacketParsePool.h"
#include "MessageService.h"

#include <map>

using namespace std;

struct _MakePacket
{
	uint8         m_PacketType;     //数据包类型 
	uint32        m_u4ConnectID;    //链接ID
	uint8         m_u1Option;       //操作类型
	CPacketParse* m_pPacketParse;   //数据包数据指针
	ACE_INET_Addr m_AddrRemote;     //数据包的来源IP信息

	_MakePacket()
	{
		m_PacketType        = PACKET_TCP;  //0为TCP,1是UDP 默认是TCP
		m_u4ConnectID       = 0;
		m_u1Option          = 0;
		m_pPacketParse      = 0;
	}
};

//_MakePacket对象池，循环使用，不new数据。提升生成速度，应用于CMakePacket
class CMakePacketPool
{
public:
	CMakePacketPool();
	~CMakePacketPool();

	void Init(uint32 u4PacketCount);
	void Close();

	_MakePacket* Create();
	bool Delete(_MakePacket* pMakePacket);

	int GetUsedCount();
	int GetFreeCount();

private:
	typedef map<_MakePacket*, _MakePacket*> mapPacket;
	mapPacket                  m_mapPacketUsed;                       //已使用的
	mapPacket                  m_mapPacketFree;                       //没有使用的
	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;                     //控制多线程锁
};

class CMakePacket : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CMakePacket(void);
	~CMakePacket(void);

	virtual int open(void* args = 0);
	virtual int svc (void);
	int Close();
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

	void Init(uint32 u4ThreadCount = MAX_MSG_THREADCOUNT, uint32 u4MaxQueue = MAX_MSG_THREADQUEUE, uint32 u4LowMask = MAX_MSG_MASK, uint32 u4HighMask = MAX_MSG_MASK);
	bool Start();
	
	bool PutMessageBlock(uint32 u4ConnectID, uint8 u1Option, CPacketParse* pPacketParse);                        //处理TCP数据包
	bool PutUDPMessageBlock(const ACE_INET_Addr& AddrRemote, uint8 u1Option, CPacketParse* pPacketParse);        //处理UDP数据包 
	bool ProcessMessageBlock(_MakePacket* pMakePacket, uint32 u4ThreadID);
	CThreadInfo* GetThreadInfo();

private:
	bool StartTimer();
	bool KillTimer();
	bool ResumeThread(int nThreadCount);
	bool IsRun();
	CMessage* SetMessage(CPacketParse* pPacketParse, uint32 u4ConnectID);                //一般数据包消息(TCP)
	CMessage* SetMessage(CPacketParse* pPacketParse, const ACE_INET_Addr& AddrRemote);   //一般数据包消息(UDP)
	CMessage* SetMessageConnect(CPacketParse* pPacketParse, uint32 u4ConnectID);         //用户链接数据包消息
	CMessage* SetMessageCDisConnect(CPacketParse* pPacketParse, uint32 u4ConnectID);     //用户断开链接数据包消息
	CMessage* SetMessageSDisConnect(CPacketParse* pPacketParse, uint32 u4ConnectID);     //服务器断开链接数据包消息

private:
	ACE_Recursive_Thread_Mutex     m_RunMutex;           //线程锁级别
	ACE_RW_Thread_Mutex            m_rwMutex;
	uint32                         m_u4ThreadCount;      //处理的线程总数
	uint32                         m_u4ThreadNo;         //当前线程ID
	uint32                         m_u4MaxQueue;         //线程中最大消息对象个数
	uint32                         m_u4TimerID;          //定时器对象
	bool                           m_blRun;              //线程是否在运行
	uint32                         m_u4HighMask;
	uint32                         m_u4LowMask;
	uint16                         m_u2ThreadTimeOut;  
	uint16                         m_u2ThreadTimeCheck;
	uint16                         m_u2PacketTimeOut;    //处理数据包超时判定阀值

	CThreadInfo                    m_ThreadInfo;
	CMakePacketPool                m_MakePacketPool;
};
typedef ACE_Singleton<CMakePacket, ACE_Null_Mutex> App_MakePacket; 

#endif
