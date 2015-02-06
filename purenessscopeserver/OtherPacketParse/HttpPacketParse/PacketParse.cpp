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

void CPacketParse::Init()
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


bool CPacketParse::SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager)
{
	//��������Լ��԰�ͷ�ķ�������Ҫ�����������ȡ�
	char* pData  = (char* )pmbHead->rd_ptr();
	uint32 u4Len = pmbHead->length();

	m_u4HeadSrcSize = u4Len;
	if(u4Len == sizeof(uint32))
	{
		memcpy_safe(pData, (uint32)sizeof(uint32), (char* )&m_u4PacketData, (uint32)sizeof(uint32));
		
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
		memcpy_safe(pData, (uint32)sizeof(uint16), (char* )&m_u2PacketCommandID, (uint32)sizeof(uint16));
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
	memcpy_safe((char* )&u4Len, (uint32)sizeof(uint32), (char* )pMbData->wr_ptr(), (uint32)sizeof(uint32));
	pMbData->wr_ptr(sizeof(uint32));
	memcpy_safe((char* )pData, u4Len, (char* )pMbData->wr_ptr(), u4Len);
	pMbData->wr_ptr(u4Len);

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

bool CPacketParse::Connect(uint32 u4ConnectID, _ClientIPInfo objClientIPInfo, _ClientIPInfo objLocalIPInfo)
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

	//OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]sizeof()=%d.\n", sizeof(pHttpInfo->m_szData)));
	//OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]m_u4DataLength=%d.\n", pHttpInfo->m_u4DataLength));
	//OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]u4Data=%d.\n", u4Data));
	
	memcpy_safe(pData, u4Data, (char* )&pHttpInfo->m_szData[pHttpInfo->m_u4DataLength], u4Data);
	pHttpInfo->m_u4DataLength += u4Data;
	pHttpInfo->m_szData[pHttpInfo->m_u4DataLength] = '\0';

	//Ѱ��Http��ͷ
	char* pHttpHead = ACE_OS::strstr(pHttpInfo->m_szData, HTTP_HEAD_END);
	if(NULL == pHttpHead)
	{
		return PACKET_GET_NO_ENOUGTH;
	}

	uint32 u4HttpHeadLen = pHttpHead - pHttpInfo->m_szData - 4;
	//OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]u4HttpHeadLen=%d.\n", u4HttpHeadLen));

	//�ҵ��������İ�ͷ
	m_pmbHead = pMessageBlockManager->Create(u4HttpHeadLen);
	if(NULL == m_pmbHead)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]m_pmbHead is NULL.\n"));
		return PACKET_GET_ERROR;
	}

	memcpy_safe((char*)pHttpInfo->m_szData, u4HttpHeadLen, (char* )m_pmbHead->wr_ptr(), u4HttpHeadLen);
	m_pmbHead->wr_ptr(u4HttpHeadLen);

	//����������
	m_u2PacketCommandID = 0xea01;

	//�鿴��û�а���
	uint32 u4HttpBodyLength = 0;
	uint8 u1Ret = GetHttpBodyLen(pHttpInfo->m_szData, pHttpInfo->m_u4DataLength, u4HttpHeadLen, u4HttpBodyLength);
	if(u1Ret != PACKET_GET_ENOUGTH)
	{
		return u1Ret;
	}

	//OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]u4HttpBodyLength=%d.\n", u4HttpBodyLength));
	if(u4HttpBodyLength == 0)
	{
		//��ð���
		m_pmbBody = pMessageBlockManager->Create(sizeof(uint32));
		if(NULL == m_pmbBody)
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_pmbBody is NULL.\n"));
			return PACKET_GET_ERROR;
		}

		memcpy_safe((char*)&u4HttpHeadLen, (uint32)sizeof(uint32), m_pmbBody->wr_ptr(), (uint32)sizeof(uint32));
		m_pmbBody->wr_ptr(sizeof(uint32));
	}
	else
	{
		//�а��壬��������
		m_pmbBody = pMessageBlockManager->Create(u4HttpBodyLength);
		if(NULL == m_pmbBody)
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::HttpDispose]m_pmbBody is NULL.\n"));
			return PACKET_GET_ERROR;
		}

		memcpy_safe((char*)pHttpHead, u4HttpBodyLength, m_pmbBody->wr_ptr(), u4HttpBodyLength);
		m_pmbBody->wr_ptr(u4HttpBodyLength);
	}

	//����������ݴӳ����Ƴ�
	pCurrMessage->rd_ptr(u4Data);
	
	pHttpInfo->m_u4DataLength = 0;
	return (uint8)PACKET_GET_ENOUGTH;
}

uint8 CPacketParse::GetHttpBodyLen(char* pData, uint32 u4Len, uint32 u4HeadLen, uint32& u4BodyLen)
{
	char szBodyLen[10] = {'\0'};
	int nNameLen = ACE_OS::strlen(HTTP_BODY_LENGTH);
		
	//OUR_DEBUG((LM_ERROR, "[CPacketParse::GetHttpBodyLen]nNameLen=%d.\n", nNameLen));
	//OUR_DEBUG((LM_ERROR, "[CPacketParse::GetHttpBodyLen]pData=%s.\n", pData));

	//����������Http����
	char* pLength = ACE_OS::strstr(pData, HTTP_BODY_LENGTH);
	if(NULL != pLength)
	{
		uint8 u1LengthLen = 0;
		//�����˰����ֶ�,�����������ֶ�
		for(int i = nNameLen; i < nNameLen + 9; i++)
		{
			if(pLength[i] == '\r')
			{
				break;
			}
			u1LengthLen++;
		}

		//OUR_DEBUG((LM_ERROR, "[CPacketParse::GetHttpBodyLen]u1LengthLen=%d.\n", u1LengthLen));
		memcpy_safe((char* )&pLength[nNameLen], u1LengthLen, szBodyLen, 10);

		u4BodyLen = ACE_OS::atoi(szBodyLen);
		//OUR_DEBUG((LM_ERROR, "[CPacketParse::GetHttpBodyLen]u4BodyLen=%d.\n", u4BodyLen));
		if(u4BodyLen == 0)
		{
			return PACKET_GET_ERROR;
		}

		//������ܵ��ֽڱ�http��������ݳ��ȶ̣�˵��û������
		if(u4BodyLen > u4Len - u4HeadLen)
		{
			return PACKET_GET_NO_ENOUGTH;
		}
	}
	else
	{
		//�Ҳ����������Ͱ�����������������
		u4BodyLen = u4Len - u4HeadLen;
	}

	return PACKET_GET_ENOUGTH;
}

void CPacketParse::GetPacketHeadInfo( _PacketHeadInfo& objPacketHeadInfo )
{
}