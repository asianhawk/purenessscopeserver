#ifndef _PROCONTROLLISTEN_H
#define _PROCONTROLLISTEN_H

#include "ProConnectAccept.h"
#include "AceProactorManager.h"
#include "IControlListen.h"

//���ӹ����࣬���ڴ���ɾ���Ͳ鿴����
//add by freeeyes
class CProControlListen : public IControlListen
{
public:
	CProControlListen();
	virtual ~CProControlListen();

	bool AddListen(const char* pListenIP, uint32 u4Port, uint8 u1IPType);  //��һ���µļ����˿�
	bool DelListen(const char* pListenIP, uint32 u4Port);                  //�ر�һ����֪������
	void ShowListen(vecControlInfo& objControlInfo);                       //�鿴�Ѵ򿪵ļ����˿�
};

typedef ACE_Singleton<CProControlListen, ACE_Null_Mutex> App_ProControlListen;

#endif
