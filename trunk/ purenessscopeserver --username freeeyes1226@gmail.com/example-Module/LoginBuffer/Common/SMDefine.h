#ifndef DEFINE_H
#define DEFINE_H

#include "define.h"
#include <ace/Basic_Types.h>
#include <ace/Time_Value.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/OS_NS_string.h>

typedef ACE_UINT32 ID_t;

#define INVALID_ID     0
#define INVALID_OFFSET 0

enum EM_ReaderInitState
{
	READERINITSTATE = 0,   //共享内存未初始化数据
	READERINITSTATED,      //共享内存已经初始化数据
};

enum EM_CheckState
{
	CHECKS_HIT = 0,      //命中（缓冲自检程序已经搜到当前共享内存数据）
	CHECKS_UNHIT,        //无命中（缓冲自检程序没有搜到当前共享内存数据）
};

//共享内存头
struct _SMHeader
{
	ID_t           m_HeaderID;          //头ID
	uint32         m_u4DataOffset;      //首地址偏移，也就是数据体T*的地址
	ACE_Time_Value m_tvUpdate;          //数据被读取处理时间 

	_SMHeader()
	{
		m_HeaderID     = INVALID_ID;
		m_u4DataOffset = INVALID_OFFSET;
	}
};

//用于缓冲当前状态的数据结构
struct _CacheBlock
{
private:
	EM_CheckState  m_u1State;           //当前维护状态，对应EM_CheckState
	bool           m_blOnline;          //在线标记，true为在线，false为离线 
	bool           m_blDelete;          //删除标记为，Watch进程维护这个标记，false为正在使用，true为该数据已经删除 

public:
	_CacheBlock()
	{
		m_u1State       = CHECKS_HIT;
		m_blDelete      = false;
		m_blOnline      = false;
	}
	
	void SetHit()
	{
		m_u1State       = CHECKS_HIT;
		m_blDelete      = false;
		m_blOnline      = false;
	}

	void SetUnHit()
	{
		m_blDelete     = true;
		m_blOnline     = false;
		m_u1State      = CHECKS_UNHIT;
	}

	//设置删除状态
	void SetDelete(bool blDelete)
	{
		m_blDelete = blDelete;
	}

	//得到删除状态
	bool GetDelete()
	{
		return m_blDelete;
	}

	//得到是否命中状态
	EM_CheckState GetCheckState()
	{
		return m_u1State;
	}

	//设置命中状态
	void SetCheckState(EM_CheckState objCheckState)
	{
		m_u1State = objCheckState;
	}
};

#endif
