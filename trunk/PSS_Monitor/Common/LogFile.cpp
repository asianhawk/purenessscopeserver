#include "LogFile.h"

CLogFile::CLogFile()
{
	m_pFile             = NULL;
	m_szFileName[0]     = '\0';
	m_szRoot[0]         = '\0';
	m_szLogPath[0]      = '\0';
	m_szLogTime[0]      = '\0';
	m_szCurrFilePath[0] = '\0';
}

CLogFile::~CLogFile()
{
	if(NULL != m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

void CLogFile::SetRoot(const char* pRoot)
{
	sprintf_safe(m_szRoot, MAX_BUFF_50, "%s", pRoot);

	//���Ŀ¼�������򴴽�֮
	char szPath[MAX_BUFF_100] = {'\0'};
	sprintf_safe(szPath, MAX_BUFF_100, "%s/MonitorLog/", m_szRoot);
	ACE_OS::mkdir(szPath);
}

void CLogFile::SetLogName(const char* pLogName)
{
	sprintf_safe(m_szFileName, MAX_BUFF_50, "%s", pLogName);

	//���Ŀ¼�������򴴽�֮
	char szPath[MAX_BUFF_100] = {'\0'};
	sprintf_safe(szPath, MAX_BUFF_100, "%sMonitorLog/%s/", m_szRoot, pLogName);
	sprintf_safe(m_szLogPath, MAX_BUFF_100, "%sMonitorLog/%s/", m_szRoot, pLogName);
	ACE_OS::mkdir(szPath);

	//���õ�ǰ��־ʱ��
	CheckTime();
}

void CLogFile::CheckTime()
{
	ACE_Date_Time dt;
	dt.update(ACE_OS::gettimeofday());
	char szDate[MAX_BUFF_50] = {'\0'};
	sprintf_safe(szDate, MAX_BUFF_50, "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());

	if(ACE_OS::strcmp(szDate, m_szLogTime) != 0)
	{
		sprintf_safe(m_szLogTime, MAX_BUFF_50, "%s", szDate);
		CreateFile(dt);
	}
}

void CLogFile::CreateFile(ACE_Date_Time& dt)
{
	bool blFileExist = false;

	//m_File.close();
	if(NULL != m_pFile)
	{
		ACE_OS::fclose(m_pFile);
	}

	sprintf_safe(m_szCurrFilePath, MAX_BUFF_100, "%s%s_%04d%02d%02d.xml", m_szLogPath, m_szFileName, dt.year(), dt.month(), dt.day());

	//�жϵ�ǰ�ļ��Ƿ����
	int nRet = ACE_OS::access(m_szCurrFilePath, 0);
	if(-1 != nRet)
	{
		//�ļ��Ѿ�����
		blFileExist = true;
	}

	m_pFile = fopen(m_szCurrFilePath, "a+");
	if(NULL == m_pFile)
	{
		OUR_DEBUG((LM_INFO, "[CLogFile::CreateFile]file create error(%s)(%d).", m_szCurrFilePath, errno));
		return;
	}

	if(false == blFileExist)
	{
		//дXMLͷ
		char szLog[MAX_BUFF_1000]     = {'\0'};
		sprintf_safe(szLog, MAX_BUFF_1000, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?xml-stylesheet type=\"text/xsl\" href=\"../../Sample.xsl\"?>\n<Group Name=\"%s\">\n</Group>", m_szFileName);
		ACE_OS::fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), m_pFile);
		ACE_OS::fflush(m_pFile);
		ACE_OS::fclose(m_pFile);
		m_pFile = NULL;
	}
}


void CLogFile::ReWriteFile()
{
	FILE* pNewFile = NULL;
	char* pBuff    = NULL;
	int nFileSize  = 0;
	pNewFile = fopen(m_szCurrFilePath, "rb");

	//��ȡ���ļ�
	if(NULL != pNewFile)
	{
		ACE_OS::fseek(pNewFile, 0L, SEEK_END);

		nFileSize = (int)ACE_OS::ftell(pNewFile);

		ACE_OS::fseek(pNewFile, 0L, SEEK_SET);

		pBuff = new char[nFileSize + 1];
		memset(pBuff, 0, nFileSize + 1);

		ACE_OS::fread(pBuff, nFileSize, sizeof(char), pNewFile);
		pBuff[nFileSize - 8] = '\0';

		ACE_OS::fclose(pNewFile);
		pNewFile = NULL;
	}

	//д�����ļ�
	pNewFile = fopen(m_szCurrFilePath, "wb+");
	if(NULL != pNewFile)
	{
		ACE_OS::fwrite(pBuff, ACE_OS::strlen(pBuff), sizeof(char), pNewFile);
		ACE_OS::fclose(pNewFile);
		pNewFile = NULL;
	}

	if(NULL != pBuff)
	{
		delete[] pBuff;
	}
}

void CLogFile::DoLog(const char* fmt, ...)
{
	char szLog[MAX_BUFF_1000]     = {'\0'};

	va_list ap;
	va_start(ap, fmt);
	ACE_OS::vsnprintf(szLog, MAX_BUFF_1000 - 1, fmt, ap);
	va_end(ap);

	//�������
	CheckTime();

	//�ļ�������λ��Ϊ�˼���XML��ʽ
	ReWriteFile();

	m_pFile = fopen(m_szCurrFilePath, "a+");
	if(NULL != m_pFile)
	{
		//������־
		ACE_OS::fwrite(szLog, ACE_OS::strlen(szLog), sizeof(char), m_pFile);

		//д���ļ�ĩβ
		char szEnd[MAX_BUFF_20] = {'\0'};
		sprintf_safe(szEnd, MAX_BUFF_20, "</Group>");
		ACE_OS::fwrite(szEnd, ACE_OS::strlen(szEnd), sizeof(char), m_pFile);

		ACE_OS::fflush(m_pFile);
		ACE_OS::fclose(m_pFile);
		m_pFile = NULL;
	}


}


