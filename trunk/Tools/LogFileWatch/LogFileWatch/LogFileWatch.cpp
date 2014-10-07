// LogFileWatch.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "ace/pre.h"
#include "ace/Date_Time.h"
#include "ace/OS_NS_unistd.h"

#include "LogFile.h"
#include "XmlOpeation.h"
#include "TimeManager.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <vector>
using namespace std;

#define CONFIG_NAME "FileWatch.xml"

typedef vector<_LogFileInfo> vecLogFileInfo;

struct _ConsoleInfo
{
	char m_szConsoleIP[20];
	int  m_nPort;
	char m_szKey[50];

	_ConsoleInfo()
	{
		m_szConsoleIP[0] = '\0';
		m_nPort          = 0;
		m_szKey[0]       = '\0';
	}
};

#ifndef WIN32
//���õ�ǰ����·��
bool SetAppPath()
{
	char szPath[300] = {'\0'};
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
		ACE_DEBUG((LM_INFO, "[SetAppPath]Set work Path (%s) OK.\n", szPath));
		return true;
	}
	else
	{
		ACE_DEBUG((LM_INFO, "[SetAppPath]Set work Path[null].\n"));
		return false;
	}
}
#endif



//��ȡ�����ļ�
bool Init_Read_Config(vecLogFileInfo& objvecLogFileInfo)
{
	CXmlOpeation objMainConfig;
	if(false == objMainConfig.Init(CONFIG_NAME))
	{
		ACE_DEBUG((LM_INFO, "[Init_Read_Config]open config file error.\n"));
		return false;
	}

	TiXmlElement* pNextTiXmlElementID          = NULL;
	TiXmlElement* pNextTiXmlElementPath        = NULL;
	TiXmlElement* pNextTiXmlElementKey         = NULL;
	TiXmlElement* pNextTiXmlElementWatchTime   = NULL;
	TiXmlElement* pNextTiXmlElementShellCmd    = NULL;

	char* pData = NULL;

	while(true)
	{
		_LogFileInfo objLogFileInfo;

		pData = objMainConfig.GetData("FileInfo", "ID", pNextTiXmlElementID);
		if(pData != NULL)
		{
			objLogFileInfo.m_nID = (int)ACE_OS::atoi(pData);
		}
		else
		{
			return true;
		}

		pData = objMainConfig.GetData("FileInfo", "Path", pNextTiXmlElementPath);
		if(pData != NULL)
		{
			ACE_OS::snprintf(objLogFileInfo.m_szLogPath, LOG_PATH_MAX_LENGTH, "%s", pData);
		}
		else
		{
			return true;
		}

		pData = objMainConfig.GetData("FileInfo", "Key", pNextTiXmlElementKey);
		if(pData != NULL)
		{
			ACE_OS::snprintf(objLogFileInfo.m_szFileKey, LOG_PATH_MAX_LENGTH, "%s", pData);
		}
		else
		{
			return true;
		}

		pData = objMainConfig.GetData("FileInfo", "WatchTime", pNextTiXmlElementWatchTime);
		if(pData != NULL)
		{
			objLogFileInfo.m_nWatchTime = (int)ACE_OS::atoi(pData);
		}
		else
		{
			return true;
		}

		pData = objMainConfig.GetData("FileInfo", "ShellCmd", pNextTiXmlElementShellCmd);
		if(pData != NULL)
		{
			ACE_OS::snprintf(objLogFileInfo.m_szShellCmd, LOG_PATH_MAX_LENGTH, "%s", pData);
		}
		else
		{
			return true;
		}

		objvecLogFileInfo.push_back(objLogFileInfo);
	}

	_ConsoleInfo objConsoleInfo;
	pData = objMainConfig.GetData("ConsoleInfo", "IP");
	if(pData != NULL)
	{
		ACE_OS::snprintf(objConsoleInfo.m_szConsoleIP, 20, "%s", pData);
	}
	else
	{
		return true;
	}

	pData = objMainConfig.GetData("ConsoleInfo", "Port");
	if(pData != NULL)
	{
		objConsoleInfo.m_nPort = ACE_OS::atoi(pData);
	}
	else
	{
		return true;
	}

	pData = objMainConfig.GetData("ConsoleInfo", "Keypwd");
	if(pData != NULL)
	{
		ACE_OS::snprintf(objConsoleInfo.m_szKey, 50, "%s", pData);
	}
	else
	{
		return true;
	}

	return true;
}


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	vecLogFileInfo objvecLogFileInfo;

#ifndef WIN32
	//Linux�����õ�ǰ·��
	SetAppPath();
#endif

	//��ȡ�����ļ�
	if(false == Init_Read_Config(objvecLogFileInfo))
	{
		getchar();
		return 0;
	}

	CLogFile* pLogFile = NULL;

	//��������������Ӧ��LOG�㷨
	for(int i = 0; i < (int)objvecLogFileInfo.size(); i++)
	{
		int nID = objvecLogFileInfo[i].m_nID;
		if(nID == 1)
		{
			//ClogFile�����һ����־������
			pLogFile = new CLogFile();
			pLogFile->Set_ID(objvecLogFileInfo[i].m_nID);
			pLogFile->Set_Log_Path(objvecLogFileInfo[i].m_szLogPath);
			pLogFile->Set_File_Key(objvecLogFileInfo[i].m_szFileKey);
			pLogFile->Set_Shell_Command(objvecLogFileInfo[i].m_szShellCmd);
			pLogFile->Set_Watch_Time(objvecLogFileInfo[i].m_nWatchTime);
			
			ILogFileBase* pLogFileBase = (ILogFileBase* )pLogFile;
			//��Ӷ�ʱ��
			App_TimerManager::instance()->schedule(pLogFileBase, NULL, 
				ACE_OS::gettimeofday() + ACE_Time_Value(pLogFileBase->Get_Watch_Time()), 
				ACE_Time_Value(pLogFileBase->Get_Watch_Time()));
		}
		else if(nID == 2)
		{
			//�������������Լ�����־�ж�������
		}
	}

	//������ʱ��
	App_TimerManager::instance()->activate();

	ACE_Thread_Manager::instance()->wait();

	//�����Դ
	delete pLogFile;

	return 0;
}

