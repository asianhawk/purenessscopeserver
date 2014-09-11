#ifndef _CONTROLLISTEN_H
#define _CONTROLLISTEN_H

#include "ConnectAccept.h"
#include "AceReactorManager.h"
#include "IControlListen.h"

class CControlListen : public IControlListen
{
public:
	CControlListen();
	virtual ~CControlListen();

	bool AddListen(const char* pListenIP, uint32 u4Port, uint8 u1IPType);  //��һ���µļ����˿�
	bool DelListen(const char* pListenIP, uint32 u4Port);                  //�ر�һ����֪������
	void ShowListen(vecControlInfo& objControlInfo);                       //�鿴�Ѵ򿪵ļ����˿�
};

typedef ACE_Singleton<CControlListen, ACE_Null_Mutex> App_ControlListen;

#endif
