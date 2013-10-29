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

enum EM_DataState
{
	STATE_OPEN = 0,    //数据块空闲
	STATE_USED,        //数据块正在使用
	STATE_INVALID,     //当前数据块无效      
};

enum EM_ReaderInitState
{
	READERINITSTATE = 0,
	READERINITSTATED,
};

//共享内存头
struct _SMHeader
{
	ID_t           m_HeaderID;          //头ID
	uint32         m_u4DataOffset;      //首地址偏移，也就是数据体T*的地址
	EM_DataState   m_DataState;         //当前数据状态
	ACE_Time_Value m_tvUpdate;          //数据被读取处理时间 

	_SMHeader()
	{
		m_HeaderID     = INVALID_ID;
		m_u4DataOffset = INVALID_OFFSET;
		m_DataState    = STATE_INVALID;
	}
};

#endif
