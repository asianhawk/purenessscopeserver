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

  return 0;

  __LEAVE_FUNCTION_WITHRETURN(0);
}

void CBaseCommand::DoModuleMessage(IBuffPacket* pBuffPacket, IBuffPacket* pReturnBuffPacket)
{
	ACE_Guard<ACE_Recursive_Thread_Mutex> WGrard(m_ThreadWriteLock);

	if(NULL == pBuffPacket || NULL == pReturnBuffPacket)
	{
		return;
	}

	//д����־�ļ�
	uint32 u4WorkThreadID = 0;
	uint32 u4ConnectID    = 0;
	VCHARM_STR strUserText;
	char szText[MAX_BUFF_200] = {'\0'};
	char szLog[MAX_BUFF_300]  = {'\0'};

	(*pBuffPacket) >> u4WorkThreadID;
	(*pBuffPacket) >> u4ConnectID;
	(*pBuffPacket) >> strUserText;

	if(strUserText.u2Len >= 200)
	{
		(*pReturnBuffPacket) << (uint32)MODULE_RETURN_ERROR;
		return;
	}

	ACE_OS::memcpy(szText, strUserText.text, strUserText.u2Len);

	ACE_Date_Time dt;
	char szDateBuff[MAX_BUFF_200] = {'\0'};

	sprintf_safe(szDateBuff, MAX_BUFF_50, "%04d-%02d-%02d %02d:%02d:%02d,", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());

	sprintf_safe(szLog, MAX_BUFF_300, "[%s]�����߳�ID:%d,����:%s.\n", szDateBuff, u4WorkThreadID, szText);

	FILE* pFile = fopen("aaa.txt", "ab+");
	if(NULL == pFile)
	{
		(*pReturnBuffPacket) << (uint32)MODULE_RETURN_ERROR;
		return;
	}

	fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);

	ACE_OS::fclose(pFile);

	(*pReturnBuffPacket) << (uint32)MODULE_RETURN_SUCCESS;

	//���︽�Ӳ���ģ��B���뷢�����ݣ��ȴ����غ�Aһ���͸��ͻ��˵Ĳ��Դ��롣
	//���Կ�ܵķ��ͻ������
	//B��Ҫ���͵����ݷ��ڻ����У�Ȼ��ȴ�A�ټ������ݺ�һ�𷢳�ȥ��

	IBuffPacket* pResponsesPacket = m_pServerObject->GetPacketManager()->Create();
	uint16 u2PostCommandID = COMMAND_RETURN_PLUGA;

	//���ؽ��
	(*pResponsesPacket) << (uint32)(2 + 4 + 4);       //�ֶ�ƴ�ӷ��ذ� 
	(*pResponsesPacket) << (uint16)u2PostCommandID;   //ƴ��Ӧ������ID
	(*pResponsesPacket) << (uint32)1;

	if(NULL != m_pServerObject->GetConnectManager())
	{
		//�������ݵ���ܻ�����
		m_pServerObject->GetConnectManager()->PostMessage(u4ConnectID, pResponsesPacket, SENDMESSAGE_JAMPNOMAL, u2PostCommandID, PACKET_SEND_CACHE, PACKET_IS_FRAMEWORK_RECYC);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[CBaseCommand::DoMessage] m_pConnectManager = NULL"));
		m_pServerObject->GetPacketManager()->Delete(pResponsesPacket);
	}

}

void CBaseCommand::DoModuleInit( IBuffPacket* pBuffPacket, IBuffPacket* pReturnBuffPacket )
{
	if(NULL == pBuffPacket || NULL == pReturnBuffPacket)
	{
		return;
	}

	//д����־�ļ�
	uint32 u4ThreadCount = 0;
	uint32 u4ThreadID    = 0;
	char szLog[MAX_BUFF_300]  = {'\0'};

	FILE* pFile = fopen("aaa.txt", "ab+");
	if(NULL == pFile)
	{
		(*pReturnBuffPacket) << (uint32)MODULE_RETURN_ERROR;
		return;
	}

	ACE_Date_Time dt;
	char szDateBuff[MAX_BUFF_50] = {'\0'};

	sprintf_safe(szDateBuff, MAX_BUFF_50, "%04d-%02d-%02d %02d:%02d:%02d,", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());

	(*pBuffPacket) >> u4ThreadCount;
	sprintf_safe(szLog, MAX_BUFF_300, "[%s]��ʼ�������̸߳���:%d.\n", szDateBuff, u4ThreadCount);
	fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);

	for(uint32 nIndex = 0; nIndex < u4ThreadCount; nIndex++)
	{
		(*pBuffPacket) >> u4ThreadID;
		sprintf_safe(szLog, MAX_BUFF_300, "[%s]��ʼ�������߳�ID:%d.\n", szDateBuff, u4ThreadID);
		fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);
	}

	ACE_OS::fclose(pFile);

	(*pReturnBuffPacket) << (uint32)MODULE_RETURN_SUCCESS;
}
