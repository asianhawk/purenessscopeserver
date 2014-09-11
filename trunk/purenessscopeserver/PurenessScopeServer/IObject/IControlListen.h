#ifndef _ICONTROLLISTEN_H
#define _ICONTROLLISTEN_H

#include "define.h"

struct _ControlInfo
{
	char m_szListenIP[MAX_BUFF_20];
	uint32 m_u4Port;

	_ControlInfo()
	{
		m_szListenIP[0] = '\0';
		m_u4Port        = 0;
	}
};

typedef vector<_ControlInfo> vecControlInfo;

class IControlListen
{
public:
	virtual bool AddListen(const char* pListenIP, uint32 u4Port, uint8 u1IPType) = 0;  //��һ���µļ����˿�
	virtual bool DelListen(const char* pListenIP, uint32 u4Port)                 = 0;  //�ر�һ����֪������
	virtual void ShowListen(vecControlInfo& objControlInfo)                      = 0;  //�鿴�Ѵ򿪵ļ����˿�
};

#endif
