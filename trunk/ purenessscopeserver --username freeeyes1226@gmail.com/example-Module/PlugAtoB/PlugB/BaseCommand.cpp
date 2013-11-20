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
  //__ENTER_FUNCTION是自动给你的函数追加try的宏，用于捕捉错误，你可以使用__THROW_FUNCTION()抛出你的错误
  //比如__THROW_FUNCTION("hello"); 它会自动在Log文件夹下的assert.log记录文件名，函数名，行数，描述。
  //推荐你使用这个宏，记住一定要在函数的结束使用__LEAVE_FUNCTION或者__LEAVE_FUNCTION_WITHRETURN与之配对。
  //__LEAVE_FUNCTION_WITHRETURN和__LEAVE_FUNCTION不同是前者可以支持函数退出的时候返回一个变量
  //比如这个函数是返回一个int
  //那么末尾就是__LEAVE_FUNCTION_WITHRETURN(0); 0就是返回的int，当然，也支持别的类型变量，具体看你自己的函数。
  //如果函数不返回任何变量，你可以使用__LEAVE_FUNCTION即可。
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

	//写入日志文件
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

	sprintf_safe(szLog, MAX_BUFF_300, "工作线程ID:%d,内容:%s.\n", u4WorkThreadID, szText);

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

	//写入日志文件
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
	sprintf_safe(szLog, MAX_BUFF_300, "初始化工作线程个数:%d.\n", u4ThreadCount);
	fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);

	for(uint32 nIndex = 0; nIndex < u4ThreadCount; nIndex++)
	{
		(*pBuffPacket) >> u4ThreadID;
		sprintf_safe(szLog, MAX_BUFF_300, "初始化工作线程ID:%d.\n", u4ThreadID);
		fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), pFile);
	}

	ACE_OS::fclose(pFile);

	(*pReturnBuffPacket) << (uint32)MODULE_RETURN_SUCCESS;
}
