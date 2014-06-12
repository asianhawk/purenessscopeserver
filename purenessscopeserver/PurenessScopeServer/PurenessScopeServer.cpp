// PurenessScopeServer.cpp : �������̨Ӧ�ó������ڵ㡣
//
// ��ʱ���˵������վ�������Բ���һ�����׵����飬���������ڱ���վ������Ҫ�������ģ�freeeyes
// add by freeeyes, freebird92
// 2008-12-22(����)
// ��Twitter����������������һЩ���õ�С���ɣ������ںϡ�
// û��Ŀ��ļ�������õģ�Ŭ������PSS�����ʺϿ������������ٿ����߿���������д���������ʡ�
// ���ṩ���걸�Ĵ���������ƣ���һЩ���õĳ����̼��ɡ�
// ����PSS����һ������Ŭ���ˣ������˸���Ļ�飬���ǻ�����㲻�ϳɳ���
// add by freeeyes
// 2013-09-24

#include "MainConfig.h"
#include "Frame_Logging_Strategy.h"

#ifndef WIN32
//�����Linux
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "ServerManager.h"

int CheckCoreLimit(int nMaxCoreFile)
{
	//��õ�ǰCore��С����
	struct rlimit rCorelimit;	

	if(getrlimit(RLIMIT_CORE, &rCorelimit) != 0)
	{
		OUR_DEBUG((LM_INFO, "[CheckCoreLimit]failed to getrlimit number of files.\n"));
		return -1;		
	}

	if(nMaxCoreFile != 0)
	{
		OUR_DEBUG((LM_INFO, "[CheckCoreLimit]** WARNING!WARNING!WARNING!WARNING! **.\n"));
		OUR_DEBUG((LM_INFO, "[CheckCoreLimit]** PSS WILL AUTO UP CORE SIZE LIMIT **.\n"));
		OUR_DEBUG((LM_INFO, "[CheckCoreLimit]** WARNING!WARNING!WARNING!WARNING! **.\n"));
		//OUR_DEBUG((LM_INFO, "[CheckCoreLimit]rlim.rlim_cur=%d, nMaxOpenFile=%d, openfile is not enougth�� please check [ulimit -a].\n", (int)rCorelimit.rlim_cur, nMaxCoreFile));	
		rCorelimit.rlim_cur = RLIM_INFINITY;
		rCorelimit.rlim_max = RLIM_INFINITY;			
		if (setrlimit(RLIMIT_CORE, &rCorelimit)!= 0) 
		{
			OUR_DEBUG((LM_INFO, "[CheckCoreLimit]failed to setrlimit core size(error=%s).\n", strerror(errno)));
			return -1;
		}	
	}
	else
	{
		if((int)rCorelimit.rlim_cur > 0)
		{
			//����ҪCore�ļ��ߴ磬�������Core�ļ���С���ó�0
			rCorelimit.rlim_cur = (rlim_t)nMaxCoreFile;
			rCorelimit.rlim_max = (rlim_t)nMaxCoreFile;			
			if (setrlimit(RLIMIT_CORE, &rCorelimit)!= 0) 
			{
				OUR_DEBUG((LM_INFO, "[Checkfilelimit]failed to setrlimit number of files.\n"));
				return -1;
			}
		}		
	}

	//OUR_DEBUG((LM_INFO, "[CheckCoreLimit]rlim.rlim_cur=%d, nMaxOpenFile=%d, openfile is not enougth�� please check [ulimit -a].\n", (int)rCorelimit.rlim_cur, nMaxCoreFile));		
	return 0;
}

//���õ�ǰ����·��
bool SetAppPath()
{
	char szPath[MAX_BUFF_300] = {'\0'};
	char* pFilePath = NULL;

	int nSize = pathconf(".",_PC_PATH_MAX);
	if((pFilePath = (char *)new char[nSize]) != NULL)
	{
		memset(pFilePath, 0, nSize);
		sprintf(pFilePath,"/proc/%d/exe",getpid());

		//�ӷ��������л�õ�ǰ�ļ�ȫ·�����ļ���
		readlink(pFilePath, szPath, nSize);
		delete[] pFilePath;
		pFilePath = NULL;
		//��szPath��������ǰ·��
		int nLen = strlen(szPath);
		while(szPath[nLen - 1]!='/') 
		{
			nLen--;
		}

		szPath[nLen > 0 ? (nLen-1) : 0]= '\0';

		chdir(szPath);
		OUR_DEBUG((LM_INFO, "[SetAppPath]Set work Path (%s) OK.\n", szPath));

		return true;
	}
	else
	{
		OUR_DEBUG((LM_INFO, "[SetAppPath]Set work Path[null].\n"));
		return false;
	}
}

//��õ�ǰ�ļ�����
int Checkfilelimit(int nMaxOpenFile)
{
	//��õ�ǰ�ļ�����
	struct rlimit rfilelimit;

	if (getrlimit(RLIMIT_NOFILE, &rfilelimit) != 0) 
	{
		OUR_DEBUG((LM_INFO, "[Checkfilelimit]failed to getrlimit number of files.\n"));
		return -1;
	}
	else
	{
		//��ʾͬʱ�ļ��������㣬��Ҫ���á�
		if((int)rfilelimit.rlim_cur < nMaxOpenFile)
		{
			OUR_DEBUG((LM_INFO, "[Checkfilelimit]** WARNING!WARNING!WARNING!WARNING! **.\n"));
			OUR_DEBUG((LM_INFO, "[Checkfilelimit]** PSS WILL AUTO UP FILE OPEN LIMIT **.\n"));
			OUR_DEBUG((LM_INFO, "[Checkfilelimit]** WARNING!WARNING!WARNING!WARNING! **.\n"));
			//����Զ������Ĺ�����ʱע�ͣ���ά��Ա����֪��������Ⲣ�Լ����ã�������ѡ��
			//������ʱ��߲����ļ���
			rfilelimit.rlim_cur = (rlim_t)nMaxOpenFile;
			rfilelimit.rlim_max = (rlim_t)nMaxOpenFile;
			if (setrlimit(RLIMIT_NOFILE, &rfilelimit)!= 0) 
			{
				OUR_DEBUG((LM_INFO, "[Checkfilelimit]failed to setrlimit number of files(error=%s).\n", strerror(errno)));
				return -1;
			}

			//����޸ĳɹ����ٴμ��һ��
			if (getrlimit(RLIMIT_NOFILE, &rfilelimit) != 0) 
			{
				OUR_DEBUG((LM_INFO, "[Checkfilelimit]failed to getrlimit number of files.\n"));
				return -1;
			} 		

			//�ٴμ���޸ĺ���ļ������
			if((int)rfilelimit.rlim_cur < nMaxOpenFile)
			{
				OUR_DEBUG((LM_INFO, "[Checkfilelimit]rlim.rlim_cur=%d, nMaxOpenFile=%d, openfile is not enougth�� please check [ulimit -a].\n", (int)rfilelimit.rlim_cur, nMaxOpenFile));
				return -1;
			}

			//OUR_DEBUG((LM_INFO, "[Checkfilelimit]rlim.rlim_cur=%d, nMaxOpenFile=%d, openfile is not enougth�� please check [ulimit -a].\n", (int)rfilelimit.rlim_cur, nMaxOpenFile));
			return 0;
		}
	}

	return 0;
}

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

	//�������õ�ǰ����·��
	SetAppPath();

	//��ȡ�����ļ�
	if(!App_MainConfig::instance()->Init())
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//�Ƿ��ACE_DEBUG�ļ��洢
	Frame_Logging_Strategy objFrameLoggingStrategy;
	Logging_Config_Param objParam;

	if(App_MainConfig::instance()->GetDebugTrunOn() == 1)
	{
		sprintf_safe(objParam.m_strLogFile, 256, "%s", App_MainConfig::instance()->GetDebugFileName());
		objParam.m_iChkInterval    = App_MainConfig::instance()->GetChkInterval();
		objParam.m_iLogFileMaxCnt  = App_MainConfig::instance()->GetLogFileMaxCnt();
		objParam.m_iLogFileMaxSize = App_MainConfig::instance()->GetLogFileMaxSize();
		sprintf_safe(objParam.m_strLogLevel, 128, "%s", App_MainConfig::instance()->GetDebugLevel());

		objFrameLoggingStrategy.InitLogStrategy(objParam);
	}

	//�жϵ�ǰ�����������Ƿ�֧�ֿ��
	if(-1 == Checkfilelimit(App_MainConfig::instance()->GetMaxHandlerCount()))
	{
		return 0;
	}

	//�жϵ�ǰCore�ļ��ߴ��Ƿ���Ҫ����
	if(-1 == CheckCoreLimit(App_MainConfig::instance()->GetCoreFileSize()))
	{
		return 0;
	}

	//�ж��Ƿ�����Ҫ�Է����״̬����
	if(App_MainConfig::instance()->GetServerType() == 1)
	{
		OUR_DEBUG((LM_INFO, "[main]Procress is run background.\n"));
		//ACE::daemonize();
		//daemonize();
		daemon(1,1);
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
	if(!App_MainConfig::instance()->Init())
	{
		OUR_DEBUG((LM_INFO, "[main]%s\n", App_MainConfig::instance()->GetError()));
	}
	else
	{
		App_MainConfig::instance()->Display();
	}

	//�Ƿ��ACE_DEBUG�ļ��洢
	Frame_Logging_Strategy objFrameLoggingStrategy;
	Logging_Config_Param objParam;

	if(App_MainConfig::instance()->GetDebugTrunOn() == 1)
	{
		sprintf_safe(objParam.m_strLogFile, 256, "%s", App_MainConfig::instance()->GetDebugFileName());
		objParam.m_iChkInterval    = App_MainConfig::instance()->GetChkInterval();
		objParam.m_iLogFileMaxCnt  = App_MainConfig::instance()->GetLogFileMaxCnt();
		objParam.m_iLogFileMaxSize = App_MainConfig::instance()->GetLogFileMaxSize();
		sprintf_safe(objParam.m_strLogLevel, 128, "%s", App_MainConfig::instance()->GetDebugLevel());

		objFrameLoggingStrategy.InitLogStrategy(objParam);
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


