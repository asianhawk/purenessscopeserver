#include "PacketParse.h"


CPacketParse::CPacketParse(void)
{
	//����ǰ�ͷģʽ��������Ҫ���ð�ͷ�ĳ���
	m_u4PacketHead      = PACKET_HEAD;

	//�����޸�������İ������汾��
	sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.90");

	//����������İ�ģʽ
	m_u1PacketMode      = PACKET_WITHSTREAM;
}

CPacketParse::~CPacketParse(void)
{
}

bool CPacketParse::SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager)
{
	//��������Լ��԰�ͷ�ķ�������Ҫ�����������ȡ�
	char* pData  = (char* )pmbHead->rd_ptr();
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

bool CPacketParse::SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager)
{
	//��������������ڵ�һЩ���ݣ������ͷ������CommandID����ô����Ͳ�����������
	char* pData  = (char* )pmbBody->rd_ptr();
	uint32 u4Len = pmbBody->length();

	m_u4BodySrcSize = u4Len;
	if(u4Len >= sizeof(uint16))
	{
		ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));
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



uint32 CPacketParse::MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen, uint16 u2CommandID)
{
	if(u2CommandID == 0)
	{
		return 0;
	}
	else
	{
		return u4DataLen + sizeof(uint32);
	}
}

bool CPacketParse::MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData, uint16 u2CommandID)
{
	if(pMbData == NULL && u2CommandID == 0)
	{
		return false;
	}

	//ƴװ���ݰ�
	ACE_OS::memcpy(pMbData->wr_ptr(), (const void*)&u4Len, sizeof(uint32));
	ACE_OS::memcpy(pMbData->wr_ptr() + sizeof(uint32), (const void*)pData, u4Len);
	pMbData->wr_ptr(u4Len + sizeof(uint32));

	return true;
}



uint8 CPacketParse::GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	//ר�Ŵ���Ϊ�����������ݰ�
	if(NULL == pCurrMessage || NULL == pMessageBlockManager)
	{
		return PACKET_GET_ERROR;
	}

	_HttpInfo* pHttpInfo = App_HttpInfoManager::instance()->GetWebSocketInfo(u4ConnectID);
	if(NULL == pHttpInfo)
	{
		//û���ҵ����������Ϣ��˵���д��󣬶Ͽ�����
		return PACKET_GET_ERROR;
	}

	//�ж�http����ͷ�Ƿ�����
	return HttpDispose(pHttpInfo, pCurrMessage, pMessageBlockManager);

}

bool CPacketParse::Connect(uint32 u4ConnectID, _ClientIPInfo& objClientIPInfo)
{
	//�������������ӽ������߼��������û������������д�κδ���
	//����false�������ӻ�Ͽ�

	//���ҵ�ǰ���Ӽ�¼�������޵�ǰ��¼
	return App_HttpInfoManager::instance()->Insert(u4ConnectID);
}

void CPacketParse::DisConnect(uint32 u4ConnectID)
{
	//�������������ӶϿ����߼�����
	App_HttpInfoManager::instance()->Delete(u4ConnectID);
}

uint8 CPacketParse::HttpDispose(_HttpInfo* pHttpInfo, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	char* pData   = pCurrMessage->rd_ptr();     //�õ�������ݿ�����ֽ�
	uint32 u4Data = pCurrMessage->length();     //�õ�������ݿ�ĳ���

	//���յ�������ճ�뻺��ȴ����а�����
	if(pHttpInfo->m_u4DataLength + u4Data > MAX_DECRYPTLENGTH)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]pWebSocketInfo is full.\n"));
		return PACKET_GET_ERROR;
	}

	ACE_OS::memcpy(&pHttpInfo->m_szData[pHttpInfo->m_u4DataLength], pData, u4Data);
	pHttpInfo->m_u4DataLength += u4Data;
	pHttpInfo->m_szData[pHttpInfo->m_u4DataLength] = '\0';

	//Ѱ��Http��ͷ
	char* pHttpHead = ACE_OS::strstr(pHttpInfo->m_szData, HTTP_HEAD_END);
	if(NULL == pHttpHead)
	{
		return PACKET_GET_NO_ENOUGTH;
	}

	uint32 u4HttpHeadLen = pHttpHead - pHttpInfo->m_szData - 4;

	//�ҵ��������İ�ͷ
	m_pmbHead = pMessageBlockManager->Create(u4HttpHeadLen);
	if(NULL == m_pmbHead)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_pmbHead is NULL.\n"));
		return PACKET_GET_ERROR;
	}

	memcpy(m_pmbHead->wr_ptr(), (char*)pHttpInfo->m_szData, u4HttpHeadLen);
	m_pmbHead->wr_ptr(u4HttpHeadLen);

	m_u2PacketCommandID = 0xea01;

	//�鿴��û�а���
	uint32 u4HttpBodyLength = GetHttpBodyLen(pHttpHead);
	if(u4HttpBodyLength == 0)
	{
		//��ð���
		m_pmbBody = pMessageBlockManager->Create(sizeof(uint32));
		if(NULL == m_pmbBody)
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_pmbBody is NULL.\n"));
			return PACKET_GET_ERROR;
		}

		memcpy(m_pmbBody->wr_ptr(), (char*)&u4HttpHeadLen, sizeof(uint32));
		m_pmbHead->wr_ptr(sizeof(uint32));
	}

	//����������ݴӳ����Ƴ�
	pCurrMessage->rd_ptr(u4Data);

	pHttpInfo->m_u4DataLength = 0;
	return (uint8)PACKET_GET_ENOUGTH;
}

uint32 CPacketParse::GetHttpBodyLen(const char* pHttpHead)
{
	//char* pLength = ACE_OS::strstr(pHttpHead, HTTP_BODY_LENGTH);

	return 0;
}
