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
	STATE_OPEN = 0,    //���ݿ����
	STATE_USED,        //���ݿ�����ʹ��
	STATE_INVALID,     //��ǰ���ݿ���Ч      
};

enum EM_ReaderInitState
{
	READERINITSTATE = 0,
	READERINITSTATED,
};

//�����ڴ�ͷ
struct _SMHeader
{
	ID_t           m_HeaderID;          //ͷID
	uint32         m_u4DataOffset;      //�׵�ַƫ�ƣ�Ҳ����������T*�ĵ�ַ
	EM_DataState   m_DataState;         //��ǰ����״̬
	ACE_Time_Value m_tvUpdate;          //���ݱ���ȡ����ʱ�� 

	_SMHeader()
	{
		m_HeaderID     = INVALID_ID;
		m_u4DataOffset = INVALID_OFFSET;
		m_DataState    = STATE_INVALID;
	}
};

#endif
