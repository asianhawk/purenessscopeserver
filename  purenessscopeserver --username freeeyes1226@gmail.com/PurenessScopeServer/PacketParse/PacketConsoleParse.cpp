#include "PacketConsoleParse.h"


CConsolePacketParse::CConsolePacketParse(void)
{
	//����ǰ�ͷģʽ��������Ҫ���ð�ͷ�ĳ���
	m_u4PacketHead      = PACKET_HEAD;

	//�����޸�������İ������汾��
	sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.90");

	//����������İ�ģʽ
	m_u1PacketMode      = PACKET_WITHHEAD;
}

CConsolePacketParse::~CConsolePacketParse(void)
{
	
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


uint32 CConsolePacketParse::MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen)
{
	if(u4ConnectID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	return u4DataLen + sizeof(uint32);
}

bool CConsolePacketParse::MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData)
{
	if(u4ConnectID == 0)
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
	if(u4ConnectID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}

	//�����ǲ��Դ��룬ר�Ŵ���Ϊ�����������ݰ�
	if(NULL == pCurrMessage || NULL == pMessageBlockManager)
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
				m_objCurrBody.WriteStream(pCurrMessage->rd_ptr() + 1, pCurrMessage->length() - 1);
			}

			m_blIsHead = true;

			//û���ҵ���β����Ҫ������������
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//���յ����������ݰ�
			uint32 u4PacketLen = u4Pos - 1;

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

		uint32 u4Pos  = 1;
		bool   blFind = false;
		if(m_objCurrBody.GetPacketLen() > 0)
		{
			//����л��壬�򿴿��Ƿ��н������
			for(u4Pos = 0; u4Pos < u4Data; u4Pos++)
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
			//�������������ݷ��뻺�壬�ȴ�������������
			m_objCurrBody.WriteStream(pData, u4Data);

			m_blIsHead = true;

			//û���ҵ���β����Ҫ������������
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//���յ����������ݰ�
			if(u4Pos > 0)
			{
				m_objCurrBody.WriteStream(pData, u4Pos);
			}
			
			m_u4PacketHead  = sizeof(uint32);
			m_u4HeadSrcSize = sizeof(uint32);
			m_u4PacketData  = m_objCurrBody.GetPacketLen();
			m_u4BodySrcSize = m_objCurrBody.GetPacketLen();

			//���ڴ��������һ����ͷ
			m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
			if(NULL == m_pmbHead)
			{
				return PACKET_GET_ERROR;
			}

			//��¼��������ת��Ϊ�����ֽ��򣬷����ͷ
			//uint32 u4NetPacketLen = ACE_HTONL(m_objCurrBody.GetPacketLen());
			uint32 u4NetPacketLen = m_objCurrBody.GetPacketLen();
			memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));

			//���ڴ������һ������
			m_pmbBody = pMessageBlockManager->Create(m_objCurrBody.GetPacketLen());
			if(NULL == m_pmbBody)
			{
				return PACKET_GET_ERROR;
			}

			//��ð�����ID
			ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));


			//�������ݷ������
			memcpy(m_pmbBody->wr_ptr(), (char*)m_objCurrBody.GetData(), m_objCurrBody.GetPacketLen());

			m_blIsHead = false;

			//����������ݴӳ����Ƴ�
			pCurrMessage->rd_ptr(u4Pos);
			return PACKET_GET_ENOUGTH;
		}

	}
}

bool CConsolePacketParse::Connect( uint32 u4ConnectID )
{
	//�������������ӽ������߼��������û������������д�κδ���
	//����false�������ӻ�Ͽ�
	if(u4ConnectID == 0)
	{
		//UDP���ݰ���û��u4ConnectID
	}

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



