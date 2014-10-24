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
#include "WorkThreadAI.h"
#include "CommandAccount.h"

#ifdef WIN32
#include "ProConnectHandle.h"
#include "WindowsCPU.h"
#else
#include "ConnectHandler.h"
#include "LinuxCPU.h"
#endif

//AI������Ϣ��
typedef vector<_WorkThreadAIInfo> vecWorkThreadAIInfo;

enum MESSAGE_SERVICE_THREAD_STATE
{
	THREAD_RUN = 0,               //�߳���������
	THREAD_MODULE_UNLOAD,         //ģ�����أ���Ҫ�߳�֧�ִ˹��� 
	THREAD_STOP,                  //�߳�ֹͣ 
};

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

	void GetAIInfo(_WorkThreadAIInfo& objAIInfo);           //�õ����й����̵߳�AI����
	void GetAITO(vecCommandTimeout& objTimeout);            //�õ����е�AI��ʱ���ݰ���Ϣ
	void GetAITF(vecCommandTimeout& objTimeout);            //�õ����е�AI������ݰ���Ϣ
	void SetAI(uint8 u1AI, uint32 u4DisposeTime, uint32 u4WTCheckTime, uint32 u4WTStopTime);  //����AI

	_CommandData* GetCommandData(uint16 u2CommandID);                      //�õ����������Ϣ
	_CommandFlowAccount GetCommandFlowAccount();                           //�õ����������Ϣ
	void GetCommandTimeOut(vecCommandTimeOut& CommandTimeOutList);         //�õ����г�ʱ����
	void GetCommandAlertData(vecCommandAlertData& CommandAlertDataList);   //�õ����г����澯��ֵ������ 
	void ClearCommandTimeOut();                                            //������еĳ�ʱ�澯
	void SaveCommandDataLog();                                             //�洢ͳ����־
	void SetThreadState(MESSAGE_SERVICE_THREAD_STATE emState);             //�����߳�״̬
	MESSAGE_SERVICE_THREAD_STATE GetThreadState();                         //�õ���ǰ�߳�״̬
	uint32 GetStepState();                                                 //�õ���ǰ���������Ϣ

	uint32 GetThreadID();

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

	MESSAGE_SERVICE_THREAD_STATE   m_emThreadState;       //��ǰ�����߳�״̬

	_ThreadInfo                    m_ThreadInfo;          //��ǰ�߳���Ϣ
	CWorkThreadAI                  m_WorkThreadAI;        //�߳����Ҽ�ص�AI�߼�  
	CCommandAccount                m_CommandAccount;      //��ǰ�߳�����ͳ������  
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

	uint32 GetWorkThreadCount();                                                              //�õ���ǰ�����̵߳�����
	uint32 GetWorkThreadIDByIndex(uint32 u4Index);                                            //�õ�ָ�������̵߳��߳�ID
	void GetWorkThreadAIInfo(vecWorkThreadAIInfo& objvecWorkThreadAIInfo);                    //�õ��̹߳���AI������Ϣ
	void GetAITO(vecCommandTimeout& objTimeout);                                              //�õ����е�AI��ʱ���ݰ���Ϣ
	void GetAITF(vecCommandTimeout& objTimeout);                                              //�õ����е�AI������ݰ���Ϣ
	void SetAI(uint8 u1AI, uint32 u4DisposeTime, uint32 u4WTCheckTime, uint32 u4WTStopTime);  //����AI

	void GetCommandData(uint16 u2CommandID, _CommandData& objCommandData);                    //���ָ������ͳ����Ϣ
	void GetFlowInfo(_CommandFlowAccount& objCommandFlowAccount);                             //���ָ�����������Ϣ

	void GetCommandTimeOut(vecCommandTimeOut& CommandTimeOutList);                            //�õ����г�ʱ����
	void GetCommandAlertData(vecCommandAlertData& CommandAlertDataList);                      //�õ����г����澯��ֵ������ 
	void ClearCommandTimeOut();                                                               //�������еĳ�ʱ�澯
	void SaveCommandDataLog();                                                                //�洢ͳ����־

	bool UnloadModule(const char* pModuleName, uint8 u1State);                                //ж�ػ�������ָ����ģ����

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
