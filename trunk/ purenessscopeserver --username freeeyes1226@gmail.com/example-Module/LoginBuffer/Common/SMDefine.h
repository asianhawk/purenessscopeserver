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
	READERINITSTATE = 0,   //�����ڴ�δ��ʼ������
	READERINITSTATED,      //�����ڴ��Ѿ���ʼ������
};

enum EM_CheckState
{
	CHECKS_HIT = 0,      //���У������Լ�����Ѿ��ѵ���ǰ�����ڴ����ݣ�
	CHECKS_UNHIT,        //�����У������Լ����û���ѵ���ǰ�����ڴ����ݣ�
};

//�����ڴ�ͷ
struct _SMHeader
{
	ID_t           m_HeaderID;          //ͷID
	uint32         m_u4DataOffset;      //�׵�ַƫ�ƣ�Ҳ����������T*�ĵ�ַ
	ACE_Time_Value m_tvUpdate;          //���ݱ���ȡ����ʱ�� 

	_SMHeader()
	{
		m_HeaderID     = INVALID_ID;
		m_u4DataOffset = INVALID_OFFSET;
	}
};

//���ڻ��嵱ǰ״̬�����ݽṹ
struct _CacheBlock
{
private:
	EM_CheckState  m_u1State;           //��ǰά��״̬����ӦEM_CheckState
	bool           m_blOnline;          //���߱�ǣ�trueΪ���ߣ�falseΪ���� 
	bool           m_blDelete;          //ɾ�����Ϊ��Watch����ά�������ǣ�falseΪ����ʹ�ã�trueΪ�������Ѿ�ɾ�� 

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

	//����ɾ��״̬
	void SetDelete(bool blDelete)
	{
		m_blDelete = blDelete;
	}

	//�õ�ɾ��״̬
	bool GetDelete()
	{
		return m_blDelete;
	}

	//�õ��Ƿ�����״̬
	EM_CheckState GetCheckState()
	{
		return m_u1State;
	}

	//��������״̬
	void SetCheckState(EM_CheckState objCheckState)
	{
		m_u1State = objCheckState;
	}
};

#endif
