#pragma once

#include <WinSock2.h>

class CTcpClientConnect
{
public:
  CTcpClientConnect(void);
  ~CTcpClientConnect(void);

  void Init(const char* pIp, int nPort);

  bool SendConsoleMessage(const char* pMessage, int nMessageLen, char* pRevBuff, int& nRecvLen);

private:
  char m_szServerIP[20];
  int  m_nServerPort;
};
