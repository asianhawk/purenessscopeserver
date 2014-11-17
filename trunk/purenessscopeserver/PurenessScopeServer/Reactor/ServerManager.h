// Define.h
// ���ﶨ��Server����Ҫ�ӿڣ����������˿ڵȵȣ�
// ��Ȼ�Եã���ʹʲô��������Ҳ��һ�����ˡ�
// add by freeeyes
// 2008-12-23


#ifndef _SERVERMANAGER_H
#define _SERVERMANAGER_H

#include "define.h"
#include "MainConfig.h"
#include "ForbiddenIP.h"
#include "ConnectAccept.h"
#include "ConsoleAccept.h"
#include "AceReactorManager.h"
#include "MessageService.h"
#include "LoadModule.h"
#include "LogManager.h"
#include "FileLogger.h"
#include "IObject.h"
#include "BuffPacketManager.h"
#include "ClientReConnectManager.h"
#include "ReUDPManager.h"
#include "CommandAccount.h"
#include "ModuleMessageManager.h"
#include "ControlListen.h"

class Frame_Logging_Strategy;

class CServerManager
{
public:
	CServerManager(void);
	~CServerManager(void);

	bool Init();
	bool Start();
	bool Close();

private:
	//CConnectAcceptorManager m_ConnectAcceptorManager;    //��ͨ�ͻ�������
	ConnectConsoleAcceptor  m_ConnectConsoleAcceptor;    //��̨��������
	CFileLogger*            m_pFileLogger;               //��־����
	Frame_Logging_Strategy* m_pFrameLoggingStrategy;   //�������
};


typedef ACE_Singleton<CServerManager, ACE_Null_Mutex> App_ServerManager;

#endif
