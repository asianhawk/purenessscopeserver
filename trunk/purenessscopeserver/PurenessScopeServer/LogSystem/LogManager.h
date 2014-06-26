#ifndef _LOGMANAGER_H
#define _LOGMANAGER_H

#include "ace/Task.h"
#include <stdio.h>
#include <stdarg.h>
#include "define.h"
#include "MapTemplate.h"
#include "ILogObject.h"
#include "ILogManager.h"

//������־��ĳ�
//һ��ʹ�������飬����־���÷ּ�����
//Ҳ������־���������
//add by freeeyes

class CLogBlockPool
{
public:
	CLogBlockPool();
	~CLogBlockPool();

	void Init(uint32 u4BlockSize, uint32 u4PoolCount);
	void Close();

	_LogBlockInfo* GetLogBlockInfo();                       //�õ�һ���������־��
	void ReturnBlockInfo(_LogBlockInfo* pLogBlockInfo);     //�黹һ���������־��

	uint32 GetBlockSize();

private:
	_LogBlockInfo* m_pLogBlockInfo;       //��־��
	uint32         m_u4MaxBlockSize;      //��־�ص����������
	uint32         m_u4PoolCount;         //��־���е���־����� 
	uint32         m_u4CurrIndex;         //��־���е�ǰ���õ�����־��ID
};


class CLogManager : public ACE_Task<ACE_MT_SYNCH>, public ILogManager
{
public:
	CLogManager(void);
	~CLogManager(void);

	virtual int open (void *args = 0);
	virtual int svc(void);
	int Close();

	void Init(int nThreadCount = 1, int nQueueMax = MAX_MSG_THREADQUEUE);
	int Start();
	int Stop();
	bool IsRun();

	int PutLog(_LogBlockInfo* pLogBlockInfo);
	int RegisterLog(CServerLogger * pServerLogger);
	int UnRegisterLog();

	void SetReset(bool blReset);

	void ResetLogData(uint16 u2LogLevel);

	//��ά���ӿ�
	uint32 GetLogCount();
	uint32 GetCurrLevel();

	uint16 GetLogID(uint16 u2Index);
	char* GetLogInfoByServerName(uint16 u2LogID);
	char* GetLogInfoByLogName(uint16 u2LogID);
	int   GetLogInfoByLogDisplay(uint16 u2LogID);

	//����д��־�Ľӿ�
	int WriteLog(int nLogType, const char* fmt, ...);

	int WriteLogBinary(int nLogType, const char* pData, int nLen);

	int WriteToMail(int nLogType, uint32 u4MailID, char* pTitle, const char* fmt, ...);

private:
	int ProcessLog(_LogBlockInfo* pLogBlockInfo);

private:
	bool                              m_blRun;                    //��־ϵͳ�Ƿ�����
	bool                              m_blIsNeedReset;            //��־ģ��ȼ��������ñ�־
	int                               m_nThreadCount;             //��¼��־�̸߳�����ĿǰĬ����1
	int                               m_nQueueMax;                //��־�߳�����������и��� 
	CLogBlockPool                     m_objLogBlockPool;          //��־���
	ACE_Recursive_Thread_Mutex        m_Logger_Mutex;             //�߳���
	CServerLogger*                    m_pServerLogger;            //��־ģ��ָ��
};


typedef ACE_Singleton<CLogManager, ACE_Recursive_Thread_Mutex> AppLogManager; 


#endif
