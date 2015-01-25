// ֧�ֹ����ڴ滺��ĵ�½���������
// �˲����ʾ�����鹲���ڴ棬�û���֤���û���Ϣ��
// �ڲ�ͬģʽ�µĻ���ӿڣ��ṩ�Թ����ڴ�Ϊ�����������ȳ����νṹ
//add by freeeyes
//2013-10-25

#include "BaseCommand.h"
#include "IObject.h"

static const char *g_szDesc      = "Login������";       //ģ�����������
static const char *g_szName      = "Login������";       //ģ�������
static const char *g_szModuleKey = "BaseLogin";           //ģ���Key

#ifdef WIN32
#ifdef TEST_TCP_BUILD_LOGIN_DLL
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
  DECLDIR bool GetModuleState(uint32& u4ErrorID);

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
    pMessageManager->AddClientCommand(COMMAND_LOGIN, &g_BaseCommand, g_szName);
	pMessageManager->AddClientCommand(COMMAND_LOGOUT, &g_BaseCommand, g_szName);
	pMessageManager->AddClientCommand(COMMAND_USERINFO, &g_BaseCommand, g_szName);
	pMessageManager->AddClientCommand(COMMAND_SET_USERINFO, &g_BaseCommand, g_szName);
    pMessageManager->AddClientCommand(CLIENT_LINK_CONNECT, &g_BaseCommand, g_szName);
    pMessageManager->AddClientCommand(CLIENT_LINK_CDISCONNET, &g_BaseCommand, g_szName);
  }
  else
  {
    OUR_DEBUG((LM_INFO, "[Base LoadModuleData] pMessageManager = NULL.\n"));
  }		

  g_BaseCommand.InitUserList();
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
      pMessageManager->DelClientCommand(COMMAND_LOGIN, &g_BaseCommand);
	  pMessageManager->DelClientCommand(COMMAND_LOGOUT, &g_BaseCommand);
	  pMessageManager->DelClientCommand(COMMAND_USERINFO, &g_BaseCommand);
	  pMessageManager->DelClientCommand(COMMAND_SET_USERINFO, &g_BaseCommand);
      pMessageManager->DelClientCommand(CLIENT_LINK_CONNECT, &g_BaseCommand);
      pMessageManager->DelClientCommand(CLIENT_LINK_CDISCONNET, &g_BaseCommand);
      pMessageManager = NULL;
    }
  }

  g_BaseCommand.ClearUserList();
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

//�������ʹ�ã����ڿ�ܶ�ʱѲ����״̬
//Ĭ�����ﷵ��true���������Ҫ�Կ���ڲ�ϸ������ء�
//��������ʵ��֮�����ݴ���Ĳ�ͬ������false��ͬʱ������ErrorID
//��ܻ����������ã������ʼ���ָ��������
bool GetModuleState(uint32& u4ErrorID)
{
	return true;
}


