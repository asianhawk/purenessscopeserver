#include "ServerParse.h"

CServerParse::CServerParse()
{
	//���������ð�ͷ�ĳ��ȣ���Ϊ�󲿷ֵİ�ͷ�����ǹ̶��ġ����峤���ǿɱ�ġ�
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

CServerParse::~CServerParse()
{
	OUR_DEBUG((LM_INFO, "[CServerParse::~CServerParse].\n"));
}

void CServerParse::Init()
{
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

uint32 CServerParse::GetPacketHeadLen()
{
	return m_u4PacketHead;
}

uint32 CServerParse::GetPacketBodyLen()
{
	return m_u4PacketData;
}

uint16 CServerParse::GetPacketCommandID()
{
	return m_u2PacketCommandID;
}

bool CServerParse::GetIsHead()
{
	return m_blIsHead;
}

bool CServerParse::SetPacketHead(char* pData, uint32 u4Len)
{
	//��������Լ��԰�ͷ�ķ�������Ҫ�����������ȡ�
	if(u4Len == sizeof(uint32))
	{
		memcpy_safe(pData, (uint32)sizeof(uint32), (char* )&m_u4PacketData, (uint32)sizeof(uint32));
		m_blIsHead = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool CServerParse::SetPacketBody(char* pData, uint32 u4Len)
{
	//��������������ڵ�һЩ���ݣ������ͷ������CommandID����ô����Ͳ�����������
	if(u4Len >= sizeof(uint16))
	{
		memcpy_safe(pData, (uint32)sizeof(uint16), (char* )&m_u2PacketCommandID, (uint32)sizeof(uint16));
		m_blIsHead = false;
		return true;
	}
	else
	{
		m_blIsHead = false;
		return false;
	}
}

bool CServerParse::SetMessageHead(ACE_Message_Block* pmbHead)
{
	m_pmbHead = pmbHead;
	return true;
}

bool CServerParse::SetMessageBody(ACE_Message_Block* pmbBody)
{
	m_pmbBody = pmbBody;
	return true;
}

ACE_Message_Block* CServerParse::GetMessageHead()
{
	return m_pmbHead;
}

ACE_Message_Block* CServerParse::GetMessageBody()
{
	return m_pmbBody;
}

uint32 CServerParse::MakePacketLength(uint32 u4DataLen)
{
	return u4DataLen + sizeof(uint32);
}

bool CServerParse::MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData)
{
	if(pMbData == NULL)
	{
		return false;
	}

	//ƴװ���ݰ�
	memcpy_safe((char* )&u4Len, (uint32)sizeof(uint32), pMbData->wr_ptr(), (uint32)sizeof(uint32));
	pMbData->wr_ptr(sizeof(uint32));
	//NOTICE�������pMbData���ܻ����ڴ������������Ҫ����һ��
	memcpy_safe((char* )pData, u4Len, pMbData->wr_ptr(), u4Len);
	pMbData->wr_ptr(u4Len);

	return true;
}

void CServerParse::Close()
{
	if(m_pmbHead != NULL)
	{
		m_pmbHead->release();
		m_pmbHead = NULL;
	}

	if(m_pmbBody != NULL)
	{
		m_pmbBody->release();
		m_pmbBody = NULL;
	}

	m_blIsHead = false;
}

