#include "PacketParseBase.h"

CPacketParseBase::CPacketParseBase(void) 
{
	//���������ð�ͷ�ĳ��ȣ���Ϊ�󲿷ֵİ�ͷ�����ǹ̶��ġ����峤���ǿɱ�ġ�
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;
	m_u4HeadSrcSize     = 0;
	m_u4BodySrcSize     = 0;

	//�����޸�������İ������汾��
	sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.90");

	//����������İ�ģʽ
	m_u1PacketMode      = PACKET_WITHHEAD;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

CPacketParseBase::~CPacketParseBase(void)
{
	m_objCurrBody.Close();
}

void CPacketParseBase::Init()
{
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;
	m_u4HeadSrcSize     = 0;
	m_u4BodySrcSize     = 0;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

void CPacketParseBase::Clear()
{
	m_pmbHead = NULL;
	m_pmbBody = NULL;

	m_blIsHead = false;

	m_objCurrBody.Clear();

	m_u4PacketHead      = 0;
	m_u4PacketData      = 0;
	m_u4HeadSrcSize     = 0;
	m_u4BodySrcSize     = 0;
	m_u2PacketCommandID = 0;
}

void CPacketParseBase::Close()
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

	m_objCurrBody.Close();
	m_blIsHead = false;
}

const char* CPacketParseBase::GetPacketVersion()
{
	return m_szPacketVersion;
}

uint8 CPacketParseBase::GetPacketMode()
{
	return (uint8)m_u1PacketMode;
}

uint32 CPacketParseBase::GetPacketHeadLen()
{
	return m_u4PacketHead;
}

uint32 CPacketParseBase::GetPacketBodyLen()
{
	return m_u4PacketData;
}

uint16 CPacketParseBase::GetPacketCommandID()
{
	return m_u2PacketCommandID;
}

bool CPacketParseBase::GetIsHead()
{
	return m_blIsHead;
}

uint32 CPacketParseBase::GetPacketHeadSrcLen()
{
	return m_u4HeadSrcSize;
};

uint32 CPacketParseBase::GetPacketBodySrcLen()
{
	return m_u4BodySrcSize;
};

ACE_Message_Block* CPacketParseBase::GetMessageHead()
{
	return m_pmbHead;
};

ACE_Message_Block* CPacketParseBase::GetMessageBody()
{
	return m_pmbBody;
};

