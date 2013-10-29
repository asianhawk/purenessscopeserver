#include "BaseCommand.h"

CBaseCommand::CBaseCommand(void)
{
  m_pServerObject = NULL;
}

CBaseCommand::~CBaseCommand(void)
{
}

void CBaseCommand::SetServerObject(CServerObject* pServerObject)
{
  m_pServerObject = pServerObject;
}

int CBaseCommand::DoMessage(IMessage* pMessage, bool& bDeleteFlag)
{
  __ENTER_FUNCTION;

  if(m_pServerObject == NULL)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] m_pServerObject is NULL.\n"));
    return -1;
  }

  if(pMessage == NULL)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] pMessage is NULL.\n"));
    return -1;
  }

  //�������ӽ�����Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CONNECT)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CONNECT OK.\n"));
    return 0;
  }

  //�������ӶϿ���Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CDISCONNET)
  {
    OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CDISCONNET OK.\n"));
    return 0;
  }

  //�û���½
  if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_LOGIN)
  {
	  Do_User_Login(pMessage);
  }
  else if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_LOGOUT)
  {
	  Do_User_Logout(pMessage);
  }
  else if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_USERINFO)
  {
	  Do_User_Info(pMessage);
  }

  return 0;

  __LEAVE_FUNCTION_WITHRETURN(0);
}

void CBaseCommand::Do_User_Login( IMessage* pMessage )
{
	uint32     u4PacketLen  = 0;
	uint16     u2CommandID  = 0;
	VCHARS_STR strUserName;
	VCHARS_STR strUserPass;
	char szUserName[MAX_BUFF_100] = {'\0'};
	char szUserPass[MAX_BUFF_100] = {'\0'};

	IBuffPacket* pBodyPacket = m_pServerObject->GetPacketManager()->Create();
	if(NULL == pBodyPacket)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] pBodyPacket is NULL.\n"));
		return;
	}

	_PacketInfo BodyPacket;
	pMessage->GetPacketBody(BodyPacket);

	pBodyPacket->WriteStream(BodyPacket.m_pData, BodyPacket.m_nDataLen);

	(*pBodyPacket) >> u2CommandID;
	(*pBodyPacket) >> strUserName;
	(*pBodyPacket) >> strUserPass;

	//����������ת��Ϊ�ַ���
	ACE_OS::memcpy(szUserName, strUserName.text, strUserName.u1Len);
	ACE_OS::memcpy(szUserPass, strUserPass.text, strUserPass.u1Len);
	szUserName[strUserName.u1Len] = '\0';
	szUserPass[strUserPass.u1Len] = '\0';

	//�����������û��Լ����߼��ж�����
	IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();
	uint16 u2PostCommandID = COMMAND_RETURN_LOGIN;

	uint32 u4Ret = LOGIN_SUCCESS;
	_UserValid* pUserValid = m_UserValidManager.GetUserValid(szUserName);
	if(NULL != pUserValid)
	{
		//�Ƚ��û������Ƿ���ȷ
		if(ACE_OS::strcmp(pUserValid->m_szUserPass, szUserPass) == 0)
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
		//���¼���һ�»���
		m_UserValidManager.GetFreeValid();

		//��������һ��
		_UserValid* pUserValid = m_UserValidManager.GetUserValid(szUserName);
		if(NULL != pUserValid)
		{
			//�Ƚ��û������Ƿ���ȷ
			if(ACE_OS::strcmp(pUserValid->m_szUserPass, szUserPass) == 0)
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
	}

	//������֤���
	(*pResponsesPacket) << (uint16)u2PostCommandID;   //ƴ��Ӧ������ID
	(*pResponsesPacket) << (uint32)u4Ret;

	m_pServerObject->GetPacketManager()->Delete(pBodyPacket);

	if(NULL != m_pServerObject->GetConnectManager())
	{
		//����ȫ������
		m_pServerObject->GetConnectManager()->PostMessage(pMessage->GetMessageBase()->m_u4ConnectID, pResponsesPacket, SENDMESSAGE_NOMAL, u2PostCommandID, true);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
	}
}

void CBaseCommand::Do_User_Logout( IMessage* pMessage )
{
	uint32     u4PacketLen  = 0;
	uint16     u2CommandID  = 0;
	VCHARS_STR strUserName;
	char szUserName[MAX_BUFF_100] = {'\0'};

	IBuffPacket* pBodyPacket = m_pServerObject->GetPacketManager()->Create();
	if(NULL == pBodyPacket)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] pBodyPacket is NULL.\n"));
		return;
	}

	_PacketInfo BodyPacket;
	pMessage->GetPacketBody(BodyPacket);

	pBodyPacket->WriteStream(BodyPacket.m_pData, BodyPacket.m_nDataLen);

	(*pBodyPacket) >> u2CommandID;
	(*pBodyPacket) >> strUserName;

	//����������ת��Ϊ�ַ���
	ACE_OS::memcpy(szUserName, strUserName.text, strUserName.u1Len);

	//�����������û��Լ����߼��ж�����
	IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();
	uint16 u2PostCommandID = COMMAND_RETURN_LOGOUT;

	uint32 u4Ret = LOGIN_SUCCESS;
	_UserValid* pUserValid = m_UserValidManager.GetUserValid(szUserName);
	if(NULL != pUserValid)
	{
		//�Ƚ��û������Ƿ���ȷ
		pUserValid->m_blOnline = false;
		u4Ret = LOGIN_SUCCESS;
	}
	else
	{
		u4Ret = LOGIN_FAIL_NOEXIST;
	}

	//������֤���
	(*pResponsesPacket) << (uint16)u2PostCommandID;   //ƴ��Ӧ������ID
	(*pResponsesPacket) << (uint32)u4Ret;

	m_pServerObject->GetPacketManager()->Delete(pBodyPacket);

	if(NULL != m_pServerObject->GetConnectManager())
	{
		//����ȫ������
		m_pServerObject->GetConnectManager()->PostMessage(pMessage->GetMessageBase()->m_u4ConnectID, pResponsesPacket, SENDMESSAGE_NOMAL, u2PostCommandID, true);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
	}
}

void CBaseCommand::Do_User_Info( IMessage* pMessage )
{
	return;
}

void CBaseCommand::InitUserList()
{
	m_UserValidManager.Init();
}

void CBaseCommand::ClearUserList()
{
	m_UserValidManager.Close();
}

