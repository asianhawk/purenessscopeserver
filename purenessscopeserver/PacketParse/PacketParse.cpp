#include "PacketParse.h"


CPacketParse::CPacketParse(void)
{
	//����ǰ�ͷģʽ��������Ҫ���ð�ͷ�ĳ���
	m_u4PacketHead      = PACKET_HEAD_LENGTH;

	//�����޸�������İ������汾��
	sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.94");

	//����������İ�ģʽ
	m_u1PacketMode      = PACKET_WITHHEAD;
}

CPacketParse::~CPacketParse(void)
{
	
}

void CPacketParse::Init()
{
	m_u4PacketHead      = PACKET_HEAD_LENGTH;
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
	//��ð�ͷ30���ֽڵ������Ϣ����ԭ�����ݰ���Ϣ�ṹ
	char* pData  = (char* )pmbHead->rd_ptr();
	uint32 u4Len = pmbHead->length();
	uint32 u4Pos = 0;

	ACE_OS::memcpy((char* )&m_objPacketHeadInfo.m_u2Version, (char* )&pData[u4Pos], sizeof(uint16));
	u4Pos += sizeof(uint16);
	ACE_OS::memcpy((char* )&m_objPacketHeadInfo.m_u2CmdID, (char* )&pData[u4Pos], sizeof(uint16));
	u4Pos += sizeof(uint16);
	ACE_OS::memcpy((char* )&m_objPacketHeadInfo.m_u4BodyLen, (char* )&pData[u4Pos], sizeof(uint32));
	u4Pos += sizeof(uint32);
	ACE_OS::memcpy(m_objPacketHeadInfo.m_szSession, (char* )&pData[u4Pos], sizeof(char)*32);
	u4Pos += sizeof(char)*32;

	m_u4PacketData      = m_objPacketHeadInfo.m_u4BodyLen;
	m_u2PacketCommandID = m_objPacketHeadInfo.m_u2CmdID;

	m_pmbHead = pmbHead;

	return true;
}

bool CPacketParse::SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager)
{
	//��������������ڵ�һЩ���ݣ������ͷ������CommandID����ô����Ͳ�����������
	m_pmbBody = pmbBody;
	return true;

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
	//�����ǲ��Դ��룬ר�Ŵ���Ϊ�����������ݰ�
	if(NULL == pCurrMessage || NULL == pMessageBlockManager)
	{
		return PACKET_GET_ERROR;
	}

	CBuffPacket* pBuffPacket = App_PacketBufferManager::instance()->GetBuffPacket(u4ConnectID);
	if(NULL == pBuffPacket)
	{
		return PACKET_GET_ERROR;
	}

	//����������ҵĹ��򣬱��磬�ҵ����ݰ���һ���ֽ���7E��ͷ����7EΪ��β����ô�һ���ô��
	//���ȣ��ж����������һ���ֽ��ǲ���7E��������ǣ��򷵻ش�������ݡ�
	//����Ǵ������ݣ�Ϊ�˱�֤���ݰ�ȫ����ܻ�ر�������ӡ�
	char* pData   = pCurrMessage->rd_ptr();     //�õ�������ݿ�����ֽ�
	uint32 u4Data = pCurrMessage->length();     //�õ�������ݿ�ĳ���

	//�ж����ݳ����Ƿ�Ϊ0
	if(u4Data <= 0)
	{
		return PACKET_GET_ERROR;
	}

	//��������ֽڲ���7E����֤������Ҫ����ȫ��Ҫճ����Ҫ�����Ǵ����ݰ���
	if(pData[0] == 0x7E)
	{
		//��ͷ��ȷ����ʼѰ�Ұ�β
		uint32 u4Pos  = 1;
		bool   blFind = false;
		for(u4Pos = 1; u4Pos < u4Data; u4Pos++)
		{
			//�ҵ��˰�ĩβ
			if(pData[u4Pos] == 0x7E)
			{
				blFind = true;
				break;
			}
		}

		if(blFind == false)
		{
			if(pCurrMessage->length() > 2)
			{
				//�������������ݷ��뻺�壬�ȴ�������������
				pBuffPacket->WriteStream(pCurrMessage->rd_ptr(), pCurrMessage->length());
			}

			m_blIsHead = true;

			//û���ҵ���β����Ҫ������������
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//���յ����������ݰ�
			uint32 u4PacketLen = u4Pos;

			m_u4PacketHead  = sizeof(uint32);
			m_u4HeadSrcSize = sizeof(uint32);
			m_u4PacketData  = u4PacketLen;
			m_u4BodySrcSize = u4PacketLen;

			//���ڴ��������һ����ͷ
			m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
			if(NULL == m_pmbHead)
			{
				return PACKET_GET_ERROR;
			}

			//��¼��������ת��Ϊ�����ֽ��򣬷����ͷ
			//uint32 u4NetPacketLen = ACE_HTONL(u4PacketLen);
			uint32 u4NetPacketLen = u4PacketLen;
			memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));
			m_pmbHead->wr_ptr(sizeof(uint32));
			
			//���ڴ������һ������
			m_pmbBody = pMessageBlockManager->Create(u4PacketLen);
			if(NULL == m_pmbBody)
			{
				return PACKET_GET_ERROR;
			}

			//��ð�����ID
			ACE_OS::memcpy(&m_u2PacketCommandID, (char*)&pData[1], sizeof(uint16));

			//�������ݷ������
			memcpy(m_pmbBody->wr_ptr(), (char*)&pData[1], u4PacketLen);
			m_pmbBody->wr_ptr(u4PacketLen);

			m_blIsHead = false;

			//����������ݴӳ����Ƴ�
			pCurrMessage->rd_ptr(u4Pos);
			return PACKET_GET_ENOUGTH;
		}
	}
	else
	{
		//������岻���ڣ���˵�������еİ������������
		if(m_blIsHead == false)
		{
			return PACKET_GET_ERROR;
		}

		pBuffPacket->WriteStream(pData, u4Data);

		uint32 u4Pos  = 0;
		bool   blFind = false;
		if(pBuffPacket->GetPacketLen() > 0)
		{
			//����л��壬�򿴿��Ƿ��н������
			for(u4Pos = 1; u4Pos < u4Data; u4Pos++)
			{
				//�ҵ��˰�ĩβ
				if(pData[u4Pos] == 0x7E)
				{
					blFind = true;
					break;
				}
			}
		}
		else
		{
			//û�л��壬������ֽ��ж�
			for(u4Pos = 1; u4Pos < u4Data; u4Pos++)
			{
				//�ҵ��˰�ĩβ
				if(pData[u4Pos] == 0x7E)
				{
					blFind = true;
					break;
				}
			}
		}

		if(blFind == false)
		{
			m_blIsHead = true;

			//û���ҵ���β����Ҫ������������
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//���յ����������ݰ�
			m_u4PacketHead  = sizeof(uint32);
			m_u4HeadSrcSize = sizeof(uint32);
			m_u4PacketData  = u4Pos + 1;
			m_u4BodySrcSize = u4Pos + 1;

			//���ڴ��������һ����ͷ
			m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
			if(NULL == m_pmbHead)
			{
				return PACKET_GET_ERROR;
			}

			//��¼��������ת��Ϊ�����ֽ��򣬷����ͷ
			//uint32 u4NetPacketLen = ACE_HTONL(m_objCurrBody.GetPacketLen());
			uint32 u4NetPacketLen = m_u4PacketData;
			memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));
			m_pmbHead->wr_ptr(sizeof(uint32));

			//���ڴ������һ������
			m_pmbBody = pMessageBlockManager->Create(m_u4PacketData);
			if(NULL == m_pmbBody)
			{
				return PACKET_GET_ERROR;
			}

			//��ð�����ID
			ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));


			//�������ݷ������
			memcpy(m_pmbBody->wr_ptr(), (char*)pBuffPacket->GetData(), m_u4PacketData);
			m_pmbBody->wr_ptr(m_u4PacketData);

			//ɾ�������е�����
			pBuffPacket->RollBack(u4Pos);

			m_blIsHead = false;

			//����������ݴӳ����Ƴ�
			pCurrMessage->rd_ptr(pCurrMessage->length());
			return PACKET_GET_ENOUGTH;
		}

	}
}

bool CPacketParse::Connect(uint32 u4ConnectID, _ClientIPInfo& objClientIPInfo)
{
	//�������������ӽ������߼��������û������������д�κδ���
	//����false�������ӻ�Ͽ�
	App_PacketBufferManager::instance()->AddBuffer(u4ConnectID);

	return true;
}

void CPacketParse::DisConnect(uint32 u4ConnectID)
{
	//�������������ӶϿ����߼�����
	App_PacketBufferManager::instance()->DelBuffer(u4ConnectID);
}

void CPacketParse::GetPacketHeadInfo(_PacketHeadInfo& objPacketHeadInfo)
{
	objPacketHeadInfo.m_u2Version = m_objPacketHeadInfo.m_u2Version;
	objPacketHeadInfo.m_u2CmdID   = m_objPacketHeadInfo.m_u2CmdID;
	objPacketHeadInfo.m_u4BodyLen = m_objPacketHeadInfo.m_u4BodyLen;
	sprintf_safe(objPacketHeadInfo.m_szSession, SESSION_LEN, "%s", m_objPacketHeadInfo.m_szSession);
}

