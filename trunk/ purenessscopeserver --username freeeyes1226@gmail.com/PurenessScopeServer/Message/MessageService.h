#ifndef _MESSAGESERVICE_H
#define _MESSAGESERVICE_H

#include "define.h"
#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Malloc_T.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include "ace/Date_Time.h"

#include "Message.h"
#include "MessageManager.h"
#include "LogManager.h"
#include "ThreadInfo.h"
#include "BuffPacket.h"
#include "MainConfig.h"
#include "TimerManager.h"
#include "RandomNumber.h"

#ifdef WIN32
#include "ProConnectHandle.h"
#include "WindowsCPU.h"
#else
#include "ConnectHandler.h"
#include "LinuxCPU.h"
#endif

class CMessageService : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CMessageService(void);
	~CMessageService(void);

	virtual int open(void* args = 0);
	virtual int svc (void);
	int Close ();

	bool SaveThreadInfo();

	void Init(uint32 u4ThreadID, uint32 u4MaxQueue = MAX_MSG_THREADQUEUE, uint32 u4LowMask = MAX_MSG_MASK, uint32 u4HighMask = MAX_MSG_MASK);

	bool Start();

	bool PutMessage(CMessage* pMessage);

	_ThreadInfo* GetThreadInfo();

private:
	bool IsRun();
	bool ProcessMessage(CMessage* pMessage, uint32 u4ThreadID);
	bool SaveThreadInfoData(); 

private:
	uint32                         m_u4ThreadID;          //��ǰ�߳�ID
	uint32                         m_u4MaxQueue;          //�߳��������Ϣ�������
	bool                           m_blRun;               //�߳��Ƿ�������
	uint32                         m_u4HighMask;
	uint32                         m_u4LowMask;
	uint16                         m_u2ThreadTimeOut;  
	uint16                         m_u2ThreadTimeCheck;
	uint64                         m_u8TimeCost;          //Put��������Ϣ�����ݴ���ʱ��
	uint32                         m_u4Count;             //��Ϣ���н��ܸ���
	uint32                         m_u4WorkQueuePutTime;  //��ӳ�ʱʱ��

	_ThreadInfo                    m_ThreadInfo;          //��ǰ�߳���Ϣ
};

//add by freeeyes
//����̹߳����û����Դ������ɸ�ACE_Task��ÿ��Task��Ӧһ���̣߳�һ��Connectidֻ��Ӧһ���̡߳�
class CMessageServiceGroup : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CMessageServiceGroup(void);
	~CMessageServiceGroup(void);

	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

	bool Init(uint32 u4ThreadCount = MAX_MSG_THREADCOUNT, uint32 u4MaxQueue = MAX_MSG_THREADQUEUE, uint32 u4LowMask = MAX_MSG_MASK, uint32 u4HighMask = MAX_MSG_MASK);
	bool PutMessage(CMessage* pMessage);
	void Close();

	bool Start();
	CThreadInfo* GetThreadInfo();

	uint32 GetWorkThreadCount();                    //�õ���ǰ�����̵߳�����
	uint32 GetWorkThreadIDByIndex(uint32 u4Index);  //�õ�ָ�������̵߳��߳�ID    

private:
	bool StartTimer();
	bool KillTimer();

private:
	typedef vector<CMessageService*> vecMessageService;
	vecMessageService m_vecMessageService;

public:
	CThreadInfo                m_objAllThreadInfo;    //��ǰ�����߳���Ϣ
	uint32                     m_u4TimerID;           //��ʱ��ID
	uint16                     m_u2ThreadTimeCheck;   //�߳��Լ�ʱ��
	uint32                     m_u4MaxQueue;          //�߳��������Ϣ�������
	uint32                     m_u4HighMask;          //�̸߳�ˮλ
	uint32                     m_u4LowMask;           //�̵߳�ˮλ
	CRandomNumber              m_objRandomNumber;     //���������UDPʹ��
	//ACE_Recursive_Thread_Mutex m_ThreadWriteLock;     //��ǰ�߳���
};


typedef ACE_Singleton<CMessageServiceGroup,ACE_Null_Mutex> App_MessageServiceGroup; 
#endif
