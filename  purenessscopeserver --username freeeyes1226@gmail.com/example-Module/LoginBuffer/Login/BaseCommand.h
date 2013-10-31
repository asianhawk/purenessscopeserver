#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"
#include "UserValidManager.h"

#include <string>
#include <map>

//����ͻ�������(TCP)
#define COMMAND_LOGIN                  0x2100     //��½
#define COMMAND_LOGOUT                 0x2101     //�˳� 
#define COMMAND_USERINFO               0x2102     //���������Ϣ  
#define COMMAND_RETURN_LOGIN           0xe100     //��½Ӧ�� 
#define COMMAND_RETURN_LOGOUT          0xe101     //�ǳ�Ӧ��
#define COMMAND_RETURN_USERINFO        0xe102     //���������Ϣ  

//���峣�ñ���
#define LOGIN_SUCCESS            0
#define LOGIN_FAIL_NOEXIST       1
#define LOGIN_FAIL_ONLINE        2
#define LOGIN_FAIL_PASSWORD      3

#define OP_OK                    0
#define OP_FAIL                  1

using namespace std;

typedef ACE_Singleton<CUserValidManager, ACE_Null_Mutex> App_UserValidManager;

class CPostServerData : public IClientMessage
{
public:
	CPostServerData() 
	{ 
		m_pServerObject = NULL;
	};

	~CPostServerData() {};

	bool RecvData(ACE_Message_Block* mbRecv)
	{
		//�жϷ������ݿ��Ƿ�С��0
		uint32 u4SendPacketSize = (uint32)mbRecv->length();
		if(u4SendPacketSize <= 0)
		{
			OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", u4SendPacketSize));
		}

		OUR_DEBUG((LM_INFO, "[CPostServerData::RecvData]Get Data(%d).\n", u4SendPacketSize));
		if(NULL != m_pServerObject)
		{
			//������õ�����
			uint16 u2RetCommand = 0x1010;
			char* pData = new char[u4SendPacketSize];
			ACE_OS::memcpy(pData, mbRecv->rd_ptr(), u4SendPacketSize);

			//������õ����ݿ�
			//4�ֽڰ���+2�ֽ����Ƴ���+����+ConnectID
			uint32 u4PacketSize = 0;
			int    nPos = 0;
			ACE_OS::memcpy(&u4PacketSize, (char* )&pData[nPos], sizeof(uint32));
			nPos += sizeof(uint32);
			uint16 u2UserName = 0;
			ACE_OS::memcpy(&u2UserName, (char* )&pData[nPos], sizeof(uint16));
			nPos += sizeof(uint16);
			char* pUserName = new char[u2UserName + 1];
			ACE_OS::memset(pUserName, 0, u2UserName + 1);
			ACE_OS::memcpy(pUserName, (char* )&pData[nPos], u2UserName);
			nPos += u2UserName;
			uint16 u2UserPass = 0;
			ACE_OS::memcpy(&u2UserPass, (char* )&pData[nPos], sizeof(uint16));
			nPos += sizeof(uint16);
			char* pUserPass = new char[u2UserPass + 1];
			ACE_OS::memset(pUserPass, 0, u2UserPass + 1);
			ACE_OS::memcpy(pUserPass, (char* )&pData[nPos], u2UserPass);
			nPos += u2UserPass;
			uint8 u1Ret = 0;
			ACE_OS::memcpy(&u1Ret, (char* )&pData[nPos], sizeof(uint8));
			nPos += sizeof(uint8);
			uint32 u4ConnectID = 0;
			ACE_OS::memcpy(&u4ConnectID, (char* )&pData[nPos], sizeof(uint32));
			nPos += sizeof(uint32);

			//���¼���һ�»���
			App_UserValidManager::instance()->GetFreeValid();

			uint32 u4Ret = LOGIN_SUCCESS;
			//��������һ��
			_UserValid* pUserValid = App_UserValidManager::instance()->GetUserValid(pUserName);
			if(NULL != pUserValid)
			{
				//�Ƚ��û������Ƿ���ȷ
				if(ACE_OS::strcmp(pUserValid->m_szUserPass, pUserPass) == 0)
				{
					pUserValid->m_blOnline = true;
					pUserValid->m_u4LoginCount++;
					u4Ret = LOGIN_SUCCESS;
				}
				else
				{
					u4Ret = LOGIN_FAIL_PASSWORD;
				}
			}
			else
			{
				u4Ret = LOGIN_FAIL_NOEXIST;
			}

			SAFE_DELETE_ARRAY(pUserPass);
			SAFE_DELETE_ARRAY(pUserName);
			SAFE_DELETE_ARRAY(pData);

			IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();
			uint16 u2PostCommandID = COMMAND_RETURN_LOGIN;

			//������֤���
			(*pResponsesPacket) << (uint16)u2PostCommandID;   //ƴ��Ӧ������ID
			(*pResponsesPacket) << (uint32)u4Ret;

			if(NULL != m_pServerObject->GetConnectManager())
			{
				//����ȫ������
				m_pServerObject->GetConnectManager()->PostMessage(u4ConnectID, pResponsesPacket, SENDMESSAGE_NOMAL, u2PostCommandID, true);
			}
			else
			{
				OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
			}
		}

		return true;
	};

	bool ConnectError(int nError)
	{
		OUR_DEBUG((LM_ERROR, "[CPostServerData::ConnectError]Get Error(%d).\n", nError));
		return true;
	};

	void SetServerObject(CServerObject* pServerObject)
	{
		m_pServerObject = pServerObject;
	}

private:
	CServerObject* m_pServerObject;
};

class CBaseCommand : public CClientCommand
{
public:
  CBaseCommand(void);
  ~CBaseCommand(void);

  void InitUserList();
  void ClearUserList();
  int DoMessage(IMessage* pMessage, bool& bDeleteFlag);
  void SetServerObject(CServerObject* pServerObject);

private:
	void Do_User_Login(IMessage* pMessage);        //�����½
	void Do_User_Logout(IMessage* pMessage);       //����ǳ�
	void Do_User_Info(IMessage* pMessage);         //����ļ��б�

private:
  CServerObject*    m_pServerObject;
  CPostServerData*  m_pPostServerData;
};


