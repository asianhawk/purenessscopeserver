#include "BaseCommand.h"

CBaseCommand::CBaseCommand(void)
{
  m_pServerObject = NULL;
  m_nCount        = 0;
}

CBaseCommand::~CBaseCommand(void)
{
}

void CBaseCommand::SetServerObject(CServerObject* pServerObject)
{
  m_pServerObject = pServerObject;
}

int CBaseCommand::DoMessage(IMessage* pMessage, bool& bDeleteFlag)
{
  //__ENTER_FUNCTION���Զ�����ĺ���׷��try�ĺ꣬���ڲ�׽���������ʹ��__THROW_FUNCTION()�׳���Ĵ���
  //����__THROW_FUNCTION("hello"); �����Զ���Log�ļ����µ�assert.log��¼�ļ�������������������������
  //�Ƽ���ʹ������꣬��סһ��Ҫ�ں����Ľ���ʹ��__LEAVE_FUNCTION����__LEAVE_FUNCTION_WITHRETURN��֮��ԡ�
  //__LEAVE_FUNCTION_WITHRETURN��__LEAVE_FUNCTION��ͬ��ǰ�߿���֧�ֺ����˳���ʱ�򷵻�һ������
  //������������Ƿ���һ��int
  //��ôĩβ����__LEAVE_FUNCTION_WITHRETURN(0); 0���Ƿ��ص�int����Ȼ��Ҳ֧�ֱ�����ͱ��������忴���Լ��ĺ�����
  //��������������κα����������ʹ��__LEAVE_FUNCTION���ɡ�
  __ENTER_FUNCTION();

  if(m_pServerObject == NULL)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] m_pServerObject is NULL.\n"));
    return -1;
  }

  if(pMessage == NULL)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] pMessage is NULL.\n"));
    return -1;
  }

  //�������ӽ�����Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CONNECT)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CONNECT OK.\n"));
	Do_Proxy_Connect(pMessage);
    return 0;
  }

  //�������ӶϿ���Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CDISCONNET)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CDISCONNET OK.\n"));
	Do_Proxy_DisConnect(pMessage);
    return 0;
  }

  //����������Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_PROXY)
  {
	  Do_Proxy_Data(pMessage);
  }

  return 0;

  __LEAVE_FUNCTION_WITHRETURN(0);
}

void CBaseCommand::Init()
{
	uint32 u4WorkThreadCount = m_pServerObject->GetMessageManager()->GetWorkThreadCount();
	OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] WorkThread Count=%d.\n", u4WorkThreadCount));

	App_ProxyThreadManager::instance()->Init(u4WorkThreadCount);


}

void CBaseCommand::Do_Proxy_Connect(IMessage* pMessage)
{
	ACE_INET_Addr AddrProxyServer;
	AddrProxyServer.set((int)PROXY_SERVER_PORT, PROXY_SERVER_IP);

	//��õ�ǰ�Ĺ����߳�ID
	uint32 u4ThreadID = pMessage->GetMessageBase()->m_u4WorkThreadID;

	CProxyClient* pProxyClient = new CProxyClient();
	if(NULL == pProxyClient)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Connect]pProxyClient new is NULL.\n"));
		return;
	}

	//���ù�����ϵ
	pProxyClient->SetWorkThreadID(u4ThreadID);
	pProxyClient->SetServerObject(pMessage->GetMessageBase()->m_u4ConnectID, m_pServerObject);
	pProxyClient->reactor(App_ProxyThreadManager::instance()->GetProxyClientReactor(u4ThreadID));

	//����Զ�̷�����
	int nError = App_ProxyThreadManager::instance()->GetProxyClientConnector(u4ThreadID)->connect(pProxyClient, AddrProxyServer);
	if(nError != 0)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Connect]Connect Fail.\n"));
		return;
	}
}

void CBaseCommand::Do_Proxy_DisConnect( IMessage* pMessage )
{
	//��õ�ǰ�Ĺ����߳�ID
	uint32 u4ThreadID = pMessage->GetMessageBase()->m_u4WorkThreadID;

	CProxyClient* pProxyClient = App_ProxyThreadManager::instance()->GetProxyClientManager(u4ThreadID)->FindProxyClient(pMessage->GetMessageBase()->m_u4ConnectID);
	if(NULL != pProxyClient)
	{
		//��Ҫ�ر�Զ�̵�����
		pProxyClient->Close();
	}
}

void CBaseCommand::Do_Proxy_Data( IMessage* pMessage )
{
	//��õ�ǰ�Ĺ����߳�ID
	uint32 u4ThreadID = pMessage->GetMessageBase()->m_u4WorkThreadID;

	CProxyClient* pProxyClient =  App_ProxyThreadManager::instance()->GetProxyClientManager(u4ThreadID)->FindProxyClient(pMessage->GetMessageBase()->m_u4ConnectID);
	if(NULL != pProxyClient)
	{
		_PacketInfo BodyPacket;
		pMessage->GetPacketBody(BodyPacket);

		char szPostData[MAX_RECV_BUFF] = {'\0'};
		int nSendSize = BodyPacket.m_nDataLen;

		if(nSendSize > MAX_RECV_BUFF)
		{
			OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Data]data is more than MAX_RECV_BUFF.\n"));
			return;
		}

		//��װת����
		ACE_OS::memcpy(szPostData, (char* )&BodyPacket.m_nDataLen, sizeof(uint32));	
		ACE_OS::memcpy(&szPostData[4], BodyPacket.m_pData, BodyPacket.m_nDataLen);	

		pProxyClient->SendData(szPostData, BodyPacket.m_nDataLen + sizeof(uint32));
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Data]pProxyClient is Lost.\n"));
	}
}