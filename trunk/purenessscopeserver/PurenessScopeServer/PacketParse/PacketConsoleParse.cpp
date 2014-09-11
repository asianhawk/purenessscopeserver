#include "PacketConsoleParse.h"


CConsolePacketParse::CConsolePacketParse(void)
{
	//����ǰ�ͷģʽ��������Ҫ���ð�ͷ�ĳ���
	m_u4PacketHead      = PACKET_HEAD;

	//�����޸�������İ������汾��
	sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.92");

	//����������İ�ģʽ
	m_u1PacketMode      = PACKET_WITHHEAD;
}

CConsolePacketParse::~CConsolePacketParse(void)
{
	
}


void CConsolePacketParse::Init()
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

bool CConsolePacketParse::SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager)
{
	char* pData = pmbHead->rd_ptr();
	if(u4ConnectID == 0 && pMessageBlockManager != NULL)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	//��������Լ��԰�ͷ�ķ�������Ҫ�����������ȡ�
	uint32 u4Len = pmbHead->length();

	m_u4HeadSrcSize = u4Len;
	if(u4Len == sizeof(uint32))
	{
		ACE_OS::memcpy(&m_u4PacketData, pData, sizeof(uint32));
		
		m_pmbHead = pmbHead;
		m_blIsHead = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool CConsolePacketParse::SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager)
{
	//��������������ڵ�һЩ���ݣ������ͷ������CommandID����ô����Ͳ�����������
	if(u4ConnectID == 0 && pMessageBlockManager != NULL)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	uint32 u4Len = pmbBody->length();

	m_u4BodySrcSize = u4Len;
	if(u4Len >= sizeof(uint16))
	{
		//ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));
		m_blIsHead = false;
		m_pmbBody = pmbBody;
		return true;
	}
	else
	{
		m_blIsHead = false;
		return false;
	}
}


uint32 CConsolePacketParse::MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen, uint16 u2CommandID)
{
	if(u4ConnectID == 0 && u2CommandID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	return u4DataLen + sizeof(uint32);
}

bool CConsolePacketParse::MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData, uint16 u2CommandID)
{
	if(u4ConnectID == 0 && u2CommandID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	if(pMbData == NULL)
	{
		return false;
	}

	//ƴװ���ݰ�
	ACE_OS::memcpy(pMbData->wr_ptr(), (const void*)&u4Len, sizeof(uint32));
	ACE_OS::memcpy(pMbData->wr_ptr() + sizeof(uint32), (const void*)pData, u4Len);
	pMbData->wr_ptr(u4Len + sizeof(uint32));

	return true;
}



uint8 CConsolePacketParse::GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	if(0 == u4ConnectID || NULL == pCurrMessage || NULL == pMessageBlockManager)
	{
		return PACKET_GET_ERROR;
	}
	else
	{
		return PACKET_GET_ERROR;
	}
}


bool CConsolePacketParse::Connect( uint32 u4ConnectID, _ClientIPInfo& objClientIPInfo)
{
	//�������������ӽ������߼��������û������������д�κδ���
	//����false�������ӻ�Ͽ�
	if(u4ConnectID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}
	
	OUR_DEBUG((LM_INFO, "[CConsolePacketParse::Connect]IP=%s,Port=%d.\n", objClientIPInfo.m_szClientIP, objClientIPInfo.m_nPort));

	return true;
}

void CConsolePacketParse::DisConnect( uint32 u4ConnectID )
{
	//�������������ӶϿ����߼�����
	if(u4ConnectID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}
}

void CConsolePacketParse::GetPacketHeadInfo(_PacketHeadInfo& objPacketHeadInfo)
{
	//Console��ʵ�ִ˷���
	objPacketHeadInfo.Clear();
	return;
}


