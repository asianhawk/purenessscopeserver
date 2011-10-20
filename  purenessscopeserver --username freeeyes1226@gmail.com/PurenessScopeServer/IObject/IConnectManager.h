#ifndef _ICONNECTMANAGER_H
#define _ICONNECTMANAGER_H

#include "IBuffPacket.h"

class IConnectManager
{
public:
	virtual ~IConnectManager() {};
	virtual bool SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket) = 0;
	virtual bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket) = 0;
	virtual bool CloseConnect(uint32 u4ConnectID)                          = 0;
	virtual _ClientIPInfo GetClientIPInfo(uint32 u4ConnectID)              = 0;
	virtual int  GetCount()                                                = 0;
};

#endif
