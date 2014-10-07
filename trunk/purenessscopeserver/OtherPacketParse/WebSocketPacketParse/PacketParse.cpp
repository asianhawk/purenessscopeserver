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

	//�����ж���Ӧ�������ְ��߼��������ݰ��߼�
	_WebSocketInfo* pWebSocketInfo = App_WebSocketInfoManager::instance()->GetWebSocketInfo(u4ConnectID);
	if(NULL == pWebSocketInfo)
	{
		//û���ҵ����������Ϣ��˵���д��󣬶Ͽ�����
		return PACKET_GET_ERROR;
	}

	if(pWebSocketInfo->m_emState == WEBSOCKET_STATE_HANDIN)
	{
		return WebSocketDisposeHandIn(pWebSocketInfo, pCurrMessage, pMessageBlockManager);
	}
	else
	{
		return WebSocketDisposeDataIn(pWebSocketInfo, pCurrMessage, pMessageBlockManager);
	}
}

bool CPacketParse::Connect(uint32 u4ConnectID, _ClientIPInfo objClientIPInfo, _ClientIPInfo objLocalIPInfo)
{
	//�������������ӽ������߼��������û������������д�κδ���
	//����false�������ӻ�Ͽ�

	//���ҵ�ǰ���Ӽ�¼�������޵�ǰ��¼
	return App_WebSocketInfoManager::instance()->Insert(u4ConnectID);
}

void CPacketParse::DisConnect(uint32 u4ConnectID)
{
	//�������������ӶϿ����߼�����
	App_WebSocketInfoManager::instance()->Delete(u4ConnectID);
}

uint8 CPacketParse::WebSocketDisposeHandIn(_WebSocketInfo* pWebSocketInfo, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	//�������ְ�
	char* pData   = pCurrMessage->rd_ptr();     //�õ�������ݿ�����ֽ�
	uint32 u4Data = pCurrMessage->length();     //�õ�������ݿ�ĳ���

	//���յ�������ճ�뻺��ȴ����а�����
	if(pWebSocketInfo->m_u4DataLength + u4Data > MAX_DECRYPTLENGTH)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]pWebSocketInfo is full.\n"));
		return PACKET_GET_ERROR;
	}

	ACE_OS::memcpy(&pWebSocketInfo->m_szData[pWebSocketInfo->m_u4DataLength], pData, u4Data);
	pWebSocketInfo->m_u4DataLength += u4Data;

	//�ж��ǲ������ְ��Ľ������ҵ�ĩβ4���ַ��ǲ���\r\n\r\n
	if(pData[u4Data - 1] == '\n' && pData[u4Data - 2] == '\r'
		&& pData[u4Data - 3] == '\n' && pData[u4Data - 4] == '\r')
	{
		//���յ����������������ݰ�����ʼ��������

		//����һ����ͷ����¼��ǰ�������г���
		m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
		if(NULL == m_pmbHead)
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_pmbHead is NULL.\n"));
			return PACKET_GET_ERROR;
		}

		uint32 u4NetPacketLen = pWebSocketInfo->m_u4DataLength;
		memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));
		m_pmbHead->wr_ptr(sizeof(uint32));

		//��ð���
		m_pmbBody = pMessageBlockManager->Create(u4NetPacketLen);
		if(NULL == m_pmbBody)
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_pmbBody is NULL.\n"));
			return PACKET_GET_ERROR;
		}

		ACE_OS::memcpy(m_pmbBody->wr_ptr(), (char*)pWebSocketInfo->m_szData, pWebSocketInfo->m_u4DataLength);
		m_pmbBody->wr_ptr(u4NetPacketLen);

		//����������(0xe001ָ��������ID)
		m_u2PacketCommandID = 0xee01;

		//����Ϊ���ݰ���ʽ
		pWebSocketInfo->m_emState = WEBSOCKET_STATE_DATAIN;

		//����������ݴӳ����Ƴ�
		pCurrMessage->rd_ptr(u4Data);

		//���û��崦�����
		pWebSocketInfo->m_u4DataLength = 0;

		return (uint8)PACKET_GET_ENOUGTH;
	}
	else
	{
		//û�н��յ��������������ݰ�����������
		return (uint8)PACKET_GET_NO_ENOUGTH;
	}
}

uint8 CPacketParse::WebSocketDisposeDataIn(_WebSocketInfo* pWebSocketInfo, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	//�������ݰ�
	char* pData   = pCurrMessage->rd_ptr();     //�õ�������ݿ�����ֽ�
	uint32 u4Data = pCurrMessage->length();     //�õ�������ݿ�ĳ���
	uint32 u4PacketLen = 0;
	uint8 u1Ret        = (uint8)PACKET_GET_ERROR;  

	//���յ�������ճ�뻺��ȴ����а�����
	if(pWebSocketInfo->m_u4DataLength + u4Data > MAX_ENCRYPTLENGTH)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]pWebSocketInfo is full.\n"));
		return (uint8)PACKET_GET_ERROR;
	}

	ACE_OS::memcpy(&pWebSocketInfo->m_szData[pWebSocketInfo->m_u4DataLength], pData, u4Data);
	pWebSocketInfo->m_u4DataLength += u4Data;

	//��������Լ��Ϊ
	//<������>,<���ݰ�����><���ݰ���>

	//���Ƚ������ݰ�,���������ݰ���5K
	//����ٴ�����չ�����С
	if(pWebSocketInfo->m_u4DecryptDataLen + pWebSocketInfo->m_u4DataLength > MAX_DECRYPTLENGTH)
	{
		OUR_DEBUG((LM_ERROR, "[CPacketParse::WebSocketDisposeHandIn]m_u4DecryptDataLen is full.\n"));
		return (uint8)PACKET_GET_ERROR;
	}

	char* pDecryptData = (char* )&pWebSocketInfo->m_szDecryptData[pWebSocketInfo->m_u4DecryptDataLen];
	uint32 u4DecryptLen = MAX_DECRYPTLENGTH;
	uint32 u4OriPacketLen = pWebSocketInfo->m_u4DataLength;
	u1Ret = Decrypt(pWebSocketInfo->m_szData, u4OriPacketLen, pDecryptData, u4DecryptLen);
	if(u1Ret != (uint8)PACKET_GET_ENOUGTH)
	{
		return u1Ret;
	}

	pWebSocketInfo->m_u4DecryptDataLen += u4DecryptLen;

	//��������ֽ�����9��������Ϊ��ͷ������
	if(u4Data < 9)
	{
		return (uint8)PACKET_GET_NO_ENOUGTH;
	}

	//�Ѿ��������������ݰ����ж��Ƿ��к������ݰ��Ĳ�������
	if(pWebSocketInfo->m_u4DataLength > u4OriPacketLen)
	{
		//�к��������ݰ�����������Ҫ����һ��
		pWebSocketInfo->m_u4DataLength -= u4OriPacketLen;

		ACE_OS::memcpy(&pWebSocketInfo->m_szData, &pWebSocketInfo->m_szData[u4OriPacketLen], pWebSocketInfo->m_u4DataLength);
	}
	else
	{
		pWebSocketInfo->m_u4DataLength = 0;
	}
	
	//���յ������������ݰ�����ʼ��������
	//��õ�ǰ��ID�Ͱ���
	uint32 u4CurrDecryptDataLen = pWebSocketInfo->m_u4DecryptDataLen;
	u1Ret = ReadDataPacketInfo(pWebSocketInfo->m_szDecryptData, u4CurrDecryptDataLen, m_u2PacketCommandID, u4PacketLen);
	if(u1Ret == PACKET_GET_ENOUGTH)
	{
		//����һ����ͷ����¼��ǰ�������г���
		m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
		if(NULL == m_pmbHead)
		{
			return (uint8)PACKET_GET_ERROR;
		}

		//ȥ����ͷ��ֻ���������ڰ�����
		char* pInfo = (char* )ACE_OS::strstr(pWebSocketInfo->m_szDecryptData, "{");
		if(NULL == pInfo)
		{
			//����Ҳ��������ţ�˵�����ݰ��쳣���Ͽ��������
			return (uint8)PACKET_GET_ERROR;
		}

		uint32 u4NetPacketLen = u4CurrDecryptDataLen - (uint32)(pInfo - pWebSocketInfo->m_szDecryptData);
		memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));
		m_pmbHead->wr_ptr(sizeof(uint32));

		//��ð���
		m_pmbBody = pMessageBlockManager->Create(u4NetPacketLen);
		if(NULL == m_pmbBody)
		{
			return PACKET_GET_ERROR;
		}

		ACE_OS::memcpy(m_pmbBody->wr_ptr(), (char*)pInfo, u4NetPacketLen);
		m_pmbBody->wr_ptr(u4NetPacketLen);

		//����������ݴӿ����Ƴ�
		pCurrMessage->reset();

		//�����ʣ�����ݣ�������ϵͳ�ж�һ��
		if(pWebSocketInfo->m_u4DataLength > 0)
		{
			pCurrMessage->size((size_t)pWebSocketInfo->m_u4DataLength);
			ACE_OS::memcpy(pCurrMessage->wr_ptr(), pWebSocketInfo->m_szData, pWebSocketInfo->m_u4DataLength);
			pCurrMessage->wr_ptr(pWebSocketInfo->m_u4DataLength);
			pWebSocketInfo->m_u4DataLength = 0;
		}

		//����������������ݣ����ٷŻ�ȥ
		pWebSocketInfo->m_u4DecryptDataLen -= u4CurrDecryptDataLen;
		if(pWebSocketInfo->m_u4DecryptDataLen > 0)
		{
			ACE_OS::memcpy(pWebSocketInfo->m_szDecryptData, 
				(char* )&pWebSocketInfo->m_szDecryptData[u4CurrDecryptDataLen], 
				pWebSocketInfo->m_u4DecryptDataLen);
		}

		return (uint8)PACKET_GET_ENOUGTH;
	}
	else
	{
		return u1Ret;
	}

}

uint8 CPacketParse::Decrypt(char* pOriData, uint32& u4Len, char* pEncryData, uint32& u4EncryLen)
{
	//֡ͷ���ȣ�Ĭ����6,��չ����8
	int nFrameLen = 6;

	if(u4Len < 6)
	{
		m_blIsHead = true;

		return PACKET_GET_NO_ENOUGTH;
	}

	unsigned char* mp_mask_byte = NULL;	
	int nMinExpectedSize = 0;


	unsigned char payloadFlags = pOriData[0];
	if (payloadFlags != 129)
		return PACKET_GET_ERROR;

	unsigned char basicSize = pOriData[1] & 0x7F;
	unsigned int payloadSize;
	int masksOffset;

	if (basicSize <= 125)
	{
		payloadSize = basicSize;
		masksOffset = 2;
	}
	else if (basicSize == 126)
	{
		nMinExpectedSize += 2;
		if (u4Len < (uint32)nMinExpectedSize)
		{
			m_blIsHead = true;
			return PACKET_GET_NO_ENOUGTH;
		}
		payloadSize = ntohs( *(u_short*) (pOriData + 2) );
		masksOffset = 4;
		nFrameLen   = 8;
	}
	else if (basicSize == 127)
	{
		nMinExpectedSize += 8;
		if (u4Len < (uint32)nMinExpectedSize)
		{
			m_blIsHead = true;

			return PACKET_GET_NO_ENOUGTH;
		}
		payloadSize = ntohl( *(u_long*) (pOriData + 2) );
		masksOffset = 6;
		nFrameLen   = 10;
	}
	else
	{
		return PACKET_GET_ERROR;
	}

	nMinExpectedSize += payloadSize;
	if (u4Len < (uint32)nMinExpectedSize)
	{
		m_blIsHead = true;

		return PACKET_GET_NO_ENOUGTH;
	}

	char masks[4];
	memcpy(masks, pOriData + masksOffset, 4);

	memcpy(pEncryData, pOriData + masksOffset + 4, payloadSize);
	for (unsigned int i = 0; i < payloadSize; i++) {
		pEncryData[i] = (pEncryData[i] ^ masks[i%4]);
	}
	pEncryData[payloadSize] = '\0';

	if(u4EncryLen < payloadSize)
	{
		//����ַ������������壬�򷵻ذ�����
		return PACKET_GET_ERROR;
	}

	//ACE_OS::memcpy(pEncryData, mp_payload_data, payloadSize);
	u4EncryLen = payloadSize;
	//����6���ֽ�ͷ�ǹ̶��ģ���һ���ֽڹ̶���-127,����������ݵĳ��Ȼ���4����6�ֽ�
	if(payloadSize + nFrameLen > u4Len)
	{
		//˵������������������������ݳ��Ⱥ͵�ǰ���ݳ��Ȳ����
		return PACKET_GET_ERROR;
	}

	u4Len      = payloadSize + nFrameLen;

	return PACKET_GET_ENOUGTH;
}

uint8 CPacketParse::ReadDataPacketInfo(const char* pData, uint32& u4DataLen, uint16& u2CommandID, uint32& u4PacketLen)
{
	uint32 u4AllDataCount = u4DataLen;

	char* pInfo = NULL;
	char szTemp[MAX_BUFF_100] = {'\0'};

	//Ѱ�ҵ�һ��{�����ڽ綨���ݰ���Ϣ�ĳ���
	pInfo = (char* )ACE_OS::strstr(pData, "{");
	if(NULL == pInfo)
	{
		//����Ҳ��������ţ�˵�����ݰ��쳣���Ͽ��������
		return (uint8)PACKET_GET_ERROR;
	}

	if((int)(pInfo - pData) == 0)
	{
		//�Ҳ���ǰ�������ͷ������ʧ��
		return (uint8)PACKET_GET_ERROR;
	}

	//��������ַ���
	ACE_OS::memcpy(szTemp, pData, (int)(pInfo - pData));

	//����ҵ��ˣ���ʼ�з����ݣ�������ݳ��Ⱥ�����ID
	char* pCommand = NULL;
	pCommand = (char* )ACE_OS::strstr(szTemp, ",");
	if(NULL == pInfo)
	{
		//����Ҳ������ţ�˵�����������Ǵ���ģ��Ͽ�����
		return (uint8)PACKET_GET_ERROR;
	}
	else
	{
		char szTemp2[MAX_BUFF_50] = {'\0'}; 

		if(pCommand - szTemp == 0)
		{
			//˵��û���ҵ�����ͷ������ʧ��
			return (uint8)PACKET_GET_ERROR;
		}

		ACE_OS::memcpy(szTemp2, pData, (int)(pCommand - szTemp));
		//�õ������ֺ����ݰ�����
		//u2CommandID = (uint16)ACE_OS::atoi(szTemp2);
		//ʮ�������ַ���ת������
		int nCommandID = 0;
		sscanf(szTemp2, "%x", &nCommandID);
		m_u2PacketCommandID = (uint16)nCommandID;
		ACE_OS::memcpy(szTemp2, pCommand + 1, ACE_OS::strlen(szTemp) - (int)(pCommand - szTemp) - 1);
		szTemp2[ACE_OS::strlen(szTemp) - (int)(pCommand - szTemp) - 1] = '\0';
		u4PacketLen = (uint32)ACE_OS::atoi(szTemp2);
		
		if(u4PacketLen == 0 || u2CommandID == 0)
		{
			//�������������ֺͳ�����һ�����ԾͶϿ�����
			return (uint8)PACKET_GET_ERROR;
		}
		else
		{
			//�жϰ��Ƿ����������
			if(u4PacketLen > u4AllDataCount)
			{
				return (uint8)PACKET_GET_NO_ENOUGTH;
			}
			else
			{
				u4DataLen = u4PacketLen;
				return (uint8)PACKET_GET_ENOUGTH;
			}
		}
	}
}

void CPacketParse::GetPacketHeadInfo( _PacketHeadInfo& objPacketHeadInfo )
{
}