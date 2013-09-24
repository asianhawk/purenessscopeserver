// PurenessScopeServer.cpp : �������̨Ӧ�ó������ڵ㡣
//
// ��ʱ���˵������վ�������Բ���һ�����׵����飬���������ڱ���վ������Ҫ�������ģ�freeeyes
// add by freeeyes
// 2008-12-22(����)
// ��Twitter����������������һЩ���õ�С���ɣ������ںϡ�
// û��Ŀ��ļ�������õģ�Ŭ������PSS�����ʺϿ������������ٿ����߿���������д���������ʡ�
// ���ṩ���걸�Ĵ���������ƣ���һЩ���õĳ����̼��ɡ�
// ����PSS����һ������Ŭ���ˣ������˸���Ļ�飬���ǻ�����㲻�ϳɳ���
// add by freeeyes
// 2013-09-24


#include "MainConfig.h"

#ifndef WIN32
//�����Linux
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

	//��һ������ȡ�����ļ�
	if(!App_MainConfig::instance()->Init(MAINCONFIG))
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//�ж��Ƿ�����Ҫ�Է����״̬����
	if(App_MainConfig::instance()->GetServerType() == 1)
	{
		//ACE::daemonize();
		OUR_DEBUG((LM_INFO, "[main]Procress is run background.\n"));
		daemonize();
	}

	//�ڶ��������������������
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
//�����windows
#include "WindowsProcess.h"
#include "WindowsDump.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	//ָ����ǰĿ¼����ֹ�����ļ�ʧ��
	TCHAR szFileName[MAX_PATH] = {0};
	GetModuleFileName(0, szFileName, MAX_PATH);
	LPTSTR pszEnd = _tcsrchr(szFileName, TEXT('\\'));

	if (pszEnd != 0)
	{
		pszEnd++;
		*pszEnd = 0;
	}
	SetCurrentDirectory(szFileName);

	//���Dump�ļ�
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);   

	//��һ������ȡ�����ļ�
	if(!App_MainConfig::instance()->Init(MAINCONFIG))
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//�ж��Ƿ�����Ҫ�Է����״̬����
	if(App_MainConfig::instance()->GetServerType() == 1)
	{
		//�Է���״̬����
		//���ȿ���û����������windows����
		App_Process::instance()->run(argc, argv);
	}
	else
	{
		//��������
		ServerMain();
	}


	return 0;
}

#endif


