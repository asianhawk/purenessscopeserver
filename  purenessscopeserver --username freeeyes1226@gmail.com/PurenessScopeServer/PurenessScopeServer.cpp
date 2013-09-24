// PurenessScopeServer.cpp : 定义控制台应用程序的入口点。
//
// 有时候的说，重新站起来绝对不是一件容易的事情，但是我现在必须站起来。要做好样的，freeeyes
// add by freeeyes
// 2008-12-22(冬至)
// 从Twitter的主程序中吸收了一些有用的小技巧，不断融合。
// 没有目标的坚持是无用的，努力的让PSS更加适合开发，尽量减少开发者开发量，让写代码变的舒适。
// 并提供更完备的错误分析机制，和一些更好的程序编程技巧。
// 现在PSS不是一个人在努力了，它有了更多的伙伴，他们会帮助你不断成长。
// add by freeeyes
// 2013-09-24


#include "MainConfig.h"

#ifndef WIN32
//如果是Linux
#include <unistd.h>
#include "ServerManager.h"

static int daemonize()
{
	pid_t pid, sid;
	int fd;

	pid = fork();
	switch (pid) {
	case -1:
		printf("[daemonize]fork() failed: %s", strerror(errno));
		return -1;

	case 0:
		break;
	default:
		_exit(0);
	}

	sid = setsid();
	if (sid < 0) {
		printf("[daemonize]setsid() failed: %s", strerror(errno));
		return -1;
	}

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		printf("[daemonize]signal(SIGHUP, SIG_IGN) failed: %s", strerror(errno));
		return -1;
	}

	int status = chdir("/");
	if (status < 0) {
		printf("chdir(\"/\") failed: %s", strerror(errno));
		return -1;
	}

	umask(0);

	fd = open("/dev/null", O_RDWR);
	if (fd < 0) {
		printf("[daemonize]open(\"/dev/null\") failed: %s", strerror(errno));
		return -1;
	}

	status = dup2(fd, STDIN_FILENO);
	if (status < 0) {
		printf("[daemonize]dup2(%d, STDIN) failed: %s", fd, strerror(errno));
		close(fd);
		return -1;
	}

	status = dup2(fd, STDOUT_FILENO);
	if (status < 0) {
		printf("[daemonize]dup2(%d, STDOUT) failed: %s", fd, strerror(errno));
		close(fd);
		return -1;
	}

	status = dup2(fd, STDERR_FILENO);
	if (status < 0) {
		printf("[daemonize]dup2(%d, STDERR) failed: %s", fd, strerror(errno));
		close(fd);
		return -1;
	}

	if (fd > STDERR_FILENO) {
		status = close(fd);
		if (status < 0) {
			printf("[daemonize]close(%d) failed: %s", fd, strerror(errno));
			return -1;
		}
	}

	return 0;    
}

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
		//ACE::daemonize();
		OUR_DEBUG((LM_INFO, "[main]Procress is run background.\n"));
		daemonize();
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


