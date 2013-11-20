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
  //__ENTER_FUNCTION���Զ�����ĺ���׷��try�ĺ꣬���ڲ�׽���������ʹ��__THROW_FUNCTION()�׳���Ĵ���
  //����__THROW_FUNCTION("hello"); �����Զ���Log�ļ����µ�assert.log��¼�ļ�������������������������
  //�Ƽ���ʹ������꣬��סһ��Ҫ�ں����Ľ���ʹ��__LEAVE_FUNCTION����__LEAVE_FUNCTION_WITHRETURN��֮��ԡ�
  //__LEAVE_FUNCTION_WITHRETURN��__LEAVE_FUNCTION��ͬ��ǰ�߿���֧�ֺ����˳���ʱ�򷵻�һ������
  //������������Ƿ���һ��int
  //��ôĩβ����__LEAVE_FUNCTION_WITHRETURN(0); 0���Ƿ��ص�int����Ȼ��Ҳ֧�ֱ�����ͱ��������忴���Լ��ĺ�����
  //��������������κα����������ʹ��__LEAVE_FUNCTION���ɡ�
  __ENTER_FUNCTION();

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

  //����������Ϣ
  if(pMessage->GetMessageBase()->m_u2Cmd == COMMAND_PLUGA)
  {
    Do_PlugA_DATA(pMessage);
  }

  return 0;

  __LEAVE_FUNCTION_WITHRETURN(0);
}

void CBaseCommand::Do_PlugA_DATA(IMessage* pMessage)
{
	uint32     u4PacketLen  = 0;
	uint16     u2CommandID  = 0;
	VCHARM_STR strUserText;

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
	(*pBodyPacket) >> strUserText;

	//������ݣ�������PlugB���ȥ����
	IBuffPacket* pModuleSendBuff = m_pServerObject->GetPacketManager()->Create();
	IBuffPacket* pModuleRecvBuff = m_pServerObject->GetPacketManager()->Create();

	(*pModuleSendBuff) << pMessage->GetMessageBase()->m_u4WorkThreadID;
	(*pModuleSendBuff) << strUserText;

	//��PlugB֪ͨ���ݴ���
	m_pServerObject->GetModuleMessageManager()->SendModuleMessage("PlugBģ��", MODULE_COMMAND_PLUGB, pModuleSendBuff, pModuleRecvBuff);

	uint32 u4Ret = 0;
	(*pModuleRecvBuff) >> u4Ret;

	m_pServerObject->GetPacketManager()->Delete(pModuleSendBuff);
	m_pServerObject->GetPacketManager()->Delete(pModuleRecvBuff);

	//�����յ������ݰ�
	m_pServerObject->GetPacketManager()->Delete(pBodyPacket);

	IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();
	uint16 u2PostCommandID = COMMAND_RETURN_PLUGA;

	//���ؽ��
	(*pResponsesPacket) << (uint16)u2PostCommandID;   //ƴ��Ӧ������ID
	(*pResponsesPacket) << (uint32)u4Ret;

	if(NULL != m_pServerObject->GetConnectManager())
	{
		//����ȫ������
		m_pServerObject->GetConnectManager()->PostMessage(pMessage->GetMessageBase()->m_u4ConnectID, pResponsesPacket, SENDMESSAGE_NOMAL, u2PostCommandID, true, true);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
		m_pServerObject->GetPacketManager()->Delete(pResponsesPacket);
	}

}

void CBaseCommand::PlugInit()
{
	if(m_pServerObject != NULL)
	{
		IBuffPacket* pModuleSendBuff = m_pServerObject->GetPacketManager()->Create();
		IBuffPacket* pModuleRecvBuff = m_pServerObject->GetPacketManager()->Create();

		uint32 u4WorkThreadCount = m_pServerObject->GetMessageManager()->GetWorkThreadCount();
		(*pModuleSendBuff) << u4WorkThreadCount;
		for(uint32 u4Index = 0; u4Index < u4WorkThreadCount; u4Index++)
		{
			(*pModuleSendBuff) << m_pServerObject->GetMessageManager()->GetWorkThreadByIndex(u4Index);
		}

		//��PlugB֪ͨ���ݴ���
		m_pServerObject->GetModuleMessageManager()->SendModuleMessage("PlugBģ��", MODULE_COMMAND_INIT, pModuleSendBuff, pModuleRecvBuff);

		m_pServerObject->GetPacketManager()->Delete(pModuleSendBuff);
		m_pServerObject->GetPacketManager()->Delete(pModuleRecvBuff);
	}
}
