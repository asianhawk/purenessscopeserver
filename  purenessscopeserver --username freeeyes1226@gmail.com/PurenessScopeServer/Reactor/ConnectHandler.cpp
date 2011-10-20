#include "ConnectHandler.h"

Mutex_Allocator _msg_send_mb_allocator;

CConnectHandler::CConnectHandler(void)
{
	m_szError[0]       = '\0';
	m_u4ConnectID      = 0;
	m_u2SendCount      = 0;
	m_u4AllRecvCount   = 0;
	m_u4AllSendCount   = 0;
	m_u4AllRecvSize    = 0;
	m_u4AllSendSize    = 0;
	m_nIOCount         = 1;
	m_u4SendThresHold  = MAX_MSG_SNEDTHRESHOLD;
	m_u2SendQueueMax   = MAX_MSG_SENDPACKET;
	m_u1ConnectState   = CONNECT_INIT;
	m_u1SendBuffState  = CONNECT_SENDNON;
	m_pTCClose         = NULL;
	m_u1IsClosing      = HANDLE_ISCLOSE_NO;
	m_pCurrMessage     = NULL;
	m_pPacketParse     = NULL;
	m_u4CurrSize       = 0;
	m_u4HandlerID      = 0;
	m_u2MaxConnectTime = 0;
	m_u4MaxPacketSize  = MAX_MSG_PACKETLENGTH;
}

CConnectHandler::~CConnectHandler(void)
{
	SAFE_DELETE(m_pTCClose);
}

const char* CConnectHandler::GetError()
{
	return m_szError;
}

bool CConnectHandler::Close(int nIOCount)
{
	m_ThreadLock.acquire();
	if(nIOCount > m_nIOCount)
	{
		m_nIOCount = 0;
	}

	if(m_nIOCount > 0)
	{
		m_nIOCount -= nIOCount;
	}
	m_ThreadLock.release();

	//OUR_DEBUG((LM_ERROR, "[CConnectHandler::Close]ConnectID=%d,m_nIOCount=%d.\n", GetConnectID(), m_nIOCount));

	//从反应器注销事件
	if(m_nIOCount == 0)
	{
		//删除对象缓冲的PacketParse
		if(m_pCurrMessage != NULL)
		{
			m_pCurrMessage->release();
		}

		//msg_queue()->deactivate();
		shutdown();
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::Close]Close(%d) OK.\n", GetConnectID()));
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
		
		//删除链接对象
		App_ConnectManager::instance()->CloseConnect(GetConnectID());
		
		//回归用过的指针
		App_ConnectHandlerPool::instance()->Delete(this);
		return true;
	}

	return false;
}

void CConnectHandler::Init(uint16 u2HandlerID)
{
	m_u4HandlerID      = u2HandlerID;
	m_u2MaxConnectTime = App_MainConfig::instance()->GetMaxConnectTime();
	m_u4SendThresHold  = App_MainConfig::instance()->GetSendThresHold();
	m_u2SendQueueMax   = App_MainConfig::instance()->GetSendQueueMax();
	m_u4MaxPacketSize  = App_MainConfig::instance()->GetRecvBuffSize();
}

bool CConnectHandler::ServerClose()
{
	OUR_DEBUG((LM_ERROR, "[CConnectHandler::ServerClose]Close(%d) OK.\n", GetConnectID()));
	AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);

	//发送客户端链接断开消息。
	if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_SDISCONNECT, NULL))
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
	}

	//msg_queue()->deactivate();
	shutdown();

	m_u1ConnectState = CONNECT_SERVER_CLOSE;

	return true;
}

void CConnectHandler::SetConnectID(uint32 u4ConnectID)
{
	m_u4ConnectID = u4ConnectID;
}

uint32 CConnectHandler::GetConnectID()
{
	return m_u4ConnectID;
}

int CConnectHandler::open(void*)
{
	m_nIOCount = 1;
	
	if(App_ForbiddenIP::instance()->CheckIP(m_addrRemote.get_host_addr()) == false)
	{
		//在禁止列表中，不允许访问
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::open]IP Forbidden(%s).\n", m_addrRemote.get_host_addr()));
		return -1;
	}

	//检查单位时间链接次数是否达到上限
	if(false == App_IPAccount::instance()->AddIP((string)m_addrRemote.get_host_addr()))
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::open]IP connect frequently.\n", m_addrRemote.get_host_addr()));
		App_ForbiddenIP::instance()->AddTempIP(m_addrRemote.get_host_addr(), App_MainConfig::instance()->GetForbiddenTime());
		return -1;
	}

	//初始化检查器
	m_TimeConnectInfo.Init(App_MainConfig::instance()->GetValid(), App_MainConfig::instance()->GetValidPacketCount(), App_MainConfig::instance()->GetValidRecvSize());
	
	int nRet = ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>::open();
	if(nRet != 0)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::open]ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>::open() error [%d].\n", nRet));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::open]ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>::open() error [%d].", nRet);
		return -1;
	}

	//设置链接为非阻塞模式
	if (this->peer().enable(ACE_NONBLOCK) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::open]this->peer().enable  = ACE_NONBLOCK error.\n"));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::open]this->peer().enable  = ACE_NONBLOCK error.");
		return -1;
	}

	//获得远程链接地址和端口
	if(this->peer().get_remote_addr(m_addrRemote) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::open]this->peer().get_remote_addr error.\n"));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::open]this->peer().get_remote_addr error.");
		return -1;
	}

	OUR_DEBUG((LM_INFO, "[CConnectHandler::open] Connection from [%s:%d]\n",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number()));
	
	m_atvConnect      = ACE_OS::gettimeofday();
	m_atvInput        = ACE_OS::gettimeofday();
	m_atvOutput       = ACE_OS::gettimeofday();
	m_atvSendAlive    = ACE_OS::gettimeofday();

	m_u4AllRecvCount  = 0;
	m_u4AllSendCount  = 0;
	m_u4AllRecvSize   = 0;
	m_u4AllSendSize   = 0;

	//设置接收缓冲池的大小
	int nTecvBuffSize = MAX_MSG_SOCKETBUFF;
	//ACE_OS::setsockopt(this->get_handle(), SOL_SOCKET, SO_RCVBUF, (char* )&nTecvBuffSize, sizeof(nTecvBuffSize));
	ACE_OS::setsockopt(this->get_handle(), SOL_SOCKET, SO_SNDBUF, (char* )&nTecvBuffSize, sizeof(nTecvBuffSize));
	//int nOverTime = MAX_MSG_SENDTIMEOUT;
	//ACE_OS::setsockopt(this->get_handle(), SOL_SOCKET, SO_SNDTIMEO, (char* )&nOverTime, sizeof(nOverTime));

	m_pPacketParse = App_PacketParsePool::instance()->Create();
	if(NULL == m_pPacketParse)
	{
		OUR_DEBUG((LM_DEBUG,"[%t|CConnectHandle::open] Open(%d) m_pPacketParse new error.\n", GetConnectID()));
		return -1;
	}

	//申请头的大小对应的mb
	m_pCurrMessage = App_MessageBlockManager::instance()->Create(m_pPacketParse->GetPacketHeadLen());
	if(m_pCurrMessage == NULL)
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
		OUR_DEBUG((LM_ERROR, "[CConnectHandle::RecvClinetPacket] pmb new is NULL.\n"));

		App_ConnectManager::instance()->Close(GetConnectID());
		return -1;
	}

	//将这个链接放入链接库
	if(false == App_ConnectManager::instance()->AddConnect(this))
	{
		OUR_DEBUG((LM_ERROR, "%s.\n", App_ConnectManager::instance()->GetError()));
		sprintf_safe(m_szError, MAX_BUFF_500, "%s", App_ConnectManager::instance()->GetError());
		return -1;
	}
	else
	{
		OUR_DEBUG((LM_DEBUG,"[CConnectHandle::open] Open ConnectID[%d].\n", GetConnectID()));	
	}

	AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Connection from [%s:%d].",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number());

	//发送链接建立消息。
	if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CONNECT, NULL))
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
	}

	m_u1ConnectState = CONNECT_OPEN;


	return nRet;
}

//接受数据
int CConnectHandler::handle_input(ACE_HANDLE fd)
{
	m_ThreadLock.acquire();
	m_nIOCount++;
	m_ThreadLock.release();
	//OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input]ConnectID=%d,m_nIOCount=%d.\n", GetConnectID(), m_nIOCount));

	ACE_Time_Value nowait(MAX_MSG_PACKETTIMEOUT);

	m_atvInput = ACE_OS::gettimeofday();

	if(fd == ACE_INVALID_HANDLE)
	{
		m_u4CurrSize = 0;
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input]fd == ACE_INVALID_HANDLE.\n"));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::handle_input]fd == ACE_INVALID_HANDLE.");

		//发送客户端链接断开消息。
		if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
		{
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
		}

		return -1;
	}

	//判断数据包结构是否为NULL
	if(m_pPacketParse == NULL)
	{
		m_u4CurrSize = 0;
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input]m_pPacketParse == NULL.\n"));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::handle_input]m_pPacketParse == NULL.");

		//发送客户端链接断开消息。
		if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
		{
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
		}

		return -1;
	}

	//判断缓冲是否为NULL
	if(m_pCurrMessage == NULL)
	{
		m_u4CurrSize = 0;
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input]m_pCurrMessage == NULL.\n"));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::handle_input]m_pCurrMessage == NULL.");

		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			m_pPacketParse->GetMessageHead()->release();
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			m_pPacketParse->GetMessageBody()->release();
		}

		if(m_pCurrMessage != NULL && m_pPacketParse->GetMessageBody() != m_pCurrMessage && m_pPacketParse->GetMessageBody() != m_pCurrMessage)
		{
			m_pCurrMessage->release();
			m_pCurrMessage = NULL;
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);

		//发送客户端链接断开消息。
		if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
		{
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
		}

		return -1;
	}

	int nCurrCount = (uint32)m_pCurrMessage->size() - m_u4CurrSize;
	//这里需要对m_u4CurrSize进行检查。
	if(nCurrCount < 0)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input][%d] nCurrCount < 0 m_u4CurrSize = %d.\n", GetConnectID(), m_u4CurrSize));
		m_u4CurrSize = 0;

		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			m_pPacketParse->GetMessageHead()->release();
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			m_pPacketParse->GetMessageBody()->release();
		}

		if(m_pCurrMessage != NULL && m_pPacketParse->GetMessageBody() != m_pCurrMessage && m_pPacketParse->GetMessageBody() != m_pCurrMessage)
		{
			m_pCurrMessage->release();
			m_pCurrMessage = NULL;
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);

		//发送客户端链接断开消息。
		if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
		{
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
		}

		return -1;
	}

	int nDataLen = this->peer().recv(m_pCurrMessage->wr_ptr(), nCurrCount, MSG_NOSIGNAL, &nowait);
	if(nDataLen <= 0)
	{
		m_u4CurrSize = 0;
		uint32 u4Error = (uint32)errno;
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_input] ConnectID = %d, recv data is error nDataLen = [%d] errno = [%d].\n", GetConnectID(), nDataLen, u4Error));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::handle_input] ConnectID = %d, recv data is error[%d].\n", GetConnectID(), nDataLen);

		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			m_pPacketParse->GetMessageHead()->release();
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			m_pPacketParse->GetMessageBody()->release();
		}

		if(m_pCurrMessage != NULL && m_pPacketParse->GetMessageBody() != m_pCurrMessage && m_pPacketParse->GetMessageBody() != m_pCurrMessage)
		{
			m_pCurrMessage->release();
			m_pCurrMessage = NULL;
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);

		if(m_u1ConnectState != CONNECT_SERVER_CLOSE)
		{
			//发送客户端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}
		}

		return -1;
	}

	m_u4CurrSize += nDataLen;

	m_pCurrMessage->wr_ptr(nDataLen);

	int na = (int)m_pCurrMessage->length();

	//如果没有读完，短读
	if(m_pCurrMessage->size() > m_u4CurrSize)
	{
		Close();
		return 0;
	}
	else if(m_pCurrMessage->length() == m_pPacketParse->GetPacketHeadLen() && m_pPacketParse->GetIsHead() == false)
	{
		m_pPacketParse->SetPacketHead(m_pCurrMessage->rd_ptr(), (uint32)m_pCurrMessage->length());
		uint32 u4PacketBodyLen = m_pPacketParse->GetPacketDataLen();
		m_u4CurrSize = 0;

		//如果超过了最大包长度，为非法数据
		if(u4PacketBodyLen >= m_u4MaxPacketSize || u4PacketBodyLen <= 0)
		{
			m_u4CurrSize = 0;
			OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_read_stream]u4PacketHeadLen(%d) more than %d.\n", u4PacketBodyLen, m_u4MaxPacketSize));
			if(m_pPacketParse->GetMessageHead() != NULL)
			{
				m_pPacketParse->GetMessageHead()->release();
			}

			if(m_pPacketParse->GetMessageBody() != NULL)
			{
				m_pPacketParse->GetMessageBody()->release();
			}

			if(m_pCurrMessage != NULL && m_pPacketParse->GetMessageBody() != m_pCurrMessage && m_pPacketParse->GetMessageBody() != m_pCurrMessage)
			{
				m_pCurrMessage->release();
				m_pCurrMessage = NULL;
			}
			App_PacketParsePool::instance()->Delete(m_pPacketParse);

			//发送客户端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}

			return -1;
		}
		else
		{
			m_pPacketParse->SetMessageHead(m_pCurrMessage);

			//申请头的大小对应的mb
			m_pCurrMessage = App_MessageBlockManager::instance()->Create(m_pPacketParse->GetPacketDataLen());
			if(m_pCurrMessage == NULL)
			{
				m_u4CurrSize = 0;
				AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
				OUR_DEBUG((LM_ERROR, "[CConnectHandle::RecvClinetPacket] pmb new is NULL.\n"));

				if(m_pPacketParse->GetMessageHead() != NULL)
				{
					m_pPacketParse->GetMessageHead()->release();
				}

				if(m_pPacketParse->GetMessageBody() != NULL)
				{
					m_pPacketParse->GetMessageBody()->release();
				}

				if(m_pCurrMessage != NULL && m_pPacketParse->GetMessageBody() != m_pCurrMessage && m_pPacketParse->GetMessageBody() != m_pCurrMessage)
				{
					m_pCurrMessage->release();
					m_pCurrMessage = NULL;
				}
				App_PacketParsePool::instance()->Delete(m_pPacketParse);

				//发送客户端链接断开消息。
				if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
				{
					OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
				}

				return -1;
			}
			Close();
		}
	}
	else
	{
		//接受完整数据完成，开始分析完整数据包
		m_pPacketParse->SetPacketData(m_pCurrMessage->rd_ptr(), (uint32)m_pCurrMessage->length());
		m_pPacketParse->SetMessageBody(m_pCurrMessage);

		if(false == CheckMessage())
		{
			return -1;
		}

		m_u4CurrSize = 0;

		//申请新的包
		m_pPacketParse = App_PacketParsePool::instance()->Create();
		if(NULL == m_pPacketParse)
		{
			OUR_DEBUG((LM_DEBUG,"[%t|CConnectHandle::open] Open(%d) m_pPacketParse new error.\n", GetConnectID()));
			return -1;
		}

		//申请头的大小对应的mb
		m_pCurrMessage = App_MessageBlockManager::instance()->Create(m_pPacketParse->GetPacketHeadLen());
		if(m_pCurrMessage == NULL)
		{
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
			OUR_DEBUG((LM_ERROR, "[CConnectHandle::RecvClinetPacket] pmb new is NULL.\n"));

			//发送客户端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}

			return -1;
		}

		Close();
	}

	return 0;
}

//关闭链接
int CConnectHandler::handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask)
{

	OUR_DEBUG((LM_DEBUG,"[CConnectHandler::handle_close]Connectid=[%d] begin(%d)...\n",GetConnectID(), errno));
	App_ConnectManager::instance()->Close(GetConnectID());
	OUR_DEBUG((LM_DEBUG,"[CConnectHandler::handle_close] Connectid=[%d] finish ok...\n", GetConnectID()));
	Close(2);

	return 0;
}

bool CConnectHandler::CheckAlive()
{
	ACE_Time_Value tvNow = ACE_OS::gettimeofday();
	ACE_Time_Value tvIntval(tvNow - m_atvInput);
	if(tvIntval.sec() > m_u2MaxConnectTime)
	{
		//如果超过了最大时间，则服务器关闭链接
		OUR_DEBUG ((LM_ERROR, "[CConnectHandle::CheckAlive] Connectid=%d Server Close!\n", GetConnectID()));
		ServerClose();
		return false;
	}
	else
	{
		return true;
	}
}

uint8 CConnectHandler::GetConnectState()
{
	return m_u1ConnectState;
}

uint8 CConnectHandler::GetSendBuffState()
{
	return m_u1SendBuffState;
}

uint8 CConnectHandler::GetIsClosing()
{
	return m_u1IsClosing;
}

bool CConnectHandler::SendMessage(IBuffPacket* pBuffPacket)
{
	m_ThreadLock.acquire();
	m_nIOCount++;
	m_ThreadLock.release();	
	//OUR_DEBUG((LM_DEBUG,"[CConnectHandler::SendMessage]Connectid=%d,m_nIOCount=%d.\n", GetConnectID(), m_nIOCount));

	CPacketParse PacketParse;

	if(NULL == pBuffPacket)
	{
		OUR_DEBUG((LM_DEBUG,"[CConnectHandler::SendMessage] Connectid=[%d] pBuffPacket is NULL.\n", GetConnectID()));
		Close();
		return false;
	}

	ACE_Message_Block* pMbData = NULL;

	int nSendLength = PacketParse.MakePacketLength(pBuffPacket->GetPacketLen());
	pMbData = App_MessageBlockManager::instance()->Create(nSendLength);

	//这里组成返回数据包
	PacketParse.MakePacket(pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), pMbData);

	App_BuffPacketManager::instance()->Delete(pBuffPacket);

	PutSendPacket(pMbData);

	return true;
}

bool CConnectHandler::PutSendPacket(ACE_Message_Block* pMbData)
{
	ACE_Time_Value     nowait(MAX_MSG_PACKETTIMEOUT);

	if(NULL == pMbData)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::SendPacket] ConnectID = %d, get_handle() == ACE_INVALID_HANDLE.\n", GetConnectID()));
		Close();
		return false;
	}

	if(get_handle() == ACE_INVALID_HANDLE)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::SendPacket] ConnectID = %d, get_handle() == ACE_INVALID_HANDLE.\n", GetConnectID()));
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectHandler::SendPacket] ConnectID = %d, get_handle() == ACE_INVALID_HANDLE.\n", GetConnectID());
		pMbData->release();
		Close();
		return false;
	}

	//发送数据
	char* pData = pMbData->rd_ptr();
	if(NULL == pData)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectHandler::SendPacket] ConnectID = %d, pData is NULL.\n", GetConnectID()));
		pMbData->release();
		Close();
		return false;
	}

	int nSendPacketLen = (int)pMbData->length();
	int nIsSendSize    = 0;

	//循环发送，直到数据发送完成。
	while(true)
	{
		int nCurrSendSize = (int)(nSendPacketLen - nIsSendSize);
		if(nCurrSendSize <= 0)
		{
			OUR_DEBUG((LM_ERROR, "[CConnectHandler::SendPacket] ConnectID = %d, nCurrSendSize error is %d.\n", GetConnectID(), nCurrSendSize));
			pMbData->release();
			return false;
		}

		int nDataLen = this->peer().send(pMbData->rd_ptr(), nCurrSendSize, &nowait);
		int nErr = ACE_OS::last_error();
		if(nDataLen <= 0)
		{
			if(nErr == EWOULDBLOCK)
			{
				//如果发送堵塞，则等10毫秒后再发送。
				ACE_Time_Value tvSleep(0, 10 * MAX_BUFF_1000);
				ACE_OS::sleep(tvSleep);
				continue;
			}

			OUR_DEBUG((LM_ERROR, "[CConnectHandler::SendPacket] ConnectID = %d, error = %d.\n", GetConnectID(), errno));
			pMbData->release();
			m_atvOutput      = ACE_OS::gettimeofday();
			App_ConnectManager::instance()->Close(GetConnectID());
			Close();
			return false;
		}
		else if(nDataLen >= nCurrSendSize)   //当数据包全部发送完毕，清空。
		{
			//OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_output] ConnectID = %d, send (%d) OK.\n", GetConnectID(), msg_queue()->is_empty()));
			m_u4AllSendCount += 1;
			m_u4AllSendSize  += (uint32)pMbData->length();
			pMbData->release();
			m_atvOutput      = ACE_OS::gettimeofday();
			Close();
			return true;
		}
		else
		{
			pMbData->rd_ptr(nDataLen);
			nIsSendSize      += nDataLen;
			m_atvOutput      = ACE_OS::gettimeofday();
			continue;
		}
	}

	return true;
}

bool CConnectHandler::CheckMessage()
{
	m_u4AllRecvSize += (uint32)m_pPacketParse->GetMessageHead()->length() + (uint32)m_pPacketParse->GetMessageBody()->length();
	m_u4AllRecvCount++;

	ACE_Date_Time dtNow;
	if(false == m_TimeConnectInfo.Check((uint8)dtNow.minute(), 1, m_u4AllRecvSize))
	{
		//超过了限定的阀值，需要关闭链接，并记录日志
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECTABNORMAL, "[TCP]IP=%s,Prot=%d,PacketCount=%d, RecvSize=%d.", m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_TimeConnectInfo.m_u4PacketCount, m_TimeConnectInfo.m_u4RecvSize);
		App_PacketParsePool::instance()->Delete(m_pPacketParse);
		//设置封禁时间
		App_ForbiddenIP::instance()->AddTempIP(m_addrRemote.get_host_addr(), App_MainConfig::instance()->GetForbiddenTime());
		OUR_DEBUG((LM_ERROR, "[CConnectHandle::CheckMessage] ConnectID = %d, PutMessageBlock is check invalid.\n", GetConnectID()));
		return false;
	}


	//将数据Buff放入消息体中
	if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_PARSE, m_pPacketParse))
	{
		App_PacketParsePool::instance()->Delete(m_pPacketParse);
		OUR_DEBUG((LM_ERROR, "[CConnectHandle::CheckMessage] ConnectID = %d, PutMessageBlock is error.\n", GetConnectID()));
	}

	return true;
}

_ClientConnectInfo CConnectHandler::GetClientInfo()
{
	_ClientConnectInfo ClientConnectInfo;

	ClientConnectInfo.m_blValid       = true;
	ClientConnectInfo.m_u4ConnectID   = GetConnectID();
	ClientConnectInfo.m_addrRemote    = m_addrRemote;
	ClientConnectInfo.m_u4RecvCount   = m_u4AllRecvCount;
	ClientConnectInfo.m_u4SendCount   = m_u4AllSendCount;
	ClientConnectInfo.m_u4AllRecvSize = m_u4AllSendSize;
	ClientConnectInfo.m_u4AllSendSize = m_u4AllSendSize;
	ClientConnectInfo.m_u4BeginTime   = (uint32)m_atvConnect.sec();
	ClientConnectInfo.m_u4AliveTime   = (uint32)(ACE_OS::gettimeofday().sec() -  m_atvConnect.sec());

	return ClientConnectInfo;
}

_ClientIPInfo  CConnectHandler::GetClientIPInfo()
{
	_ClientIPInfo ClientIPInfo;
	sprintf_safe(ClientIPInfo.m_szClientIP, MAX_BUFF_20, "%s", m_addrRemote.get_host_addr());
	ClientIPInfo.m_nPort = (int)m_addrRemote.get_port_number();
	return ClientIPInfo;
}

//***************************************************************************
CConnectManager::CConnectManager(void)
{
	m_u4TimeCheckID      = 0;
	m_u4ConnectCurrID    = 0;
	m_szError[0]         = '\0';

	m_pTCTimeSendCheck   = NULL;
	m_tvCheckConnect     = ACE_OS::gettimeofday();
	m_blRun              = false;
}

CConnectManager::~CConnectManager(void)
{
	m_blRun = false;
	CloseAll();
}

void CConnectManager::CloseAll()
{
	KillTimer();
	mapConnectManager::iterator b = m_mapConnectManager.begin();
	mapConnectManager::iterator e = m_mapConnectManager.end();

	for(b; b != e;)
	{
		mapConnectManager::iterator itr = b;
		CConnectHandler* pConnectHandler = (CConnectHandler* )itr->second;
		if(pConnectHandler != NULL)
		{
			if(true == pConnectHandler->Close())
			{
				itr++;
				b = itr;
			}
			else
			{
				b++;
			}
		}
		else
		{
			b++;
		}

	}

	m_mapConnectManager.clear();
}

bool CConnectManager::Close(uint32 u4ConnectID)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CConnectHandler* pConnectHandler = (CConnectHandler* )f->second;
		if(pConnectHandler != NULL)
		{
			m_mapConnectManager.erase(f);
		}

		return true;
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::Close] ConnectID[%d] is not find.", u4ConnectID);
		return true;
	}
}

bool CConnectManager::CloseConnect(uint32 u4ConnectID)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CConnectHandler* pConnectHandler = (CConnectHandler* )f->second;
		if(pConnectHandler != NULL)
		{
			pConnectHandler->ServerClose();
		}
		m_mapConnectManager.erase(f);
		return true;
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::CloseConnect] ConnectID[%d] is not find.", u4ConnectID);
		return true;
	}
}

bool CConnectManager::AddConnect(CConnectHandler* pConnectHandler)
{
	if(pConnectHandler == NULL)
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::AddConnect] pConnectHandler is NULL.");
		return false;		
	}
	
	mapConnectManager::iterator f = m_mapConnectManager.find(m_u4ConnectCurrID);
	if(f != m_mapConnectManager.end())
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::AddConnect] ConnectID[%d] is exist.", m_u4ConnectCurrID);
		return false;
	}

	pConnectHandler->SetConnectID(m_u4ConnectCurrID);
	//加入map
	m_mapConnectManager.insert(mapConnectManager::value_type(m_u4ConnectCurrID, pConnectHandler));
	m_u4ConnectCurrID++;

	return true;
}

bool CConnectManager::SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket)
{
	if(NULL == pBuffPacket)
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::SendMessage] ConnectID[%d] pBuffPacket is NULL.", u4ConnectID);
		return false;
	}

	m_ThreadWriteLock.acquire();
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CConnectHandler* pConnectHandler = (CConnectHandler* )f->second;

		if(NULL != pConnectHandler)
		{
			pConnectHandler->SendMessage(pBuffPacket);
			m_ThreadWriteLock.release();
			return true;
		}
		else
		{
			sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::SendMessage] ConnectID[%d] is not find.", u4ConnectID);
			App_BuffPacketManager::instance()->Delete(pBuffPacket);
			m_ThreadWriteLock.release();
			return true;
		}
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CConnectManager::SendMessage] ConnectID[%d] is not find.", u4ConnectID);
		App_BuffPacketManager::instance()->Delete(pBuffPacket);
		m_ThreadWriteLock.release();
		return true;
	}

	return true;
}

bool CConnectManager::PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket)
{
	if(NULL == pBuffPacket)
	{
		OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] pBuffPacket is NULL.\n"));
		return false;
	}

	ACE_Message_Block* mb = NULL;

	ACE_NEW_MALLOC_NORETURN(mb, 
							static_cast<ACE_Message_Block*>(_msg_send_mb_allocator.malloc(sizeof(ACE_Message_Block))),
							ACE_Message_Block(sizeof(CMessage*), // size
							ACE_Message_Block::MB_DATA, // type
							0,
							0,
							&_msg_send_mb_allocator, // allocator_strategy
							0, // locking strategy
							ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
							ACE_Time_Value::zero,
							ACE_Time_Value::max_time,
							&_msg_send_mb_allocator,
							&_msg_send_mb_allocator
		));

	if(NULL != mb)
	{
		//放入发送队列
		_SendMessgae* pSendMessgae = new _SendMessgae();

		if(NULL == pSendMessgae)
		{
			OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] new _SendMessgae is error.\n"));
			return false;
		}

		IBuffPacket* pSendBuff = App_BuffPacketManager::instance()->Create();
		if(NULL == pSendBuff)
		{
			OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] new pSendBuff is error.\n"));
			return false;
		}

		pSendBuff->WriteStream(pBuffPacket->GetData(), pBuffPacket->GetPacketLen());

		pSendMessgae->m_u4ConnectID = u4ConnectID;
		pSendMessgae->m_pBuffPacket = pSendBuff;

		_SendMessgae** ppSendMessage = (_SendMessgae **)mb->base();
		*ppSendMessage = pSendMessgae;

		//判断队列是否是已经最大
		int nQueueCount = (int)msg_queue()->message_count();
		if(nQueueCount >= (int)MAX_MSG_THREADQUEUE)
		{
			OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));
			mb->release();
			return false;
		}

		ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(this->putq(mb, &xtime) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
			mb->release();
			return false;
		}
	}
	else
	{
		OUR_DEBUG((LM_ERROR,"[CMessageService::PutMessage] mb new error.\n"));
		return false;
	}

	return true;
}

const char* CConnectManager::GetError()
{
	return m_szError;
}

bool CConnectManager::StartTimer()
{
	//启动发送线程
	if(0 != open())
	{
		OUR_DEBUG((LM_ERROR, "[CConnectManager::StartTimer]Open() is error.\n"));
		return false;
	}

	//避免定时器重复启动
	KillTimer();
	OUR_DEBUG((LM_ERROR, "CConnectManager::StartTimer()-->begin....\n"));
	//得到第二个Reactor
	ACE_Reactor* pReactor = App_ReactorManager::instance()->GetAce_Reactor(REACTOR_POSTDEFINE);
	if(NULL == pReactor)
	{
		OUR_DEBUG((LM_ERROR, "CConnectManager::StartTimer() -->GetAce_Reactor(REACTOR_POSTDEFINE) is NULL.\n"));
		return false;
	}

	m_pTCTimeSendCheck = new _TimerCheckID();
	if(NULL == m_pTCTimeSendCheck)
	{
		OUR_DEBUG((LM_ERROR, "CConnectManager::StartTimer() m_pTCTimeSendCheck is NULL.\n"));
		return false;
	}

	m_pTCTimeSendCheck->m_u2TimerCheckID = PARM_CONNECTHANDLE_CHECK;
	m_u4TimeCheckID = pReactor->schedule_timer(this, (const void *)m_pTCTimeSendCheck, ACE_Time_Value(App_MainConfig::instance()->GetSendAliveTime(), 0), ACE_Time_Value(App_MainConfig::instance()->GetSendAliveTime(), 0));
	if(-1 == m_u4TimeCheckID)
	{
		OUR_DEBUG((LM_ERROR, "CConnectManager::StartTimer()--> Start thread m_u4TimeCheckID error.\n"));
		return false;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "CConnectManager::StartTimer()--> Start thread time OK.\n"));
		return true;
	}
}

bool CConnectManager::KillTimer()
{
	if(m_u4TimeCheckID > 0)
	{
		App_ReactorManager::instance()->GetAce_Reactor(REACTOR_POSTDEFINE)->cancel_timer(m_u4TimeCheckID);
		m_u4TimeCheckID = 0;
	}

	SAFE_DELETE(m_pTCTimeSendCheck);
	return true;
}

int CConnectManager::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	_TimerCheckID* pTimerCheckID = (_TimerCheckID*)arg;
	if(NULL == pTimerCheckID)
	{
		return 0;
	}

	vecConnectManager vecConnect;

	//定时检测发送，这里将定时记录链接信息放入其中，减少一个定时器
	if(pTimerCheckID->m_u2TimerCheckID == PARM_CONNECTHANDLE_CHECK)
	{
		if(m_mapConnectManager.size() == 0)
		{
		}
		else
		{
			mapConnectManager::iterator b = m_mapConnectManager.begin();
			mapConnectManager::iterator e = m_mapConnectManager.end();

			for(b; b != e;)
			{
				CConnectHandler* pConnectHandler = (CConnectHandler* )b->second;
				if(pConnectHandler != NULL)
				{
					if(false == pConnectHandler->CheckAlive())
					{
						m_mapConnectManager.erase(b++);
					}
					else
					{
						b++;
					}
				}
				else
				{
					b++;
				}
			}
		}

		//判定是否应该记录链接日志
		ACE_Time_Value tvNow = ACE_OS::gettimeofday();
		ACE_Time_Value tvInterval(tvNow - m_tvCheckConnect);
		if(tvInterval.sec() >= MAX_MSG_HANDLETIME)
		{
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "[CConnectManager]CurrConnectCount = %d.", GetCount());
			m_tvCheckConnect = tvNow;
		}

		return 0;

	}

	return 0;
}

int CConnectManager::GetCount()
{
	return (int)m_mapConnectManager.size(); 
}

int CConnectManager::open(void* args)
{
	m_blRun = true;
	msg_queue()->high_water_mark(MAX_MSG_MASK);
	msg_queue()->low_water_mark(MAX_MSG_MASK);

	OUR_DEBUG((LM_INFO,"[CConnectManager::open] m_u4HighMask = [%d] m_u4LowMask = [%d]\n", MAX_MSG_MASK, MAX_MSG_MASK));
	if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, MAX_MSG_THREADCOUNT) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CConnectManager::open] activate error ThreadCount = [%d].", MAX_MSG_THREADCOUNT));
		m_blRun = false;
		return -1;
	}

	resume();

	return 0;
}

int CConnectManager::svc (void)
{
	ACE_Message_Block* mb = NULL;
	ACE_Time_Value xtime;

	while(IsRun())
	{
		mb = NULL;
		xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(getq(mb, &xtime) == -1)
		{
			//OUR_DEBUG((LM_ERROR,"[CConnectManager::PutMessage] getq error errno = [%d].\n", errno));
			continue;
		}
		if (mb == NULL)
		{
			continue;
		}
		_SendMessgae* msg = *((_SendMessgae**)mb->base());
		if (! msg)
		{
			mb->release();
			continue;
		}

		//处理发送数据
		SendMessage(msg->m_u4ConnectID, msg->m_pBuffPacket);
		SAFE_DELETE(msg);

		mb->release();
	}

	OUR_DEBUG((LM_INFO,"[CConnectManager::svc] svc finish!\n"));
	return 0;
}

bool CConnectManager::IsRun()
{
	return m_blRun;
}

int CConnectManager::close(u_long)
{
	m_blRun = false;
	OUR_DEBUG((LM_INFO,"[CConnectManager::close] close().\n"));
	return 0;
}

void CConnectManager::GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo)
{
	VecClientConnectInfo.clear();

	mapConnectManager::iterator b = m_mapConnectManager.begin();
	mapConnectManager::iterator e = m_mapConnectManager.end();

	for(b; b != e; b++)
	{
		CConnectHandler* pConnectHandler = (CConnectHandler* )b->second;
		if(pConnectHandler != NULL)
		{
			VecClientConnectInfo.push_back(pConnectHandler->GetClientInfo());
		}
	}
}

_ClientIPInfo CConnectManager::GetClientIPInfo(uint32 u4ConnectID)
{
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CConnectHandler* pConnectHandler = (CConnectHandler* )f->second;
		if(NULL != pConnectHandler)
		{
			return pConnectHandler->GetClientIPInfo();
		}
		else
		{
			_ClientIPInfo ClientIPInfo;
			return ClientIPInfo;
		}
	}
	else
	{
		_ClientIPInfo ClientIPInfo;
		return ClientIPInfo;
	}
}

//*********************************************************************************

CConnectHandlerPool::CConnectHandlerPool(void)
{
	m_u4CurrMaxCount = 0;
}

CConnectHandlerPool::~CConnectHandlerPool(void)
{
	Close();
}

void CConnectHandlerPool::Init(int nObjcetCount)
{
	Close();

	for(int i = 0; i < nObjcetCount; i++)
	{
		CConnectHandler* pPacket = new CConnectHandler();
		if(NULL != pPacket)
		{
			//添加到Free map里面
			mapHandle::iterator f = m_mapMessageFree.find(pPacket);
			if(f == m_mapMessageFree.end())
			{
				pPacket->Init(m_u4CurrMaxCount);
				m_mapMessageFree.insert(mapHandle::value_type(pPacket, pPacket));
				m_u4CurrMaxCount++;
			}
		}
	}
}

void CConnectHandlerPool::Close()
{
	//清理所有已存在的指针
	mapHandle::iterator itorFreeB = m_mapMessageFree.begin();
	mapHandle::iterator itorFreeE = m_mapMessageFree.end();

	for(itorFreeB; itorFreeB != itorFreeE; itorFreeB++)
	{
		CConnectHandler* pObject = (CConnectHandler* )itorFreeB->second;
		SAFE_DELETE(pObject);
	}

	mapHandle::iterator itorUsedB = m_mapMessageUsed.begin();
	mapHandle::iterator itorUsedE = m_mapMessageUsed.end();

	for(itorUsedB; itorUsedB != itorUsedE; itorUsedB++)
	{
		CConnectHandler* pPacket = (CConnectHandler* )itorUsedB->second;
		SAFE_DELETE(pPacket);
	}

	m_u4CurrMaxCount = 0;
	m_mapMessageFree.clear();
	m_mapMessageUsed.clear();
}

int CConnectHandlerPool::GetUsedCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapMessageUsed.size();
}

int CConnectHandlerPool::GetFreeCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapMessageFree.size();
}

CConnectHandler* CConnectHandlerPool::Create()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	//如果free池中已经没有了，则添加到free池中。
	if(m_mapMessageFree.size() <= 0)
	{
		CConnectHandler* pPacket = new CConnectHandler();

		if(pPacket != NULL)
		{
			//添加到Free map里面
			mapHandle::iterator f = m_mapMessageFree.find(pPacket);
			if(f == m_mapMessageFree.end())
			{
				pPacket->Init(m_u4CurrMaxCount);
				m_mapMessageFree.insert(mapHandle::value_type(pPacket, pPacket));
				m_u4CurrMaxCount++;
			}
		}
		else
		{
			return NULL;
		}
	}

	//从free池中拿出一个,放入到used池中
	mapHandle::iterator itorFreeB = m_mapMessageFree.begin();
	CConnectHandler* pPacket = (CConnectHandler* )itorFreeB->second;
	m_mapMessageFree.erase(itorFreeB);
	//添加到used map里面
	mapHandle::iterator f = m_mapMessageUsed.find(pPacket);
	if(f == m_mapMessageUsed.end())
	{
		m_mapMessageUsed.insert(mapHandle::value_type(pPacket, pPacket));
	}

	return (CConnectHandler* )pPacket;
}

bool CConnectHandlerPool::Delete(CConnectHandler* pObject)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	if(NULL == pObject)
	{
		return false;
	}

	mapHandle::iterator f = m_mapMessageUsed.find(pObject);
	if(f != m_mapMessageUsed.end())
	{
		m_mapMessageUsed.erase(f);

		//添加到Free map里面
		mapHandle::iterator f = m_mapMessageFree.find(pObject);
		if(f == m_mapMessageFree.end())
		{
			m_mapMessageFree.insert(mapHandle::value_type(pObject, pObject));
		}
	}

	return true;
}


