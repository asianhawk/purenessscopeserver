#include "BaseCommand.h"
#ifndef __LINUX__
#include "ace/Select_Reactor.h"
#else
#include "ace/Dev_Poll_Reactor.h"
#endif

//ȫ�ֵ�Reactor��Ӧ��������ת��ר��ʹ�á�
ACE_Reactor* g_pReactor = NULL;

void* Worker(void *arg) 
{
#ifndef __LINUX__	
	OUR_DEBUG((LM_ERROR, "[Worker]***__WINDOWS__****.\n"));
	ACE_Select_Reactor* selectreactor = new ACE_Select_Reactor();
	g_pReactor = new ACE_Reactor(selectreactor, 1);
	//g_pReactor = ACE_Reactor::instance();
#else
	OUR_DEBUG((LM_ERROR, "[Worker]***__LINUX__****.\n"));
	//�����Linux����������ΪĬ��ʹ��epoll������ļ��������������Ҫ������Ҫ����һ��
	//ĿǰĬ�ϵ���1000
	//�ļ��򿪾����ֵΪ����ǰ��ֵ+ǰ������򿪵����������+10
	ACE_Dev_Poll_Reactor* devreactor = new ACE_Dev_Poll_Reactor(1000);
	g_pReactor = new ACE_Reactor(devreactor, 1);
#endif	
	
	while(true)
	{
		if(g_pReactor != NULL)
		{
#ifndef __LINUX__
			g_pReactor->handle_events();
#else
			g_pReactor->run_reactor_event_loop(); 
#endif
		}
	}
	
	return NULL; 
} 


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
	//������Ӧ���߳�
	ACE_thread_t threadId;
	ACE_hthread_t threadHandle;

	ACE_Thread::spawn(
		(ACE_THR_FUNC)Worker,        //�߳�ִ�к���
		NULL,                        //ִ�к�������
		THR_JOINABLE | THR_NEW_LWP,
		&threadId,
		&threadHandle
		);
	
	//�ȴ�10����󣬰�g_pReactor	
	ACE_Time_Value tvSleep(0, 10 * 1000);
	ACE_OS::sleep(tvSleep);		
		
	m_objReactorConnect.open(g_pReactor);	
}

void CBaseCommand::Do_Proxy_Connect(IMessage* pMessage)
{
	ACE_INET_Addr AddrProxyServer;
	AddrProxyServer.set((int)PROXY_SERVER_PORT, PROXY_SERVER_IP);

	CProxyClient* pProxyClient = new CProxyClient();
	if(NULL == pProxyClient)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Connect]pProxyClient new is NULL.\n"));
		return;
	}

	//���ù�����ϵ
	pProxyClient->SetServerObject(pMessage->GetMessageBase()->m_u4ConnectID, m_pServerObject);
	pProxyClient->reactor(g_pReactor);

	//����Զ�̷�����
	int nError = m_objReactorConnect.connect(pProxyClient, AddrProxyServer);
	if(nError != 0)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Connect]Connect Fail.\n"));
		return;
	}
}

void CBaseCommand::Do_Proxy_DisConnect( IMessage* pMessage )
{
	CProxyClient* pProxyClient = App_ProxyManager::instance()->FindProxyClient(pMessage->GetMessageBase()->m_u4ConnectID);
	if(NULL != pProxyClient)
	{
		//��Ҫ�ر�Զ�̵�����
		pProxyClient->Close();
	}
}

void CBaseCommand::Do_Proxy_Data( IMessage* pMessage )
{
	CProxyClient* pProxyClient = App_ProxyManager::instance()->FindProxyClient(pMessage->GetMessageBase()->m_u4ConnectID);
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