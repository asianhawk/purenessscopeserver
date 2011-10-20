#include "FileLogger.h"

CFileLogger::CFileLogger()
{
	m_nCount = 0;
}

CFileLogger::~CFileLogger()
{

}

int CFileLogger::DoLog(int nLogType, ACE_TString* pLogText)
{
	mapLogFile::iterator f = m_mapLogFile.find(nLogType);
	if(f == m_mapLogFile.end())
	{
		return -1;
	}
	else
	{
		CLogFile* pLogFile = (CLogFile* )f->second;
		pLogFile->doLog(pLogText);
	}		

	return 0;
}

int CFileLogger::GetLogTypeCount()
{
	return (int)m_vecLogType.size();
}

int CFileLogger::GetLogType(int nIndex)
{
	if(nIndex >= (int)m_vecLogType.size())
	{
		return 0;
	}

	return (int)m_vecLogType[nIndex];
}

bool CFileLogger::Init()
{
	CAppConfig conf;
	int nFileClass = 0;
	char szFile[MAX_BUFF_1024] = {'\0'};
	char szName[MAX_BUFF_100]  = {'\0'};
	ACE_TString strFileValue;
	ACE_TString strFileClass;
	ACE_TString strServerName;

	m_vecLogType.clear();

	sprintf_safe(szFile, MAX_BUFF_1024, "%s%s", App_MainConfig::instance()->GetModulePath(), FILELOG_CONFIG);
	if(false == conf.ReadConfig(szFile))
	{
		OUR_DEBUG((LM_ERROR,"[CFileLogger::Init] Read Configfile[%s] failed\n", szFile));
		return false; 
	}

	//得到服务器名称
	conf.GetValue("ServferName",strServerName,"\\ServerInfo");
	OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]strServerName=%s\n", strServerName.c_str()));	

	//得到绝对路径
	ACE_TString tmp;
	conf.GetValue("Root",tmp,"\\ROOT");
	m_strRoot = tmp;
	OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]m_strRoot=%s\n", m_strRoot.c_str()));

	//得到日志的个数
	conf.GetValue("Count",tmp,"\\COUNT");
	m_nCount = atoi(tmp.c_str());	
	OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]m_nCount=%d\n", m_nCount));

	//添加子类的个数
	for(int i = 1000; i < 1000 + m_nCount; i++)
	{
		sprintf_safe(szName, MAX_BUFF_100, "%d", i);
		conf.GetValue(szName,tmp,"\\LOGNAME");
		strFileValue = tmp;
		OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]strFileValue=%s\n", strFileValue.c_str()));

		sprintf_safe(szName, MAX_BUFF_100, "%d", i);                                                         
		conf.GetValue(szName,tmp,"\\LOGTYPE");                                               
		strFileClass = tmp;
		nFileClass = atoi(strFileClass.c_str());                                                      
		OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]strFileClass=%d\n", nFileClass));

		//添加到管理日志文件对象map中
		mapLogFile::iterator f = m_mapLogFile.find(i);

		if(f != m_mapLogFile.end())
		{
			continue;
		}

		CLogFile* pLogFile = new CLogFile();

		pLogFile->SetLoggerName(strFileValue.c_str());
		pLogFile->SetLoggerType(i);
		pLogFile->SetLoggerClass(nFileClass);
		pLogFile->SetServerName(strServerName.c_str());
		pLogFile->Run();

		m_mapLogFile.insert(mapLogFile::value_type(pLogFile->GetLoggerType(), pLogFile));
		m_vecLogType.push_back(i);

	}

	conf.GetValue("Root",tmp,"\\ROOT");
	m_strRoot = tmp;
	return true;
}

bool CFileLogger::Close()
{
	mapLogFile::iterator b = m_mapLogFile.begin();
	mapLogFile::iterator e = m_mapLogFile.end();

	for(b; b != e; b++)
	{
		CLogFile* pLogFile = (CLogFile* )b->second;
		delete pLogFile;
	}

	m_mapLogFile.clear();
	m_vecLogType.clear();
	m_nCount = 0;

	return true;
}
