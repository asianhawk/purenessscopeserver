// ֧�ֹ����ڴ滺��ĵ�½���������
// �˲����ʾ�����鹲���ڴ棬�û���֤���û���Ϣ��
// �ڲ�ͬģʽ�µĻ���ӿڣ��ṩ�Թ����ڴ�Ϊ�����������ȳ����νṹ
//add by freeeyes
//2013-10-25

#include "BaseCommand.h"
#include "IObject.h"

static const char *g_szDesc      = "Linux��Ϣ����";       //ģ�����������
static const char *g_szName      = "Linux��Ϣ����";       //ģ�������
static const char *g_szModuleKey = "LinuxQueue";          //ģ���Key

#ifdef WIN32
#ifdef TEST_TCP_BUILD_QUEUE_DLL
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
		pMessageManager->AddClientCommand(COMMAND_LINUXQUEUE, &g_BaseCommand, g_szName);
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[Base LoadModuleData] pMessageManager = NULL.\n"));
	}		

	bool blInit = g_BaseCommand.InitQueue();
	if(false == blInit)
	{
		OUR_DEBUG((LM_INFO, "[Base LoadModuleData] InitQueue is fail,this mudule is not support Windows!!!\n"));
	}

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
			pMessageManager->DelClientCommand(COMMAND_LINUXQUEUE, &g_BaseCommand);
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

