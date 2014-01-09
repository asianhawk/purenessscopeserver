#include "BaseCommand.h"
#ifndef __LINUX__
#include "ace/Select_Reactor.h"
#else
#include "ace/Dev_Poll_Reactor.h"
#endif

//全局的Reactor反应器，用于转发专门使用。
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
	//如果是Linux，这里设置为默认使用epoll，最大文件句柄数在这里需要根据需要设置一下
	//目前默认的是1000
	//文件打开句柄数值为，当前数值+前端允许打开的最大连接数+10
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
  //__ENTER_FUNCTION是自动给你的函数追加try的宏，用于捕捉错误，你可以使用__THROW_FUNCTION()抛出你的错误
  //比如__THROW_FUNCTION("hello"); 它会自动在Log文件夹下的assert.log记录文件名，函数名，行数，描述。
  //推荐你使用这个宏，记住一定要在函数的结束使用__LEAVE_FUNCTION或者__LEAVE_FUNCTION_WITHRETURN与之配对。
  //__LEAVE_FUNCTION_WITHRETURN和__LEAVE_FUNCTION不同是前者可以支持函数退出的时候返回一个变量
  //比如这个函数是返回一个int
  //那么末尾就是__LEAVE_FUNCTION_WITHRETURN(0); 0就是返回的int，当然，也支持别的类型变量，具体看你自己的函数。
  //如果函数不返回任何变量，你可以使用__LEAVE_FUNCTION即可。
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

  //处理链接建立信息
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CONNECT)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CONNECT OK.\n"));
	Do_Proxy_Connect(pMessage);
    return 0;
  }

  //处理链接断开信息
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CDISCONNET)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CDISCONNET OK.\n"));
	Do_Proxy_DisConnect(pMessage);
    return 0;
  }

  //处理正常信息
  if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_PROXY)
  {
	  Do_Proxy_Data(pMessage);
  }

  return 0;

  __LEAVE_FUNCTION_WITHRETURN(0);
}

void CBaseCommand::Init()
{
	//启动反应器线程
	ACE_thread_t threadId;
	ACE_hthread_t threadHandle;

	ACE_Thread::spawn(
		(ACE_THR_FUNC)Worker,        //线程执行函数
		NULL,                        //执行函数参数
		THR_JOINABLE | THR_NEW_LWP,
		&threadId,
		&threadHandle
		);
	
	//等待10毫秒后，绑定g_pReactor	
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

	//设置关联关系
	pProxyClient->SetServerObject(pMessage->GetMessageBase()->m_u4ConnectID, m_pServerObject);
	pProxyClient->reactor(g_pReactor);

	//连接远程服务器
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
		//需要关闭远程的连接
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

		//组装转发包
		ACE_OS::memcpy(szPostData, (char* )&BodyPacket.m_nDataLen, sizeof(uint32));	
		ACE_OS::memcpy(&szPostData[4], BodyPacket.m_pData, BodyPacket.m_nDataLen);	

		pProxyClient->SendData(szPostData, BodyPacket.m_nDataLen + sizeof(uint32));
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::Do_Proxy_Data]pProxyClient is Lost.\n"));
	}
}