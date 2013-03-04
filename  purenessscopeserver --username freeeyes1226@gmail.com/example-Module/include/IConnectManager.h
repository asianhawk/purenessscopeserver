#ifndef _ICONNECTMANAGER_H
#define _ICONNECTMANAGER_H

#include "IBuffPacket.h"

class IConnectManager
{
public:
	virtual ~IConnectManager() {};
	virtual bool PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket, uint8 u1SendType = SENDMESSAGE_NOMAL, uint16 u2CommandID = 0, bool blSendState = true)           = 0;
  virtual bool PostMessage(uint32 u4ConnectID, const char* pData, uint32 nDataLen, uint8 u1SendType = SENDMESSAGE_NOMAL, uint16 u2CommandID = 0, bool blSendState = true) = 0;
	virtual bool CloseConnect(uint32 u4ConnectID)                                                                = 0;
	virtual _ClientIPInfo GetClientIPInfo(uint32 u4ConnectID)                                                    = 0;
	virtual int  GetCount()                                                                                      = 0;
};

#endif
