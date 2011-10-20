#ifndef _ICLIENTMANAGER_H
#define _ICLIENTMANAGER_H

#include "ClientMessage.h"
#include "ClientUDPMassage.h"

//负责管理服务器间通讯的数据管理
class IClientManager
{
public:
	virtual ~IClientManager() {};

	virtual bool Connect(int nServerID, const char* pIP, int nPort, IClientMessage* pClientMessage)          = 0;
	virtual bool ConnectUDP(int nServerID, const char* pIP, int nPort, IClientUDPMessage* pClientUDPMessage) = 0;
	virtual bool Close(int nServerID)                                                                        = 0;
	virtual bool CloseUDP(int nServerID)                                                                     = 0;
	virtual bool SendData(int nServerID, const char* pData, int nSize)                                       = 0;
	virtual bool SendDataUDP(int nServerID, const char* pIP, int nPort, const char* pMessage, uint32 u4Len)  = 0;
	virtual bool StartConnectTask(int nIntervalTime)                                                         = 0;                             
	virtual void CancelConnectTask()                                                                         = 0;                                                                  
	virtual void Close()                                                                                     = 0;
};

#endif
