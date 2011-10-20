#include "ProConnectHandle.h"

Mutex_Allocator _msg_prosend_mb_allocator;

CProConnectHandle::CProConnectHandle(void)
{
	m_szError[0]       = '\0';
	m_u4ConnectID      = 0;
	m_u4AllRecvCount   = 0;
	m_u4AllSendCount   = 0;
	m_u4AllRecvSize    = 0;
	m_u4AllSendSize    = 0;
	m_nIOCount         = 0;
	m_u4HandlerID      = 0;
	m_u2MaxConnectTime = 0;
	m_u4SendThresHold  = MAX_MSG_SNEDTHRESHOLD;
	m_u2SendQueueMax   = MAX_MSG_SENDPACKET;
	m_u1ConnectState   = CONNECT_INIT;
	m_u1SendBuffState  = CONNECT_SENDNON;
	m_pPacketParse     = NULL;
	m_u4MaxPacketSize  = MAX_MSG_PACKETLENGTH;
}

CProConnectHandle::~CProConnectHandle(void)
{
}

void CProConnectHandle::Init(uint16 u2HandlerID)
{
	m_u4HandlerID      = u2HandlerID;
	m_u2MaxConnectTime = App_MainConfig::instance()->GetMaxConnectTime();
	m_u4SendThresHold  = App_MainConfig::instance()->GetSendThresHold();
	m_u2SendQueueMax   = App_MainConfig::instance()->GetSendQueueMax();
	m_u4MaxPacketSize  = App_MainConfig::instance()->GetRecvBuffSize();
}

const char* CProConnectHandle::GetError()
{
	return m_szError;
}

bool CProConnectHandle::Close(int nIOCount)
{
	m_ThreadWriteLock.acquire();
	if(nIOCount > m_nIOCount)
	{
		m_nIOCount = 0;
	}

	if(m_nIOCount > 0)
	{
		m_nIOCount -= nIOCount;
	}
	m_ThreadWriteLock.release();

	//OUR_DEBUG((LM_DEBUG, "[CProConnectHandle::Close]ConnectID=%d, m_nIOCount = %d.\n", GetConnectID(), m_nIOCount));

	if(m_nIOCount == 0)
	{
		m_ThreadWriteLock.acquire();

		m_Reader.cancel();
		m_Writer.cancel();

		if(this->handle() != ACE_INVALID_HANDLE)
		{
			ACE_OS::closesocket(this->handle());
			this->handle(ACE_INVALID_HANDLE);
		}

		m_ThreadWriteLock.release();

		OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::Close] Close(%d) delete OK.\n", GetConnectID()));

		//删除存在列表中的对象引用
		App_ProConnectManager::instance()->Close(GetConnectID());

		//将对象指针放入空池中
		App_ProConnectHandlerPool::instance()->Delete(this);

		return true;
	}

	return false;
}

bool CProConnectHandle::ServerClose()
{
	OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::ServerClose] Close(%d) delete OK.\n", GetConnectID()));

	//发送服务器端链接断开消息。
	if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_SDISCONNECT, NULL))
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
		return false;
	}

	m_Reader.cancel();
	m_Writer.cancel();

	if(this->handle() != ACE_INVALID_HANDLE)
	{
		ACE_OS::closesocket(this->handle());
		this->handle(ACE_INVALID_HANDLE);
	}

	m_u1ConnectState = CONNECT_SERVER_CLOSE;

	return true;
}

void CProConnectHandle::SetConnectID(uint32 u4ConnectID)
{
	m_u4ConnectID = u4ConnectID;
}

uint32 CProConnectHandle::GetConnectID()
{
	return m_u4ConnectID;
}

void CProConnectHandle::addresses (const ACE_INET_Addr &remote_address, const ACE_INET_Addr &local_address)
{
	m_addrRemote = remote_address;
}

void CProConnectHandle::open(ACE_HANDLE h, ACE_Message_Block&)
{
	ACE_Time_Value tvOpenBegin(ACE_OS::gettimeofday());

	OUR_DEBUG((LM_INFO, "[CProConnectHandle::open] [0x%08x]Connection from [%s:%d]\n", this, m_addrRemote.get_host_addr(), m_addrRemote.get_port_number()));

	m_atvConnect      = ACE_OS::gettimeofday();
	m_atvInput        = ACE_OS::gettimeofday();
	m_atvOutput       = ACE_OS::gettimeofday();
	m_atvSendAlive    = ACE_OS::gettimeofday();

	m_u4AllRecvCount    = 0;
	m_u4AllSendCount    = 0;
	m_u4AllRecvSize     = 0;
	m_u4AllSendSize     = 0;
	m_blCanWrite        = true;
	m_blTimeClose       = false;
	m_u4RecvPacketCount = 0;
	m_nIOCount          = 1;

	ACE_Time_Value tvOpenEnd(ACE_OS::gettimeofday());
	ACE_Time_Value tvOpen(tvOpenEnd - tvOpenBegin);

	//检查单位时间链接次数是否达到上限
	if(false == App_IPAccount::instance()->AddIP((string)m_addrRemote.get_host_addr()))
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open]IP connect frequently.\n", m_addrRemote.get_host_addr()));
		App_ForbiddenIP::instance()->AddTempIP(m_addrRemote.get_host_addr(), App_MainConfig::instance()->GetForbiddenTime());
		Close();
		return;
	}

	//初始化检查器
	m_TimeConnectInfo.Init(App_MainConfig::instance()->GetValid(), App_MainConfig::instance()->GetValidPacketCount(), App_MainConfig::instance()->GetValidRecvSize());


	AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Connection from [%s:%d] DisposeTime = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), tvOpen.msec());

	this->handle(h);

	if(this->m_Reader.open(*this, h, 0, App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE)) == -1 || 
		this->m_Writer.open(*this, h, 0, App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE)) == -1)
	{
		OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::open] m_reader or m_reader == 0.\n"));	
		Close();
		return;
	}

	//ACE_Sig_Action writeAction((ACE_SignalHandler)SIG_IGN);
	//writeAction.register_action(SIGPIPE, 0);

	//int nTecvBuffSize = MAX_MSG_SOCKETBUFF;
	//ACE_OS::setsockopt(this->get_handle(), SOL_SOCKET, SO_RCVBUF, (char* )&nTecvBuffSize, sizeof(nTecvBuffSize));
	//ACE_OS::setsockopt(h, SOL_SOCKET, SO_SNDBUF, (char* )&nTecvBuffSize, sizeof(nTecvBuffSize));

	//将这个链接放入链接库
	if(false == App_ProConnectManager::instance()->AddConnect(this))
	{
		OUR_DEBUG((LM_ERROR, "%s.\n", App_ProConnectManager::instance()->GetError()));
		sprintf_safe(m_szError, MAX_BUFF_500, "%s", App_ProConnectManager::instance()->GetError());
		Close();
		return;
	}

	m_u1ConnectState = CONNECT_OPEN;

	OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::open] Open(%d).\n", GetConnectID()));	

	m_pPacketParse = App_PacketParsePool::instance()->Create();
	if(NULL == m_pPacketParse)
	{
		OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::open] Open(%d) m_pPacketParse new error.\n", GetConnectID()));
		Close();
		return;
	}

	//发送链接建立消息。
	if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CONNECT, NULL))
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
	}

	RecvClinetPacket(m_pPacketParse->GetPacketHeadLen());

	return;
}

void CProConnectHandle::handle_read_stream(const ACE_Asynch_Read_Stream::Result &result)
{
	ACE_Message_Block& mb = result.message_block();
	uint32 u4PacketLen = (uint32)result.bytes_transferred();
	int nTran = (int)result.bytes_transferred();

	if(!result.success() || result.bytes_transferred() == 0)
	{
		//链接断开
		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageHead());
			m_pPacketParse->SetMessageHead(NULL);
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageBody());
			m_pPacketParse->SetMessageBody(NULL);
		}

		if(&mb != m_pPacketParse->GetMessageHead() && &mb != m_pPacketParse->GetMessageBody())
		{
			//OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::handle_read_stream] Message_block release.\n"));
			App_MessageBlockManager::instance()->Close(&mb);
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);

		OUR_DEBUG((LM_DEBUG,"[%tCConnectHandler::handle_read_stream]Connectid=[%d] error(%d)...\n", GetConnectID(), errno));
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
		//因为是要关闭连接，所以要多关闭一次IO，对应Open设置的1的初始值

		if(m_u1ConnectState != CONNECT_SERVER_CLOSE)
		{
			//发送客户端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}
		}

		Close(2);

		return;
	}

	m_atvInput = ACE_OS::gettimeofday();

	if(result.bytes_transferred() < result.bytes_to_read())
	{
		//短读，继续读
		int nRead = (int)result.bytes_to_read() - (int)result.bytes_transferred();
		if(-1 == m_Reader.read(mb, nRead))
		{
			if(m_pPacketParse->GetMessageHead() != NULL)
			{
				App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageHead());
				m_pPacketParse->SetMessageHead(NULL);
			}

			if(m_pPacketParse->GetMessageBody() != NULL)
			{
				App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageBody());
				m_pPacketParse->SetMessageBody(NULL);
			}

			if(&mb != m_pPacketParse->GetMessageHead() && &mb != m_pPacketParse->GetMessageBody())
			{
				App_MessageBlockManager::instance()->Close(&mb);
			}
			App_PacketParsePool::instance()->Delete(m_pPacketParse);

			OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_read_stream]Read Shoter error(%d).", errno));
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
			//因为是要关闭连接，所以要多关闭一次IO，对应Open设置的1的初始值

			if(m_u1ConnectState != CONNECT_SERVER_CLOSE)
			{
				//发送客户端链接断开消息。
				if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_CDISCONNECT, NULL))
				{
					OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
				}
			}

			Close(2);
			return;
		}

	}
	else if(mb.length() == m_pPacketParse->GetPacketHeadLen() && m_pPacketParse->GetIsHead() == false)
	{
		//判断头的合法性
		m_pPacketParse->SetPacketHead(mb.rd_ptr(), (uint32)mb.length());
		uint32 u4PacketBodyLen = m_pPacketParse->GetPacketDataLen();

		//如果超过了最大包长度，为非法数据
		if(u4PacketBodyLen >= m_u4MaxPacketSize || u4PacketBodyLen <= 0)
		{
			OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_read_stream]u4PacketHeadLen(%d) more than %d.\n", u4PacketBodyLen, m_u4MaxPacketSize));

			if(m_pPacketParse->GetMessageHead() != NULL)
			{
				App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageHead());
				m_pPacketParse->SetMessageHead(NULL);
			}

			if(m_pPacketParse->GetMessageBody() != NULL)
			{
				App_MessageBlockManager::instance()->Close(m_pPacketParse->GetMessageBody());
				m_pPacketParse->SetMessageBody(NULL);
			}

			if(&mb != m_pPacketParse->GetMessageHead() && &mb != m_pPacketParse->GetMessageBody())
			{
				App_MessageBlockManager::instance()->Close(&mb);
			}
			App_PacketParsePool::instance()->Delete(m_pPacketParse);

			//发送服务器端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_SDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}

			Close(2);
			return;
		}
		else
		{
			m_pPacketParse->SetMessageHead(&mb);
			Close();
			RecvClinetPacket(u4PacketBodyLen);
		}
	}
	else
	{
		//接受完整数据完成，开始分析完整数据包
		m_pPacketParse->SetPacketData(mb.rd_ptr(), (uint32)mb.length());
		m_pPacketParse->SetMessageBody(&mb);

		if(false == CheckMessage())
		{
			Close(2);
			return;
		}

		m_pPacketParse = App_PacketParsePool::instance()->Create();
		if(NULL == m_pPacketParse)
		{
			OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::handle_read_stream] Open(%d) m_pPacketParse new error.\n", GetConnectID()));
			//因为是要关闭连接，所以要多关闭一次IO，对应Open设置的1的初始值
			//发送服务器端链接断开消息。
			if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_SDISCONNECT, NULL))
			{
				OUR_DEBUG((LM_ERROR, "[CProConnectHandle::open] ConnectID = %d, PACKET_CONNECT is error.\n", GetConnectID()));
			}

			Close(2);
			return;
		}
		Close();

		//接受下一个数据包
		RecvClinetPacket(m_pPacketParse->GetPacketHeadLen());
	}

	return;
}

void CProConnectHandle::handle_write_stream(const ACE_Asynch_Write_Stream::Result &result)
{
	if(!result.success() || result.bytes_transferred()==0)
	{
		//链接断开
		OUR_DEBUG ((LM_DEBUG,"[CConnectHandler::handle_write_stream] Connectid=[%d] begin(%d)...\n",GetConnectID(), errno));

		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);

		OUR_DEBUG((LM_DEBUG,"[CConnectHandler::handle_write_stream] Connectid=[%d] finish ok...\n", GetConnectID()));
		m_atvOutput = ACE_OS::gettimeofday();
		App_MessageBlockManager::instance()->Close(&result.message_block());
		Close();
		return;
	}
	else
	{
		//短读不考虑
		m_blCanWrite = true;
		m_atvOutput = ACE_OS::gettimeofday();
		App_MessageBlockManager::instance()->Close(&result.message_block());
		m_u4AllSendSize += (uint32)result.bytes_to_write();
		Close();
		return;
	}
}

uint8 CProConnectHandle::GetConnectState()
{
	return m_u1ConnectState;
}

uint8 CProConnectHandle::GetSendBuffState()
{
	return m_u1SendBuffState;
}

bool CProConnectHandle::GetIsClosing()
{
	return m_blTimeClose;
}

bool CProConnectHandle::SendMessage(IBuffPacket* pBuffPacket)
{
	CPacketParse PacketParse;

	if(NULL == pBuffPacket)
	{
		//OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::SendMessage] Connectid=[%d] pBuffPacket is NULL.\n", GetConnectID()));
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

bool CProConnectHandle::CheckAlive()
{
	ACE_Time_Value tvNow = ACE_OS::gettimeofday();
	ACE_Time_Value tvIntval(tvNow - m_atvInput);
	if(tvIntval.sec() > m_u2MaxConnectTime)
	{
		//如果超过了最大时间，则服务器关闭链接
		OUR_DEBUG ((LM_ERROR, "[CProConnectHandle::CheckAlive] Connectid=%d Server Close!\n", GetConnectID()));
		ServerClose();
		return false;
	}
	else
	{
		return true;
	}
}

bool CProConnectHandle::PutSendPacket(ACE_Message_Block* pMbData)
{
	int nSendSize = m_u4AllSendSize;

	m_ThreadWriteLock.acquire();
	m_nIOCount++;
	m_ThreadWriteLock.release();
	//OUR_DEBUG ((LM_ERROR, "[CConnectHandler::PutSendPacket] Connectid=%d, m_nIOCount=%d!\n", GetConnectID(), m_nIOCount));

	//异步发送方法
	if(NULL != pMbData)
	{
		if(0 != m_Writer.write(*pMbData, pMbData->length()))
		{
			OUR_DEBUG ((LM_ERROR, "[CConnectHandler::PutSendPacket] Connectid=%d mb=%d m_writer.write error(%d)!\n", GetConnectID(),  pMbData->length(), errno));
			pMbData->release();
		    Close();
			return false;
		}
		else
		{
			m_u4AllSendCount += 1;
			m_atvOutput      = ACE_OS::gettimeofday();
			return true;
		}
		//pMbData->release();
	}
	else
	{
		OUR_DEBUG ((LM_ERROR,"[CConnectHandler::PutSendPacket] Connectid=%d mb is NULL!\n", GetConnectID()));
		Close();
		return false;
	}
}

bool CProConnectHandle::RecvClinetPacket(uint32 u4PackeLen)
{
	m_ThreadWriteLock.acquire();
	m_nIOCount++;
	m_ThreadWriteLock.release();
	//OUR_DEBUG((LM_ERROR, "[CProConnectHandle::RecvClinetPacket]Connectid=%d, m_nIOCount=%d.\n", GetConnectID(), m_nIOCount));

	ACE_Message_Block* pmb = NULL;
	pmb = App_MessageBlockManager::instance()->Create(u4PackeLen);
	if(pmb == NULL)
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::RecvClinetPacket] pmb new is NULL.\n"));
		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			m_pPacketParse->GetMessageHead()->release();
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			m_pPacketParse->GetMessageBody()->release();
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);
		Close(2);
		return false;
	}

	if(m_Reader.read(*pmb, u4PackeLen) == -1)
	{
		//如果读失败，则关闭连接。
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "Close Connection from [%s:%d] RecvSize = %d, RecvCount = %d, SendSize = %d, SendCount = %d.",m_addrRemote.get_host_addr(), m_addrRemote.get_port_number(), m_u4AllRecvSize, m_u4AllRecvCount, m_u4AllSendSize, m_u4AllSendCount);
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::RecvClinetPacket] m_reader.read is error(%d)(%d).\n", GetConnectID(), errno));
		pmb->release();
		if(m_pPacketParse->GetMessageHead() != NULL)
		{
			m_pPacketParse->GetMessageHead()->release();
		}

		if(m_pPacketParse->GetMessageBody() != NULL)
		{
			m_pPacketParse->GetMessageBody()->release();
		}
		App_PacketParsePool::instance()->Delete(m_pPacketParse);
		Close(2);
		return false;
	}

	return true;
}

bool CProConnectHandle::CheckMessage()
{
	if(m_pPacketParse->GetMessageHead() != NULL && m_pPacketParse->GetMessageBody() != NULL)
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
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::CheckMessage] ConnectID = %d, PutMessageBlock is check invalid.\n", GetConnectID()));
			return false;
		}

		//将数据Buff放入消息体中，传递给MakePacket处理。
		if(false == App_MakePacket::instance()->PutMessageBlock(GetConnectID(), PACKET_PARSE, m_pPacketParse))
		{
			App_PacketParsePool::instance()->Delete(m_pPacketParse);
			OUR_DEBUG((LM_ERROR, "[CProConnectHandle::CheckMessage] ConnectID = %d, PutMessageBlock is error.\n", GetConnectID()));
		}
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::CheckMessage] ConnectID = %d, m_pPacketParse is NULL.\n", GetConnectID()));
	}

	return true;
}

_ClientConnectInfo CProConnectHandle::GetClientInfo()
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
	ClientConnectInfo.m_u4AliveTime   = (uint32)(ACE_OS::gettimeofday().sec()  -  m_atvConnect.sec());

	return ClientConnectInfo;
}

_ClientIPInfo CProConnectHandle::GetClientIPInfo()
{
	_ClientIPInfo ClientIPInfo;
	sprintf_safe(ClientIPInfo.m_szClientIP, MAX_BUFF_20, "%s", m_addrRemote.get_host_addr());
	ClientIPInfo.m_nPort = (int)m_addrRemote.get_port_number();
	return ClientIPInfo;
}

//***************************************************************************
CProConnectManager::CProConnectManager(void)
{
	m_u4TimeCheckID      = 0;
	m_u4ConnectCurrID    = 0;
	m_szError[0]         = '\0';
	m_blRun              = false;

	m_tvCheckConnect     = ACE_OS::gettimeofday();
}

CProConnectManager::~CProConnectManager(void)
{
	CloseAll();
}

void CProConnectManager::CloseAll()
{
	KillTimer();
	mapConnectManager::iterator b = m_mapConnectManager.begin();
	mapConnectManager::iterator e = m_mapConnectManager.end();

	for(b; b != e; b++)
	{
		CProConnectHandle* pConnectHandler = (CProConnectHandle* )b->second;
		if(pConnectHandler != NULL)
		{
			pConnectHandler->Close();
		}
	}

	m_mapConnectManager.clear();
}

bool CProConnectManager::Close(uint32 u4ConnectID)
{
	//客户端关闭
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGrard(m_ThreadWriteLock);

	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		m_mapConnectManager.erase(f);
		return true;
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::Close] ConnectID[%d] is not find.", u4ConnectID);
		return true;
	}
}

bool CProConnectManager::CloseConnect(uint32 u4ConnectID)
{
	//服务器关闭
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CProConnectHandle* pConnectHandler = (CProConnectHandle* )f->second;
		m_mapConnectManager.erase(f);
		if(pConnectHandler != NULL)
		{
			pConnectHandler->ServerClose();
		}
		return true;
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::CloseConnect] ConnectID[%d] is not find.", u4ConnectID);
		return true;
	}
}

bool CProConnectManager::AddConnect(CProConnectHandle* pConnectHandler)
{
	if(pConnectHandler == NULL)
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::AddConnect] pConnectHandler is NULL.");
		return false;		
	}

	m_ThreadWriteLock.acquire();
	mapConnectManager::iterator f = m_mapConnectManager.find(m_u4ConnectCurrID);
	if(f != m_mapConnectManager.end())
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::AddConnect] ConnectID[%d] is exist.", m_u4ConnectCurrID);
		m_ThreadWriteLock.release();
		return false;
	}

	pConnectHandler->SetConnectID(m_u4ConnectCurrID);
	//加入map
	m_mapConnectManager.insert(mapConnectManager::value_type(m_u4ConnectCurrID, pConnectHandler));
	m_u4ConnectCurrID++;
	m_ThreadWriteLock.release();

	return true;
}

bool CProConnectManager::SendMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket)
{
	//ACE_Guard<ACE_Recursive_Thread_Mutex> WGrard(m_ThreadWriteLock);
	//OUR_DEBUG((LM_ERROR,"[CProConnectManager::SendMessage]BEGIN.\n"));

	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CProConnectHandle* pConnectHandler = (CProConnectHandle* )f->second;

		if(NULL != pConnectHandler)
		{
			pConnectHandler->SendMessage(pBuffPacket);
			return true;
		}
		else
		{
			sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::SendMessage] ConnectID[%d] is not find.", u4ConnectID);
			App_BuffPacketManager::instance()->Delete(pBuffPacket);
			return true;
		}
	}
	else
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "[CProConnectManager::SendMessage] ConnectID[%d] is not find.", u4ConnectID);
		//OUR_DEBUG((LM_ERROR,"[CProConnectManager::SendMessage]%s.\n", m_szError));
		App_BuffPacketManager::instance()->Delete(pBuffPacket);
		return true;
	}

	return true;
}

bool CProConnectManager::PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket)
{
	//OUR_DEBUG((LM_ERROR,"[CProConnectManager::PutMessage]BEGIN.\n"));
	if(NULL == pBuffPacket)
	{
		OUR_DEBUG((LM_ERROR,"[CProConnectManager::PutMessage] pBuffPacket is NULL.\n"));
		return false;
	}

	ACE_Message_Block* mb = NULL;

	ACE_NEW_MALLOC_NORETURN(mb, 
		static_cast<ACE_Message_Block*>(_msg_prosend_mb_allocator.malloc(sizeof(ACE_Message_Block))),
		ACE_Message_Block(sizeof(CMessage*), // size
		ACE_Message_Block::MB_DATA, // type
		0,
		0,
		&_msg_prosend_mb_allocator, // allocator_strategy
		0, // locking strategy
		ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
		ACE_Time_Value::zero,
		ACE_Time_Value::max_time,
		&_msg_prosend_mb_allocator,
		&_msg_prosend_mb_allocator
		));

	if(NULL != mb)
	{
		//放入发送队列
		_SendMessgae* pSendMessgae = new _SendMessgae();

		if(NULL == pSendMessgae)
		{
			OUR_DEBUG((LM_ERROR,"[CProConnectManager::PutMessage] new _SendMessgae is error.\n"));
			return false;
		}

		pSendMessgae->m_u4ConnectID = u4ConnectID;
		pSendMessgae->m_pBuffPacket = pBuffPacket;

		_SendMessgae** ppSendMessage = (_SendMessgae **)mb->base();
		*ppSendMessage = pSendMessgae;

		//判断队列是否是已经最大
		int nQueueCount = (int)msg_queue()->message_count();
		if(nQueueCount >= (int)MAX_MSG_THREADQUEUE)
		{
			OUR_DEBUG((LM_ERROR,"[CProConnectManager::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));
			mb->release();
			return false;
		}

		ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(this->putq(mb, &xtime) == -1)
		{
			OUR_DEBUG((LM_ERROR,"[CProConnectManager::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
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

const char* CProConnectManager::GetError()
{
	return m_szError;
}

bool CProConnectManager::StartTimer()
{
	//启动发送线程
	if(0 != open())
	{
		OUR_DEBUG((LM_ERROR, "[CConnectManager::StartTimer]Open() is error.\n"));
		return false;
	}

	//避免定时器重复启动
	KillTimer();
	OUR_DEBUG((LM_ERROR, "CProConnectManager::StartTimer()-->begin....\n"));

	//检测链接发送存活包数
	uint16 u2SendAlive = App_MainConfig::instance()->GetSendAliveTime();
	m_u4TimeCheckID = App_TimerManager::instance()->schedule(this, (void *)NULL, ACE_OS::gettimeofday() + ACE_Time_Value(u2SendAlive), ACE_Time_Value(u2SendAlive));
	if(-1 == m_u4TimeCheckID)
	{
		OUR_DEBUG((LM_ERROR, "CProConnectManager::StartTimer()--> Start thread m_u4TimeCheckID error.\n"));
		return false;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "CProConnectManager::StartTimer()--> Start thread time OK.\n"));
		return true;
	}
}

bool CProConnectManager::KillTimer()
{
	if(m_u4TimeCheckID > 0)
	{
		App_TimerManager::instance()->cancel(m_u4TimeCheckID);
		m_u4TimeCheckID = 0;
	}

	return true;
}

int CProConnectManager::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	m_ThreadWriteLock.acquire();
	if(m_mapConnectManager.size() != 0)
	{
		mapConnectManager::iterator b = m_mapConnectManager.begin();
		mapConnectManager::iterator e = m_mapConnectManager.end();

		for(b; b != e;)
		{
			CProConnectHandle* pConnectHandler = (CProConnectHandle* )b->second;
			if(pConnectHandler != NULL)
			{
				if(false == pConnectHandler->CheckAlive())
				{
					//删除释放对象
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
	m_ThreadWriteLock.release();

	//判定是否应该记录链接日志
	ACE_Time_Value tvNow = ACE_OS::gettimeofday();
	ACE_Time_Value tvInterval(tvNow - m_tvCheckConnect);
	if(tvInterval.sec() >= MAX_MSG_HANDLETIME)
	{
		AppLogManager::instance()->WriteLog(LOG_SYSTEM_CONNECT, "[CProConnectManager]CurrConnectCount = %d.", GetCount());
		m_tvCheckConnect = tvNow;
	}

	return 0;
}

int CProConnectManager::GetCount()
{
	return (int)m_mapConnectManager.size(); 
}

int CProConnectManager::open(void* args)
{
	m_blRun = true;
	msg_queue()->high_water_mark(MAX_MSG_MASK);
	msg_queue()->low_water_mark(MAX_MSG_MASK);

	OUR_DEBUG((LM_INFO,"[CProConnectManager::open] m_u4HighMask = [%d] m_u4LowMask = [%d]\n", MAX_MSG_MASK, MAX_MSG_MASK));
	if(activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED | THR_SUSPENDED, MAX_MSG_THREADCOUNT) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectManager::open] activate error ThreadCount = [%d].", MAX_MSG_THREADCOUNT));
		m_blRun = false;
		return -1;
	}

	resume();

	return 0;
}

int CProConnectManager::svc (void)
{
	ACE_Message_Block* mb = NULL;
	ACE_Time_Value xtime;

	while(IsRun())
	{
		mb = NULL;
		xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
		if(getq(mb, &xtime) == -1)
		{
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

	OUR_DEBUG((LM_INFO,"[CProConnectManager::svc] svc finish!\n"));
	return 0;
}

bool CProConnectManager::IsRun()
{
	return m_blRun;
}

int CProConnectManager::close(u_long)
{
	m_blRun = false;
	OUR_DEBUG((LM_INFO,"[CProConnectManager::close] close().\n"));
	return 0;
}

void CProConnectManager::GetConnectInfo(vecClientConnectInfo& VecClientConnectInfo)
{
	VecClientConnectInfo.clear();

	mapConnectManager::iterator b = m_mapConnectManager.begin();
	mapConnectManager::iterator e = m_mapConnectManager.end();

	for(b; b != e; b++)
	{
		CProConnectHandle* pConnectHandler = (CProConnectHandle* )b->second;
		if(pConnectHandler != NULL)
		{
			VecClientConnectInfo.push_back(pConnectHandler->GetClientInfo());
		}
	}
}

_ClientIPInfo CProConnectManager::GetClientIPInfo(uint32 u4ConnectID)
{
	mapConnectManager::iterator f = m_mapConnectManager.find(u4ConnectID);

	if(f != m_mapConnectManager.end())
	{
		CProConnectHandle* pConnectHandler = (CProConnectHandle* )f->second;
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

CProConnectHandlerPool::CProConnectHandlerPool(void)
{
	m_u4CurrMaxCount = 0;
}

CProConnectHandlerPool::~CProConnectHandlerPool(void)
{
	Close();
}

void CProConnectHandlerPool::Init(int nObjcetCount)
{
	Close();

	for(int i = 0; i < nObjcetCount; i++)
	{
		CProConnectHandle* pPacket = new CProConnectHandle();
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

void CProConnectHandlerPool::Close()
{
	//清理所有已存在的指针
	mapHandle::iterator itorFreeB = m_mapMessageFree.begin();
	mapHandle::iterator itorFreeE = m_mapMessageFree.end();

	for(itorFreeB; itorFreeB != itorFreeE; itorFreeB++)
	{
		CProConnectHandle* pObject = (CProConnectHandle* )itorFreeB->second;
		SAFE_DELETE(pObject);
	}

	mapHandle::iterator itorUsedB = m_mapMessageUsed.begin();
	mapHandle::iterator itorUsedE = m_mapMessageUsed.end();

	for(itorUsedB; itorUsedB != itorUsedE; itorUsedB++)
	{
		CProConnectHandle* pPacket = (CProConnectHandle* )itorUsedB->second;
		SAFE_DELETE(pPacket);
	}

	m_u4CurrMaxCount = 0;
	m_mapMessageFree.clear();
	m_mapMessageUsed.clear();
}

int CProConnectHandlerPool::GetUsedCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapMessageUsed.size();
}

int CProConnectHandlerPool::GetFreeCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapMessageFree.size();
}

CProConnectHandle* CProConnectHandlerPool::Create()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	//如果free池中已经没有了，则添加到free池中。
	if(m_mapMessageFree.size() <= 0)
	{
		CProConnectHandle* pPacket = new CProConnectHandle();

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
	CProConnectHandle* pPacket = (CProConnectHandle* )itorFreeB->second;
	m_mapMessageFree.erase(itorFreeB);
	//添加到used map里面
	mapHandle::iterator f = m_mapMessageUsed.find(pPacket);
	if(f == m_mapMessageUsed.end())
	{
		m_mapMessageUsed.insert(mapHandle::value_type(pPacket, pPacket));
	}

	return (CProConnectHandle* )pPacket;
}

bool CProConnectHandlerPool::Delete(CProConnectHandle* pObject)
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


