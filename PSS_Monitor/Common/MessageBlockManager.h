#ifndef MESSAGEBLOCKMANAGER_H
#define MESSAGEBLOCKMANAGER_H

//���︺��������д�����mb�����ݣ���ʹ���ڴ�ء�
//�������freebird���뷨����д�ⲿ�ִ��룬��Ȼ������æ�������ⲿ�ֱ�����ɣ�
//add by freeeyes
//2010-06-12

#include "ace/Thread_Mutex.h"
#include "ace/Malloc.h"
#include "ace/Malloc_T.h"
#include "ace/Local_Memory_Pool.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include "ace/Task.h"
#include "ace/Synch.h"

#include "IMessageBlockManager.h"

typedef  ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> MUTEX_MALLOC;
typedef ACE_Allocator_Adapter<MUTEX_MALLOC> Mutex_Allocator;

class CMessageBlockManager : public IMessageBlockManager
{
public:
	CMessageBlockManager(void);
	virtual ~CMessageBlockManager(void);

	void Init();
	void Close();

	ACE_Message_Block* Create(uint32 u4Size);
	bool Close(ACE_Message_Block* pMessageBlock);

private:
	ACE_Allocator * m_pmsgallocator;
	ACE_Allocator * m_pdata_allocator;
	ACE_Allocator * m_pbuff_allocator;

	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;                     //���ƶ��߳���
};

typedef ACE_Singleton<CMessageBlockManager,ACE_Null_Mutex> App_MessageBlockManager; 
#endif
