#include "BaseCommand.h"

CBaseCommand::CBaseCommand(void)
{
	m_pServerObject = NULL;
	m_nCount        = 0;
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

	//处理链接建立信息
	if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CONNECT)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CONNECT OK.\n"));
	}

	//处理链接断开信息
	if(pMessage->GetMessageBase()->m_u2Cmd == CLIENT_LINK_CDISCONNET)
	{
		OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] CLIENT_LINK_CDISCONNET OK.\n"));
	}

	//处理正常信息
	if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_BASE)
	{
		uint32     u4PacketLen = 0;
		uint16     u2CommandID = 0;
		VCHARS_STR strsName;
		string     strName;

		IBuffPacket* pBodyPacket = m_pServerObject->GetPacketManager()->Create();
		if(NULL == pBodyPacket)
		{
			OUR_DEBUG((LM_ERROR, "[CBaseCommand::DoMessage] pBodyPacket is NULL.\n"));
			return -1;
		}
		
		_PacketInfo BodyPacket;
		pMessage->GetPacketBody(BodyPacket);

		pBodyPacket->WriteStream(BodyPacket.m_pData, BodyPacket.m_nDataLen);

		(*pBodyPacket) >> u2CommandID;
		(*pBodyPacket) >> strsName;
		strName.assign(strsName.text, strsName.u1Len);

		m_pServerObject->GetPacketManager()->Delete(pBodyPacket);

		//ACE_Time_Value tvSleep(10);
		//ACE_OS::sleep(tvSleep);

		if(NULL != m_pServerObject->GetLogManager())
		{
			_ClientIPInfo ClientIPInfo = m_pServerObject->GetConnectManager()->GetClientIPInfo(pMessage->GetMessageBase()->m_u4ConnectID);
			m_pServerObject->GetLogManager()->WriteLog(LOG_SYSTEM,  "[CBaseCommand::DoMessage] Get CommandID = %d, IP=%s, Port=%d", u2CommandID, ClientIPInfo.m_szClientIP, ClientIPInfo.m_nPort);
		}
		else
		{
			OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pLogManager = NULL"));
		}

		//测试返回数据(TCP)
		IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();

		VCHARS_STR strsPass;
		uint16 u2PostCommandID = 0xe102;
		string strUserName     = "aaaac";
		string strUserPass     = "aaa";

		strsName.text = strUserName.c_str();
		strsName.u1Len = (uint8)strUserName.length();

		strsPass.text = strUserPass.c_str();
		strsPass.u1Len = (uint8)strUserPass.length();

		(*pResponsesPacket) << u2PostCommandID;
		(*pResponsesPacket) << strsName;
		(*pResponsesPacket) << strsPass;

		if(NULL != m_pServerObject->GetConnectManager())
		{
			m_pServerObject->GetConnectManager()->SendMessage(pMessage->GetMessageBase()->m_u4ConnectID, pResponsesPacket);
		}
		else
		{
			OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
		}

	}


	return 0;
}


