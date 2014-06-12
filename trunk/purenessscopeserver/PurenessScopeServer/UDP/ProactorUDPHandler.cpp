#include "ProactorUDPHandler.h"

CProactorUDPHandler::CProactorUDPHandler(void)
{
	m_pPacketParse      = NULL;
	m_u4RecvPacketCount = 0;
	m_u4SendPacketCount = 0;
	m_u4RecvSize        = 0;
	m_u4SendSize        = 0;
}

CProactorUDPHandler::~CProactorUDPHandler(void)
{
}

int CProactorUDPHandler::OpenAddress(const ACE_INET_Addr& AddrLocal, ACE_Proactor* pProactor)
{
	if(m_skRemote.open(AddrLocal) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::OpenAddress]Open error(%d).\n", errno));
		return -1;
	}

	sprintf_safe(m_szCompletionkey, MAX_BUFF_20, "CompUDP");
	sprintf_safe(m_szAct, MAX_BUFF_20, "ActUDP");


	//���÷��ͳ�ʱʱ�䣨��ΪUDP����ͻ��˲����ڵĻ���sendto������һ��recv����
	//����������һ����ʱ���ø�recv�������޵���ȥ
	struct timeval timeout = {MAX_RECV_UDP_TIMEOUT, 0}; 
	ACE_OS::setsockopt(m_skRemote.get_handle(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

	//Linux���޴˶�Ӧ�꣬Ϊ�˿���ͨ�ã��ⲿ��ֻ��windows������
#ifdef WIN32
	//����wsaIoctl
	bool blBehavior = false;
	unsigned long lRet = 0;
	int nStatus = ACE_OS::ioctl(m_skRemote.get_handle(), SIO_UDP_CONNRESET, &blBehavior, sizeof(blBehavior), NULL, 0, &lRet, NULL, NULL);
	if(0 != nStatus)
	{
		OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::OpenAddress]ioctl SIO_UDP_CONNRESET error.\n"));
	}
#endif

	if(m_Read.open(*this, m_skRemote.get_handle(), m_szCompletionkey, pProactor) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::OpenAddress]m_Read error.\n"));
		return -1;
	}

	if(m_Write.open(*this, m_skRemote.get_handle(), m_szCompletionkey, pProactor) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::OpenAddress]m_Write error.\n"));
		return -1;
	}

	ACE_Message_Block* pMBBuff = App_MessageBlockManager::instance()->Create(MAX_UDP_PACKET_LEN);
	if(NULL == pMBBuff)
	{
		OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::OpenAddress]pMBBuff is NULL.\n"));
		return -1;
	}

	//��ʼ�������
	m_TimeConnectInfo.Init(App_MainConfig::instance()->GetClientDataAlert()->m_u4RecvPacketCount, 
		App_MainConfig::instance()->GetClientDataAlert()->m_u4RecvDataMax, 
		App_MainConfig::instance()->GetClientDataAlert()->m_u4SendPacketCount,
		App_MainConfig::instance()->GetClientDataAlert()->m_u4SendDataMax);

	m_pPacketParse = App_PacketParsePool::instance()->Create();

	size_t stRecvLen = MAX_UDP_PACKET_LEN;
	//OUR_DEBUG((LM_INFO, "[CProactorUDPHandler::OpenAddress]pMBBuff=0x%08x.\n", pMBBuff));
	int nRecvSize = m_Read.recv(pMBBuff, stRecvLen, 0, PF_INET, m_szAct);   
	return nRecvSize;   
}  

void CProactorUDPHandler::Close()
{
	App_PacketParsePool::instance()->Delete(m_pPacketParse);
	m_Read.cancel();
	m_Write.cancel();
	m_skRemote.close();
}

void CProactorUDPHandler::handle_read_dgram(const ACE_Asynch_Read_Dgram::Result& result)
{
	//��ΪUDP��һ���Է������������ݰ������Բ�����ճ������
	ACE_Message_Block* pMBBuff = NULL;

	ACE_Message_Block* pMb = result.message_block();
	int nTran = (int)result.bytes_transferred();

	result.remote_address(m_addrRemote);

	if(nTran != 0)
	{
		//��������
		CheckMessage(pMb, (uint32)nTran);
		m_pPacketParse = App_PacketParsePool::instance()->Create();
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CProactorUDPHandler::handle_read_dgram]result.bytes_transferred() is 0.\n"));
	}

	pMb->release();
	pMBBuff = App_MessageBlockManager::instance()->Create(MAX_UDP_PACKET_LEN);

	if(NULL == pMBBuff)
	{
		OUR_DEBUG((LM_INFO, "[CProactorUDPHandler::handle_read_dgram]pMBBuff is NULL.\n"));
		return;
	}
	else
	{
		size_t stRecvLen = MAX_UDP_PACKET_LEN;
		//OUR_DEBUG((LM_INFO, "[CProactorUDPHandler::handle_read_dgram]pMBBuff=0x%08x.\n", pMBBuff));
		m_Read.recv(pMBBuff, stRecvLen, 0, PF_INET, m_szAct); 
	}
}

bool CProactorUDPHandler::SendMessage(const char* pMessage, uint32 u4Len, const char* szIP, int nPort, bool blHead, uint16 u2CommandID)
{
	ACE_Time_Value m_tvBegin = ACE_OS::gettimeofday();

	ACE_INET_Addr AddrRemote;
	int nErr = AddrRemote.set(nPort, szIP);
	if(nErr != 0)
	{
		OUR_DEBUG((LM_INFO, "[CProactorUDPHandler::SendMessage]set_address error[%d].\n", errno));
		SAFE_DELETE(pMessage);
		return false;
	}

	//�����Ҫƴ�Ӱ�ͷ����ƴ�Ӱ�ͷ
	if(blHead == true)
	{
		CPacketParse PacketParse;

		ACE_Message_Block* pMbData = NULL;
		uint32 u4SendLength = PacketParse.MakePacketLength(0, u4Len);
		pMbData = App_MessageBlockManager::instance()->Create(u4SendLength);
		if(NULL == pMbData)
		{
			SAFE_DELETE(pMessage);
			return false;
		}

		PacketParse.MakePacket(0, pMessage, u4Len, pMbData);

		uint32 u4DataLen = (uint32)pMbData->length();
		int nSize = (int)m_skRemote.send(pMbData->rd_ptr(), u4DataLen, AddrRemote);
		if((uint32)nSize == u4DataLen)
		{
			m_atvOutput = ACE_OS::gettimeofday();
			m_u4SendSize += u4Len;
			m_u4SendPacketCount++;
			SAFE_DELETE_ARRAY(pMessage);

			//ͳ�Ʒ�����Ϣ
			ACE_Time_Value tvInterval = ACE_OS::gettimeofday() - m_tvBegin;
			uint32 u4SendCost = (uint32)(tvInterval.msec());
			App_CommandAccount::instance()->SaveCommandData(u2CommandID, u4SendCost, PACKET_UDP, u4DataLen, u4Len, COMMAND_TYPE_OUT);

			//�ͷŷ�����
			pMbData->release();

			return true;
		}
		else
		{
			OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::SendMessage]send error(%d).\n", errno));
			SAFE_DELETE_ARRAY(pMessage);

			//�ͷŷ�����
			pMbData->release();

			return false;
		}


	}
	else
	{
		int nSize = (int)m_skRemote.send(pMessage, u4Len, AddrRemote);
		if((uint32)nSize == u4Len)
		{
			m_atvOutput = ACE_OS::gettimeofday();
			m_u4SendSize += u4Len;
			m_u4SendPacketCount++;
			SAFE_DELETE_ARRAY(pMessage);

			//ͳ�Ʒ�����Ϣ
			ACE_Time_Value tvInterval = ACE_OS::gettimeofday() - m_tvBegin;
			uint32 u4SendCost = (uint32)(tvInterval.msec());
			App_CommandAccount::instance()->SaveCommandData(u2CommandID, u4SendCost, PACKET_UDP, u4Len, u4Len, COMMAND_TYPE_OUT);

			return true;
		}
		else
		{
			OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::SendMessage]send error(%d).\n", errno));
			SAFE_DELETE_ARRAY(pMessage);
			return false;
		}
	}
}

_ClientConnectInfo CProactorUDPHandler::GetClientConnectInfo()
{
	_ClientConnectInfo ClientConnectInfo;
	ClientConnectInfo.m_blValid       = true;
	ClientConnectInfo.m_u4ConnectID   = 0;
	ClientConnectInfo.m_u4AliveTime   = 0;
	ClientConnectInfo.m_u4BeginTime   = (uint32)m_atvInput.sec();
	ClientConnectInfo.m_u4AllRecvSize = m_u4RecvSize;
	ClientConnectInfo.m_u4AllSendSize = m_u4SendSize;
	ClientConnectInfo.m_u4RecvCount   = m_u4RecvPacketCount;
	ClientConnectInfo.m_u4SendCount   = m_u4SendPacketCount;
	return ClientConnectInfo;
}

bool CProactorUDPHandler::CheckMessage(ACE_Message_Block* pMbData, uint32 u4Len)
{
	if(NULL == m_pPacketParse || NULL == pMbData)
	{
		return false;
	}

	if(m_pPacketParse->GetPacketMode() == PACKET_WITHHEAD)
	{
		if(u4Len <= m_pPacketParse->GetPacketHeadLen())
		{
			return false;
		}

		//�����������ݰ�ת��ΪPacketParse����
		ACE_Message_Block* pMBHead = App_MessageBlockManager::instance()->Create(m_pPacketParse->GetPacketHeadLen());
		ACE_OS::memcpy(pMBHead->wr_ptr(), (const void*)pMbData->rd_ptr(), m_pPacketParse->GetPacketHeadLen());
		pMBHead->wr_ptr(m_pPacketParse->GetPacketHeadLen());

		m_pPacketParse->SetPacketHead(0, pMBHead, App_MessageBlockManager::instance());
		if(u4Len != m_pPacketParse->GetPacketHeadLen() + m_pPacketParse->GetPacketBodyLen())
		{
			return false;
		}

		pMbData->rd_ptr(m_pPacketParse->GetPacketHeadLen());


		ACE_Message_Block* pMBBody = App_MessageBlockManager::instance()->Create(m_pPacketParse->GetPacketBodyLen());
		ACE_OS::memcpy(pMBBody->wr_ptr(), (const void*)pMbData->rd_ptr(), m_pPacketParse->GetPacketBodyLen());
		pMBBody->wr_ptr(m_pPacketParse->GetPacketBodyLen());
		m_pPacketParse->SetPacketBody(0, pMBBody, App_MessageBlockManager::instance());

		//UDP��Ϊ�����������ӵ�
		if(false == App_MakePacket::instance()->PutUDPMessageBlock(m_addrRemote, PACKET_PARSE, m_pPacketParse))
		{
			OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::SendMessage]PutMessageBlock is error.\n"));
			App_PacketParsePool::instance()->Delete(m_pPacketParse);
			return false;
		}
	}
	else
	{
		//������������
		if(PACKET_GET_ENOUGTH == m_pPacketParse->GetPacketStream(0, pMbData, App_MessageBlockManager::instance()))
		{
			//UDP��Ϊ�����������ӵ�
			if(false == App_MakePacket::instance()->PutUDPMessageBlock(m_addrRemote, PACKET_PARSE, m_pPacketParse))
			{
				App_PacketParsePool::instance()->Delete(m_pPacketParse);
				OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::SendMessage]PutMessageBlock is error.\n"));
				return false;
			}
		}
		else
		{
			OUR_DEBUG((LM_ERROR, "[CProactorUDPHandler::SendMessage]m_pPacketParse GetPacketStream is error.\n"));
			App_PacketParsePool::instance()->Delete(m_pPacketParse);
			return false;
		}
	}

	m_atvInput = ACE_OS::gettimeofday();
	m_u4RecvSize += u4Len;
	m_u4RecvPacketCount++;

	return true;
}

