// PurenessScopeServer.cpp : 定义控制台应用程序的入口点。
//
// 有时候的说，重新站起来绝对不是一件容易的事情，但是我现在必须站起来。要做好样的，freeeyes
// add by freeeyes, freebird92
// 2008-12-22(冬至)

#include "MainConfig.h"

#ifndef WIN32
//如果是Linux
#include "ServerManager.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	if(argc > 0)
	{
		OUR_DEBUG((LM_INFO, "[main]argc = %d.\n", argc));
		for(int i = 0; i < argc; i++)
		{
			OUR_DEBUG((LM_INFO, "[main]argc(%d) = %s.\n", argc, argv[i]));
		}
	}

	//第一步，读取配置文件
	if(!App_MainConfig::instance()->Init(MAINCONFIG))
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//判断是否是需要以服务的状态启动
	if(App_MainConfig::instance()->GetServerType() == 1)
	{
		ACE::daemonize();
	}

	//第二步，启动主服务器监控
	if(!App_ServerManager::instance()->Init())
	{
		OUR_DEBUG((LM_INFO, "[main]App_ServerManager::instance()->Init() error.\n"));
		getchar();
	}

	if(!App_ServerManager::instance()->Start())
	{
		OUR_DEBUG((LM_INFO, "[main]App_ServerManager::instance()->Start() error.\n"));
		getchar();
	}

	OUR_DEBUG((LM_INFO, "[main]Server Run is End.\n"));

	ACE_Time_Value tvSleep(2, 0);
	ACE_OS::sleep(tvSleep);

	OUR_DEBUG((LM_INFO, "[main]Server Exit.\n"));

	return 0;
}

#else
//如果是windows
#include "WindowsProcess.h"
#include "WindowsDump.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	//指定当前目录，防止访问文件失败
	TCHAR szFileName[MAX_PATH] = {0};
	GetModuleFileName(0, szFileName, MAX_PATH);
	LPTSTR pszEnd = _tcsrchr(szFileName, TEXT('\\'));

	if (pszEnd != 0)
	{
		pszEnd++;
		*pszEnd = 0;
	}
	SetCurrentDirectory(szFileName);

	//添加Dump文件
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);   

	//第一步，读取配置文件
	if(!App_MainConfig::instance()->Init(MAINCONFIG))
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//判断是否是需要以服务的状态启动
	if(App_MainConfig::instance()->GetServerType() == 1)
	{
		//以服务状态启动
		//首先看有没有配置启动windows服务
		App_Process::instance()->run(argc, argv);
	}
	else
	{
		//正常启动
		ServerMain();
	}


	return 0;
}

#endif


