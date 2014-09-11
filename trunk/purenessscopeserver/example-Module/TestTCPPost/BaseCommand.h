#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"

#include <string>

//����ͻ�������(TCP)
#define COMMAND_BASE            0x1000
#define COMMAND_RETURN_ALIVE    0xf001

using namespace std;

class CPostServerData : public IClientMessage
{
public:
	CPostServerData() 
	{ 
		m_pServerObject      = NULL;
		m_u4ConnectID        = 0;
		m_u2BufferDataLength = 0;
	};

	~CPostServerData() {};

	bool RecvData(ACE_Message_Block* mbRecv,  _ClientIPInfo objServerIPInfo)
	{
		//�жϷ������ݿ��Ƿ�С��0
		if(mbRecv->length() <= 0)
		{
			OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", mbRecv->length()));
			return false;
		}

		//ͳһ���뻺�壬���л����и������
		ACE_OS::memcpy(&m_szBufferData[m_u2BufferDataLength], mbRecv->rd_ptr(), mbRecv->length());
		m_u2BufferDataLength += mbRecv->length();

		//�жϻ����Ƿ��������
		if(m_u2BufferDataLength < 4)
		{
			//����������ͷ�����룬������
			return true;
		}
		else
		{
			while(true)
			{
				//�а����õ�������
				uint32 u4PacketLength = 0;
				ACE_OS::memcpy(&u4PacketLength, m_szBufferData, sizeof(uint32));

				uint16 u2SendPacketLength = (uint16)u4PacketLength + (uint16)sizeof(uint32);
				if(u2SendPacketLength > m_u2BufferDataLength)
				{
					//���ݰ�û��ȫ����������
					return true;
				}
				else
				{
					//���ݰ��Ѿ���ȫ.��������
					OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", mbRecv->length()));
					if(NULL != m_pServerObject &&  mbRecv->length() > 0)
					{
						uint16 u2RetCommand = 0x1010;
						char* pData = new char[u4PacketLength + sizeof(uint32)];
						ACE_OS::memcpy(pData, m_szBufferData, u4PacketLength + sizeof(uint32));

						uint32 u4SendLength = u4PacketLength + sizeof(uint32);
						m_pServerObject->GetConnectManager()->PostMessage(m_u4ConnectID, pData, u4SendLength, SENDMESSAGE_JAMPNOMAL, u2RetCommand, PACKET_SEND_IMMEDIATLY, PACKET_IS_FRAMEWORK_RECYC);

						//����ǰ�ƣ�����ж����������
						m_u2BufferDataLength -= u2SendPacketLength;
						if(m_u2BufferDataLength > 0)
						{
							ACE_OS::memcpy(&m_szBufferData, (char* )&m_szBufferData[u2SendPacketLength], m_u2BufferDataLength);
						}
					}
				}
			}

		}

		return true;
	};

	bool ConnectError(int nError)
	{
		OUR_DEBUG((LM_ERROR, "[CPostServerData::ConnectError]Get Error(%d).\n", nError));
		return true;
	};

	void ReConnect(int nServerID)
	{
		//���������ɹ��ӿ�
		OUR_DEBUG((LM_ERROR, "[CPostServerData::ReConnect]ReConnect(%d).\n", nServerID));
	}

	void SetServerObject(CServerObject* pServerObject)
	{
		m_pServerObject = pServerObject;
	}

	void SetConnectID(uint32 u4ConnectID)
	{
		m_u4ConnectID = u4ConnectID;
	}

private:
	CServerObject* m_pServerObject;
	uint32         m_u4ConnectID;
	char           m_szBufferData[MAX_BUFF_1024];    //��������
	uint16         m_u2BufferDataLength;             //���峤��
};

class CBaseCommand : public CClientCommand
{
public:
	CBaseCommand(void);
	~CBaseCommand(void);

	int DoMessage(IMessage* pMessage, bool& bDeleteFlag);
	void SetServerObject(CServerObject* pServerObject);
	void InitServer();

private:
	CServerObject*   m_pServerObject;
	int              m_nCount;
	CPostServerData* m_pPostServerData1;  //�м���������Ͷ���
};
