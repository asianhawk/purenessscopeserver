#ifndef POSTSERVERLOGIC_H
#define POSTSERVERLOGIC_H

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"

//���ദ������͸����������߼�

#define SEND_BUFF_SIZE  MAX_BUFF_1024      //���ͻ����С
#define RECV_BUFF_SIZE  MAX_BUFF_1024      //���ջ����С

class CPostServerData : public IClientMessage
{
public:
	CPostServerData() 
	{ 
		m_pServerObject      = NULL;
		m_u4ServerID         = 0;
		m_u4ConnectID        = 0;
		m_u2RecvBuffLength   = 0;
		m_u2SendBuffLength   = 0;
	};

	~CPostServerData() 
	{
		if(m_u4ServerID != 0)
		{
			Close(S2S_INNEED_CALLBACK);
		}
	};

	bool RecvData(ACE_Message_Block* mbRecv,  _ClientIPInfo objServerIPInfo)
	{
		//�жϷ������ݿ��Ƿ�С��0���߳�����󻺳��С
		if(mbRecv->length() <= 0 || mbRecv->length() >= RECV_BUFF_SIZE)
		{
			OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", mbRecv->length()));
			return false;
		}

		//ͳһ���뻺�壬���л����и������
		ACE_OS::memcpy(&m_szRecvBuffData[m_u2RecvBuffLength], mbRecv->rd_ptr(), mbRecv->length());
		m_u2RecvBuffLength += mbRecv->length();

		//�жϻ����Ƿ��������
		if(m_u2RecvBuffLength < 4)
		{
			//����������ͷ�����룬������
			return true;
		}
		else
		{
			while(true)
			{
				//��������ֽ���8�ֽڣ�����Ϊ��һ��������
				//�����������㷨

				if(m_u2RecvBuffLength < 8)
				{
					//���ݰ�û��ȫ����������
					return true;
				}
				else
				{
					//���ݰ��Ѿ���ȫ.���͸��ͻ�������
					//OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", mbRecv->length()));
					if(NULL != m_pServerObject &&  mbRecv->length() > 0)
					{
						uint16 u2RetCommand = 0x1010;
						uint32 u4PacketLength = 8;      //�̶�����8�ֽ�
						char* pData = new char[u4PacketLength + sizeof(uint32)];
						ACE_OS::memcpy(pData, (char* )&u4PacketLength, sizeof(uint32));
						ACE_OS::memcpy(&pData[sizeof(uint32)], m_szRecvBuffData, u4PacketLength);

						uint32 u4SendLength = u4PacketLength + sizeof(uint32);
						m_pServerObject->GetConnectManager()->PostMessage(m_u4ConnectID, pData, u4SendLength, SENDMESSAGE_JAMPNOMAL, u2RetCommand, PACKET_SEND_IMMEDIATLY, PACKET_IS_FRAMEWORK_RECYC);
						OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData](%d)Send Data(%d) OK.\n", m_u4ConnectID, u4SendLength));

						//����ǰ�ƣ�����ж����������
						m_u2RecvBuffLength -= u4PacketLength;
						if(m_u2RecvBuffLength > 0)
						{
							ACE_OS::memcpy(&m_szRecvBuffData, (char* )&m_szRecvBuffData[m_u2RecvBuffLength], u4PacketLength);
						}
					}
				}
			}

		}

		return true;
	};

	//��������
	bool SendData(const char* pData, uint32 u4Size)
	{
		//�����������ͳ���
		if(m_u2SendBuffLength + u4Size >= MAX_BUFF_1024 || u4Size >= SEND_BUFF_SIZE)
		{
			OUR_DEBUG((LM_ERROR, "[CPostServerData::SendData](%d) send buff full.\n", m_u4ServerID));
			return false;
		}

		//�õ���ǰ����״̬
		if(SERVER_CONNECT_OK == m_pServerObject->GetClientManager()->GetConnectState(m_u4ServerID))
		{
			//�����ݷ��뻺�壬��֮ǰ������(����еĻ�)һ����
			ACE_OS::memcpy(&m_szSendBuffData[m_u2SendBuffLength], pData, u4Size);
			m_u2SendBuffLength += u4Size;

			//��������
			OUR_DEBUG((LM_ERROR, "[CPostServerData::SendData](%d) Send [%d] Begin.", m_u4ServerID, m_u2SendBuffLength));
			if(false == m_pServerObject->GetClientManager()->SendData((int)m_u4ServerID, m_szSendBuffData, m_u2SendBuffLength, false))
			{
				//����ʧ�ܣ�����
				OUR_DEBUG((LM_ERROR, "[CPostServerData::SendData](%d) Send [%d] End 1.", m_u4ServerID, m_u2SendBuffLength));
				return false;
			}
			else
			{
				//���ͳɹ���������
				OUR_DEBUG((LM_ERROR, "[CPostServerData::SendData](%d) Send [%d] End.", m_u4ServerID, m_u2SendBuffLength));
				m_u2SendBuffLength = 0;
				return true;
			}
		}
		else
		{
			OUR_DEBUG((LM_ERROR, "[CPostServerData::SendData](%d) Send Buffer.", m_u4ServerID));
			//������δ�����ã����뻺�壬�����ӽ������Զ��ص���
			ACE_OS::memcpy(&m_szSendBuffData[m_u2SendBuffLength], pData, u4Size);
			m_u2SendBuffLength += u4Size;
			return true;
		}
	}

	//�ر�Զ�̷���
	bool Close(EM_s2s ems2s)
	{
		m_pServerObject->GetClientManager()->Close(m_u4ServerID, ems2s);
		m_u4ServerID = 0;
		return true;
	}

	bool ConnectError(int nError,  _ClientIPInfo objServerIPInfo)
	{
		OUR_DEBUG((LM_ERROR, "[CPostServerData::ConnectError]Get Error(%d).\n", nError));
		return true;
	};

	void ReConnect(int nServerID)
	{
		//���������ɹ��ӿ�
		//m_u4ServerID = (uint32)nServerID;
		OUR_DEBUG((LM_ERROR, "[CPostServerData::ReConnect]ReConnect(%d).\n", m_u4ServerID));
		if(m_u2SendBuffLength > 0)
		{
			//��������
			if(true == m_pServerObject->GetClientManager()->SendData((int)m_u4ServerID, m_szSendBuffData, m_u2SendBuffLength, false))
			{
				//���ͳɹ���������
				m_u2SendBuffLength = 0;
			}
		}
	}

	CServerObject* GetServerObject()
	{
		return m_pServerObject; 
	}

	void SetServerObject(CServerObject* pServerObject)
	{
		m_pServerObject = pServerObject;
	}

	void SetConnectID(uint32 u4ConnectID)
	{
		m_u4ConnectID = u4ConnectID;
	}

	void SetServerID(uint32 u4ServerID)
	{
		m_u4ServerID = u4ServerID;
	}

	uint32 GetServerID()
	{
		return m_u4ServerID;
	}

private:
	ACE_Recursive_Thread_Mutex m_ThreadLock;
	CServerObject*             m_pServerObject;
	uint32                     m_u4ServerID;
	uint32                     m_u4ConnectID;
	char                       m_szRecvBuffData[RECV_BUFF_SIZE];  //���ջ����
	char                       m_szSendBuffData[SEND_BUFF_SIZE];  //���ͻ���� 
	uint16                     m_u2RecvBuffLength;                //���ջ��峤��
	uint16                     m_u2SendBuffLength;                //���ͻ��峤��
};

#endif
