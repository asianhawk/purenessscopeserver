#include "MakePacket.h"


CMakePacketPool::CMakePacketPool()
{
}

CMakePacketPool::~CMakePacketPool()
{
	OUR_DEBUG((LM_INFO, "[CMakePacketPool::~CMakePacketPool].\n"));
	Close();
}

void CMakePacketPool::Init(uint32 u4PacketCount)
{
	Close();

	for(int i = 0; i < (int)u4PacketCount; i++)
	{
		_MakePacket* pPacket = new _MakePacket();
		if(NULL != pPacket)
		{
			//��ӵ�Free map����
			mapPacket::iterator f = m_mapPacketFree.find(pPacket);
			if(f == m_mapPacketFree.end())
			{
				m_mapPacketFree.insert(mapPacket::value_type(pPacket, pPacket));
			}
		}
	}
}

void CMakePacketPool::Close()
{
	//���������Ѵ��ڵ�ָ��
	for(mapPacket::iterator itorFreeB = m_mapPacketFree.begin(); itorFreeB != m_mapPacketFree.end(); itorFreeB++)
	{
		_MakePacket* pPacket = (_MakePacket* )itorFreeB->second;
		SAFE_DELETE(pPacket);
	}

	for(mapPacket::iterator itorUsedB = m_mapPacketUsed.begin(); itorUsedB != m_mapPacketUsed.end(); itorUsedB++)
	{
		_MakePacket* pPacket = (_MakePacket* )itorUsedB->second;
		OUR_DEBUG((LM_ERROR, "[CMakePacketPool::Close]MakePacket has used!!memory address[0x%08x].\n", pPacket));
		SAFE_DELETE(pPacket);
	}

	m_mapPacketFree.clear();
	m_mapPacketUsed.clear();
}

int CMakePacketPool::GetUsedCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapPacketUsed.size();
}

int CMakePacketPool::GetFreeCount()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	return (int)m_mapPacketFree.size();
}

_MakePacket* CMakePacketPool::Create()
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	//���free�����Ѿ�û���ˣ�����ӵ�free���С�
	if(m_mapPacketFree.size() <= 0)
	{
		_MakePacket* pPacket = new _MakePacket();

		if(pPacket != NULL)
		{
			//��ӵ�Free map����
			mapPacket::iterator f = m_mapPacketFree.find(pPacket);
			if(f == m_mapPacketFree.end())
			{
				m_mapPacketFree.insert(mapPacket::value_type(pPacket, pPacket));
			}
		}
		else
		{
			return NULL;
		}
	}

	//��free�����ó�һ��,���뵽used����
	mapPacket::iterator itorFreeB = m_mapPacketFree.begin();
	_MakePacket* pPacket = (_MakePacket* )itorFreeB->second;
	m_mapPacketFree.erase(itorFreeB);
	//��ӵ�used map����
	mapPacket::iterator f = m_mapPacketUsed.find(pPacket);
	if(f == m_mapPacketUsed.end())
	{
		m_mapPacketUsed.insert(mapPacket::value_type(pPacket, pPacket));
	}

	return (_MakePacket* )pPacket;
}

bool CMakePacketPool::Delete(_MakePacket* pBuffPacket)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

	if(NULL == pBuffPacket)
	{
		return false;
	}
	pBuffPacket->Clear();

	mapPacket::iterator f = m_mapPacketUsed.find(pBuffPacket);
	if(f != m_mapPacketUsed.end())
	{
		m_mapPacketUsed.erase(f);

		//��ӵ�Free map����
		mapPacket::iterator f = m_mapPacketFree.find(pBuffPacket);
		if(f == m_mapPacketFree.end())
		{
			m_mapPacketFree.insert(mapPacket::value_type(pBuffPacket, pBuffPacket));
		}
	}

	return true;
}


//*********************************************************************

CMakePacket::CMakePacket(void)
{
}

CMakePacket::~CMakePacket(void)
{
	OUR_DEBUG((LM_INFO, "[CMakePacket::~CMakePacket].\n"));
}

bool CMakePacket::Init()
{
	m_MakePacketPool.Init(MAX_PACKET_PARSE);
	return true;
}


bool CMakePacket::PutUDPMessageBlock(const ACE_INET_Addr& AddrRemote, uint8 u1Option, _MakePacket* pMakePacket)
{
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutUDPMessageBlock] Get pMakePacket is NULL.\n"));
		return false;
	}

	pMakePacket->m_u1Option          = u1Option;
	pMakePacket->m_AddrRemote        = AddrRemote;

	ProcessMessageBlock(pMakePacket);

	return true;
}

bool CMakePacket::PutMessageBlock(uint32 u4ConnectID, uint8 u1Option, _MakePacket* pMakePacket)
{
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutMessageBlock] Get pMakePacket is NULL.\n"));
		return false;
	}

	pMakePacket->m_u4ConnectID       = u4ConnectID;
	pMakePacket->m_u1Option          = u1Option;
	pMakePacket->m_PacketType        = PACKET_TCP;

	ProcessMessageBlock(pMakePacket);

	return true;
}

bool CMakePacket::ProcessMessageBlock(_MakePacket* pMakePacket)
{
	if(NULL == pMakePacket)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::ProcessMessageBlock] pMakePacket is NULL.\n"));
		return false;
	}

	//���ݲ���OP��������Ӧ�ķ�����
	CMessage* pMessage = App_MessageServiceGroup::instance()->CreateMessage(pMakePacket->m_u4ConnectID, pMakePacket->m_PacketType);
	if(NULL == pMessage)
	{
		OUR_DEBUG((LM_ERROR,"[CMakePacket::ProcessMessageBlock] pMessage is NULL.\n"));
		return false;
	}
	
	if(pMakePacket->m_u1Option == PACKET_PARSE)
	{
		if(pMakePacket->m_PacketType == 0)
		{
			//TCP���ݰ�������
			SetMessage(pMakePacket->m_pPacketParse, pMakePacket->m_u4ConnectID, pMessage);
		}
		else
		{
			//UDP���ݰ�������
			SetMessage(pMakePacket->m_pPacketParse, pMakePacket->m_AddrRemote, pMessage);
		}		
	}
	else if(pMakePacket->m_u1Option == PACKET_CONNECT)
	{
		SetMessageConnect(pMakePacket->m_u4ConnectID, pMessage);
	}
	else if(pMakePacket->m_u1Option == PACKET_CDISCONNECT)
	{
		SetMessageCDisConnect(pMakePacket->m_u4ConnectID, pMessage);
	}
	else if(pMakePacket->m_u1Option == PACKET_SDISCONNECT)
	{
		SetMessageSDisConnect(pMakePacket->m_u4ConnectID, pMessage);
	}
	else if(pMakePacket->m_u1Option == PACKET_SEND_TIMEOUT)
	{
		SetMessageSendTimeout(pMakePacket->m_u4ConnectID, pMessage);
	}
	else if(pMakePacket->m_u1Option == PACKET_CHEK_TIMEOUT)
	{
		SetMessageSendTimeout(pMakePacket->m_u4ConnectID, pMessage);
	}

	//��Ҫ�������Ϣ������Ϣ�����߳�
	if(false == App_MessageServiceGroup::instance()->PutMessage(pMessage))
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::ProcessMessageBlock] App_MessageServiceGroup::instance()->PutMessage Error.\n"));
		App_MessageServiceGroup::instance()->DeleteMessage(pMakePacket->m_u4ConnectID, pMessage);
		return false;
	}

	return true;
}

void CMakePacket::SetMessage(CPacketParse* pPacketParse, uint32 u4ConnectID, CMessage* pMessage)
{
	//������ݰ�ͷ��Ϣ
	_PacketHeadInfo objPacketHeadInfo;
	pPacketParse->GetPacketHeadInfo(objPacketHeadInfo);

	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = pPacketParse->GetPacketCommandID();
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = pPacketParse->GetPacketHeadSrcLen();
		pMessage->GetMessageBase()->m_u4BodySrcSize = pPacketParse->GetPacketBodySrcLen();

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHeadInfo(objPacketHeadInfo);
		pMessage->SetPacketHead(pPacketParse->GetMessageHead());
		pMessage->SetPacketBody(pPacketParse->GetMessageBody());
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

void CMakePacket::SetMessage(CPacketParse* pPacketParse, const ACE_INET_Addr& AddrRemote, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = UDP_HANDER_ID;
		pMessage->GetMessageBase()->m_u2Cmd         = pPacketParse->GetPacketCommandID();
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4Port        = (uint32)AddrRemote.get_port_number();
		pMessage->GetMessageBase()->m_u1PacketType  = PACKET_UDP;
		pMessage->GetMessageBase()->m_u4HeadSrcSize = pPacketParse->GetPacketHeadSrcLen();
		pMessage->GetMessageBase()->m_u4BodySrcSize = pPacketParse->GetPacketBodySrcLen();
		sprintf_safe(pMessage->GetMessageBase()->m_szIP, MAX_BUFF_20, "%s", AddrRemote.get_host_addr());

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(pPacketParse->GetMessageHead());
		pMessage->SetPacketBody(pPacketParse->GetMessageBody());
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CProConnectHandle::SetMessage] UDP ConnectID, pMessage->GetMessageBase() is NULL.\n"));
	}
}

void CMakePacket::SetMessageConnect(uint32 u4ConnectID, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = CLIENT_LINK_CONNECT;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageConnect] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

void CMakePacket::SetMessageCDisConnect(uint32 u4ConnectID, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = CLIENT_LINK_CDISCONNET;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageCDisConnect] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

void CMakePacket::SetMessageSDisConnect(uint32 u4ConnectID, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = CLIENT_LINK_SDISCONNET;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageSDisConnect] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

void CMakePacket::SetMessageSendTimeout(uint32 u4ConnectID, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = CLINET_LINK_SENDTIMEOUT;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageSendTimeout] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

void CMakePacket::SetMessageCheckTimeout(uint32 u4ConnectID, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = CLINET_LINK_CHECKTIMEOUT;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(NULL);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageCheckTimeout] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}


void CMakePacket::SetMessageSendError(uint32 u4ConnectID, ACE_Message_Block* pBodyMessage, CMessage* pMessage)
{
	if(NULL != pMessage->GetMessageBase())
	{
		//��ʼ��װ����
		pMessage->GetMessageBase()->m_u4ConnectID   = u4ConnectID;
		pMessage->GetMessageBase()->m_u2Cmd         = (uint16)CLINET_LINK_SENDERROR;
		pMessage->GetMessageBase()->m_u4MsgTime     = (uint32)ACE_OS::gettimeofday().sec();
		pMessage->GetMessageBase()->m_u4HeadSrcSize = 0;
		pMessage->GetMessageBase()->m_u4BodySrcSize = 0;

		//�����ܵ����ݻ������CMessage����
		pMessage->SetPacketHead(NULL);
		pMessage->SetPacketBody(pBodyMessage);
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::SetMessageSendError] ConnectID = %d, pMessage->GetMessageBase() is NULL.\n", u4ConnectID));
	}
}

bool CMakePacket::PutSendErrorMessage(uint32 u4ConnectID, ACE_Message_Block* pBodyMessage)
{
	CMessage* pMessage = App_MessageServiceGroup::instance()->CreateMessage(u4ConnectID, (uint8)PACKET_TCP);
	if(NULL == pMessage)
	{
		OUR_DEBUG((LM_ERROR, "[CMakePacket::PutSendErrorMessage] pMessage is NULL.\n"));
		pBodyMessage->release();
		return false;
	}
	
	SetMessageSendError(u4ConnectID, pBodyMessage, pMessage);
	if(NULL != pMessage)
	{
		//��Ҫ�������Ϣ������Ϣ�����߳�
		if(false == App_MessageServiceGroup::instance()->PutMessage(pMessage))
		{
			OUR_DEBUG((LM_ERROR, "[CMakePacket::PutSendErrorMessage] App_MessageServiceGroup::instance()->PutMessage Error.\n"));
			pBodyMessage->release();
			App_MessageServiceGroup::instance()->DeleteMessage(u4ConnectID, pMessage);
			return false;
		}
	}
	else
	{
		return false;	
	}

	return true;
}

