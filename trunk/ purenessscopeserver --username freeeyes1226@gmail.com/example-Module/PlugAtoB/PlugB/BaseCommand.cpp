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
	VCHARM_STR strUserText;
	char szText[MAX_BUFF_200] = {'\0'};
	char szLog[MAX_BUFF_300]  = {'\0'};

	(*pBuffPacket) >> u4WorkThreadID;
	(*pBuffPacket) >> strUserText;

	if(strUserText.u2Len >= 200)
	{
		(*pReturnBuffPacket) << (uint32)MODULE_RETURN_ERROR;
		return;
	}

	ACE_OS::memcpy(szText, strUserText.text, strUserText.u2Len);

	sprintf_safe(szLog, MAX_BUFF_300, "�����߳�ID:%d,����:%s.\n", u4WorkThreadID, szText);

	FILE* pFile = fopen("aaa.txt", "ab+");
	if(NULL == pFile)
	{
		(*pReturnBuffPacket) << (uint32)MODULE_RETURN_ERROR;
		return;
	}

	fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);

	ACE_OS::fclose(pFile);

	(*pReturnBuffPacket) << (uint32)MODULE_RETURN_SUCCESS;

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

	(*pBuffPacket) >> u4ThreadCount;
	sprintf_safe(szLog, MAX_BUFF_300, "��ʼ�������̸߳���:%d.\n", u4ThreadCount);
	fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);

	for(uint32 nIndex = 0; nIndex < u4ThreadCount; nIndex++)
	{
		(*pBuffPacket) >> u4ThreadID;
		sprintf_safe(szLog, MAX_BUFF_300, "��ʼ�������߳�ID:%d.\n", u4ThreadID);
		fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);
	}

	ACE_OS::fclose(pFile);

	(*pReturnBuffPacket) << (uint32)MODULE_RETURN_SUCCESS;
}
