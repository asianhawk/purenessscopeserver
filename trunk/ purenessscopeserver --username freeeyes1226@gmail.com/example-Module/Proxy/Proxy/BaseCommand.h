#pragma once
#include "ace/Thread.h" 

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "ProxyClient.h"

#include <string>

//����ͻ�������(TCP)
#define COMMAND_PROXY           0x1030

#define PROXY_SERVER_IP   "127.0.0.1"
#define PROXY_SERVER_PORT 10040

using namespace std;

class CBaseCommand : public CClientCommand
{
public:
  CBaseCommand(void);
  ~CBaseCommand(void);

  int DoMessage(IMessage* pMessage, bool& bDeleteFlag);
  void SetServerObject(CServerObject* pServerObject);
  void Init();

private:
	void Do_Proxy_Connect(IMessage* pMessage);          //�������������ӽ���  
	void Do_Proxy_DisConnect(IMessage* pMessage);       //�����ӷ������Ͽ�����
	void Do_Proxy_Data(IMessage* pMessage);             //���ʹ�������

private:
  CProxyClientConnector    m_objReactorConnect;
  CServerObject*           m_pServerObject;
  int                      m_nCount;

};
