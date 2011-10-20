// IObject.h
// 虚接口化所有DLL用到的类
// add by freeeyes
// 2008-04-06

#ifndef _IOBJECT_H
#define _IOBJECT_H

#include "IMessage.h"
#include "IMessageManager.h"
#include "ILogManager.h"
#include "IConnectManager.h"
#include "IPacketManager.h"
#include "IClientManager.h"
#include "IUDPConnectManager.h"

class CServerObject
{
public:
	CServerObject() 
	{ 
		m_pIMessageManager   = NULL;
		m_pLogManager        = NULL;
		m_pConnectManager    = NULL;
		m_pPacketManager     = NULL;
		m_pClientManager     = NULL;
		m_pUDPConnectManager = NULL;
	}

	~CServerObject() {};
	void SetMessageManager(IMessageManager* pIMessageManager) { m_pIMessageManager = pIMessageManager; };
	void SetLogManager(ILogManager* pLogManager) { m_pLogManager = pLogManager; };
	void SetConnectManager(IConnectManager* pConnectManager) { m_pConnectManager = pConnectManager; };
	void SetPacketManager(IPacketManager*  pPacketManager) { m_pPacketManager = pPacketManager; };
	void SetClientManager(IClientManager* pClientManager) { m_pClientManager = pClientManager; }
	void SetUDPConnectManager(IUDPConnectManager* pUDPConnectManager) { m_pUDPConnectManager = pUDPConnectManager; };

	IMessageManager*    GetMessageManager() { return m_pIMessageManager; };
	ILogManager*        GetLogManager() { return m_pLogManager; };
	IConnectManager*    GetConnectManager() { return m_pConnectManager; };
	IPacketManager*     GetPacketManager() { return m_pPacketManager; };
	IClientManager*     GetClientManager() { return m_pClientManager; }
	IUDPConnectManager* GetUDPConnectManager() { return m_pUDPConnectManager; };

private:
	IMessageManager*    m_pIMessageManager;
	ILogManager*        m_pLogManager;
	IConnectManager*    m_pConnectManager;
	IPacketManager*     m_pPacketManager;
	IClientManager*     m_pClientManager;
	IUDPConnectManager* m_pUDPConnectManager;
};

typedef ACE_Singleton<CServerObject, ACE_Null_Mutex> App_ServerObject; 

#endif
