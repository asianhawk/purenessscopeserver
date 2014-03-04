//����PurenessScopeServer��ר�ò������
//add by freeeyes
//2013-12-24

#include "BaseCommand.h"
#include "IObject.h"

static const char *g_szDesc      = "�Զ����ģ��";       //ģ�����������
static const char *g_szName      = "AutoTest";           //ģ�������
static const char *g_szModuleKey = "AutoTest";           //ģ���Key

#ifdef WIN32
#ifdef TEST_TCP_BUILD_DLL
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif
#else
#define DECLDIR
#endif

extern "C"
{
	DECLDIR int LoadModuleData(CServerObject* pServerObject);
	DECLDIR int UnLoadModuleData();
	DECLDIR const char* GetDesc();
	DECLDIR const char* GetName();
	DECLDIR const char* GetModuleKey();
	DECLDIR int DoModuleMessage(uint16 u2CommandID, IBuffPacket* pBuffPacket, IBuffPacket* pReturnBuffPacket);
}

static CBaseCommand g_BaseCommand;
CServerObject*      g_pServerObject = NULL;

int LoadModuleData(CServerObject* pServerObject)
{
	g_pServerObject = pServerObject;
	OUR_DEBUG((LM_INFO, "[Base LoadModuleData] Begin.\n"));
	if(g_pServerObject != NULL)
	{
		g_BaseCommand.SetServerObject(pServerObject);	
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[Base LoadModuleData] pServerObject is NULL.\n"));
	}

	IMessageManager* pMessageManager = g_pServerObject->GetMessageManager();
	if(NULL != pMessageManager)
	{
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_HEAD, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_NOHEAD, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_HEADBUFF, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_NOHEADBUFF, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_LOGDATA, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(COMMAND_AUTOTEST_WORKTIMEOUT, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(CLIENT_LINK_CONNECT, &g_BaseCommand, g_szName);
		pMessageManager->AddClientCommand(CLIENT_LINK_CDISCONNET, &g_BaseCommand, g_szName);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[Base LoadModuleData] pMessageManager = NULL.\n"));
	}

	//��ʾ��ܹ����߳�ID
	OUR_DEBUG((LM_INFO, "[Base LoadModuleData] *********************************.\n"));

	uint32 u4WorkThread = pMessageManager->GetWorkThreadCount();
	OUR_DEBUG((LM_INFO, "[Base LoadModuleData] WorkThreadCount=%d.\n", u4WorkThread));

	for(uint32 u4Index = 0; u4Index < u4WorkThread; u4Index++)
	{
		OUR_DEBUG((LM_INFO, "[Base LoadModuleData] WorkThreadID=%d.\n", pMessageManager->GetWorkThreadByIndex(u4Index)));
	}

	OUR_DEBUG((LM_INFO, "[Base LoadModuleData] *********************************.\n"));

	OUR_DEBUG((LM_INFO, "[Base LoadModuleData] End.\n"));

	return 0;
}

int UnLoadModuleData()
{
	OUR_DEBUG((LM_INFO, "[Base UnLoadModuleData] Begin.\n"));
	if(g_pServerObject != NULL)
	{
		IMessageManager* pMessageManager = g_pServerObject->GetMessageManager();
		if(NULL != pMessageManager)
		{
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_HEAD, &g_BaseCommand);
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_NOHEAD, &g_BaseCommand);
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_HEADBUFF, &g_BaseCommand);
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_NOHEADBUFF, &g_BaseCommand);
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_LOGDATA, &g_BaseCommand);
			pMessageManager->DelClientCommand(COMMAND_AUTOTEST_WORKTIMEOUT, &g_BaseCommand);
			pMessageManager->DelClientCommand(CLIENT_LINK_CONNECT, &g_BaseCommand);
			pMessageManager->DelClientCommand(CLIENT_LINK_CDISCONNET, &g_BaseCommand);
			pMessageManager = NULL;
		}
	}
	OUR_DEBUG((LM_INFO, "[Base UnLoadModuleData] End.\n"));
	return 0;
}

const char* GetDesc()
{
	return g_szDesc;
}

const char* GetName()
{
	return g_szName;
}

const char* GetModuleKey()
{
	return g_szModuleKey;
}

//����ģ���ĵ��ýӿ�
int DoModuleMessage(uint16 u2CommandID, IBuffPacket* pBuffPacket, IBuffPacket* pReturnBuffPacket)
{
	return 0;
}

