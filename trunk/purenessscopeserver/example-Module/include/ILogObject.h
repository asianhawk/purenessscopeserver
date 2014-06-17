#ifndef _ILOGOBJECT
#define _ILOGOBJECT

#include "ace/SString.h"

//��־�����ݽṹ
struct _LogBlockInfo
{
public:
	char*  m_pBlock;                     //��ָ��
	uint32 m_u4Length;                   //�鳤��
	bool   m_blIsUsed;                   //�Ƿ�����ʹ��
	uint32 m_u4LogID;                    //LogID���
	uint32 m_u4MailID;                   //�ʼ������ID���
	char   m_szMailTitle[MAX_BUFF_200];  //�ʼ�����ı���

	_LogBlockInfo()
	{
		m_pBlock         = NULL;
		m_u4Length       = 0;
		m_u4LogID        = 0;
		m_blIsUsed       = false;
		m_u4MailID       = 0;
		m_szMailTitle[0] = '\0';
	}

	void clear()
	{
		m_pBlock         = NULL;
		m_u4Length       = 0;
		m_u4LogID        = 0;
		m_blIsUsed       = false;
		m_u4MailID       = 0;
		m_szMailTitle[0] = '\0';
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

	virtual uint32 GetCurrLevel()                                 = 0;
	virtual uint16 GetLogID(uint16 u2Index)                       = 0;
	virtual char*  GetLogInfoByServerName(uint16 u2LogID)         = 0;
	virtual char*  GetLogInfoByLogName(uint16 u2LogID)            = 0;
	virtual int    GetLogInfoByLogDisplay(uint16 u2LogID)         = 0;
	virtual bool   ReSet(uint32 u4CurrLogLevel)                   = 0;
};
#endif
