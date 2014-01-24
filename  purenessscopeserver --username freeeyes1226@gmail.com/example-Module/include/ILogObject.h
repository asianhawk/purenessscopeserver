#ifndef _ILOGOBJECT
#define _ILOGOBJECT

#include "ace/SString.h"

//��־�����ݽṹ
struct _LogBlockInfo
{
public:
	char*  m_pBlock;      //��ָ��
	uint32 m_u4Length;    //�鳤��
	bool   m_blIsUsed;    //�Ƿ�����ʹ��

	_LogBlockInfo()
	{
		m_pBlock   = NULL;
		m_u4Length = 0;
		m_blIsUsed = false;
	}
};

//��־�����(��һ�����࣬�ɱ����̳�ʵ���ڲ�)
class CServerLogger 
{
public:
	virtual ~CServerLogger() {};

	virtual int DoLog(int nLogType, _LogBlockInfo* pLogBlockInfo) = 0;
	virtual int GetLogTypeCount()                                 = 0;
	virtual int GetLogType(int nIndex)                            = 0;

	virtual uint32 GetBlockSize()                                 = 0;
	virtual uint32 GetPoolCount()                                 = 0;
};
#endif
