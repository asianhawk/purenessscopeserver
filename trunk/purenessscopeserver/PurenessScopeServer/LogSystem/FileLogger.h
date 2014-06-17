#ifndef _FILELOGGER_H
#define _FILELOGGER_H

#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/FILE_IO.h"
#include "ace/Date_Time.h"

#include "define.h"
#include "MainConfig.h"
#include "ILogObject.h"
#include "smtp.h"

#include <map>
#include <vector>

using namespace std;

#define MAX_CMD_NUM   100
#define MAX_TIME_SIZE 100

//����Log��������
#define LOGTYPE_OPERATION "Operation"
#define LOGTYPE_ERROR     "ServerError"

#define FILELOG_CONFIG    "logger.xml"

//��Ԫģʽ��־��
class CLogFile {
public:
	CLogFile(const char* pFileRoot, uint32 u4BufferSize)
	{
		m_nDisplay          = 0;
		m_nType             = 0;
		m_StrServerName     = "";
		m_StrlogType        = "ServerError";
		m_pBuffer           = new char[u4BufferSize];   //������������־ƴ��ʱ������
		m_u4BufferSize      = u4BufferSize;
		sprintf_safe(m_szFilRoot, MAX_BUFF_100, "%s", pFileRoot);
	};

	virtual ~CLogFile()
	{
		OUR_DEBUG((LM_INFO,"[CLogFile::~CLogFile].\n"));
		SAFE_DELETE_ARRAY(m_pBuffer);
		m_u4BufferSize = 0;
		m_File.close();
		OUR_DEBUG((LM_INFO,"[CLogFile::~CLogFile] End.\n"));
	};

	virtual int doLog(_LogBlockInfo* pLogBlockInfo)
	{
		//ÿ���Զ����
		CheckTime();

		ACE_Date_Time dt;
		char szDateBuff[MAX_TIME_SIZE] = {'\0'};

		sprintf_safe(szDateBuff, MAX_TIME_SIZE, "%04d-%02d-%02d %02d:%02d:%02d%02d,", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), dt.microsec()/10000);
		
		//ƴ��ʵ�ʵ���־�ַ���
		sprintf_safe(m_pBuffer, m_u4BufferSize, "%s %s\n", szDateBuff, pLogBlockInfo->m_pBlock);

		if(m_nDisplay == 0)
		{
			//������־
			int nLen = m_File.send(m_pBuffer, strlen(m_pBuffer));
			if(nLen != (int)strlen(m_pBuffer))
			{
				OUR_DEBUG((LM_INFO,"[%s]Write error[%s].\n", m_StrlogName.c_str(), m_pBuffer));
			}
		}
		else
		{
			//�������Ļ
			OUR_DEBUG((LM_INFO,"%s.\n", m_pBuffer));
		}

		//�鿴�Ƿ�Ҫ�����ʼ�
		if(pLogBlockInfo->m_u4MailID > 0)
		{
			SendMail(pLogBlockInfo);
		}
 
		return 0;
	};

	bool SendMail(_LogBlockInfo* pLogBlockInfo)
	{
		//�����ʼ�
		_MailAlert* pMailAlert = App_MainConfig::instance()->GetMailAlert(pLogBlockInfo->m_u4MailID);
		if(NULL == pMailAlert)
		{
			OUR_DEBUG((LM_ERROR, "[CLogFile::SendMail]MailID(%d) is no find.\n", pLogBlockInfo->m_u4MailID));
			return false;
		}

		unsigned char * pMail = NULL;
		pMail = (unsigned char * )ACE_OS::calloc(1, 1);

		int nRet = 0;
		nRet = mailText(&pMail, 
			(const unsigned char *)pMailAlert->m_szFromMailAddr, 
			(const unsigned char *)pMailAlert->m_szToMailAddr, 
			(const unsigned char *)pLogBlockInfo->m_szMailTitle, 
			(const unsigned char *)pLogBlockInfo->m_pBlock);
		nRet = mailEnd(&pMail);

		ACE_HANDLE fd;

		nRet = connectSmtp(fd, (const unsigned char *)pMailAlert->m_szMailUrl, 
			pMailAlert->m_u4MailPort);
		if(nRet != 0)
		{
			OUR_DEBUG((LM_ERROR, "[CLogFile::SendMail]MailID(%d) connectSmtp error.\n", pLogBlockInfo->m_u4MailID));
			return false;
		}

		nRet = authEmail(fd, 
			(const unsigned char *)pMailAlert->m_szFromMailAddr, 
			(const unsigned char *)pMailAlert->m_szMailPass);
		if(nRet != 0)
		{
			OUR_DEBUG((LM_ERROR, "[CLogFile::SendMail]MailID(%d) authEmail error.\n", pLogBlockInfo->m_u4MailID));
			return false;
		}

		nRet = sendEmail(fd, (const unsigned char *)pMailAlert->m_szFromMailAddr, 
			(const unsigned char *)pMailAlert->m_szToMailAddr,
			(const unsigned char *)pMail, 
			(const int)strlen((const char *)pMail));
		if(nRet != 0)
		{
			OUR_DEBUG((LM_ERROR, "[CLogFile::SendMail]MailID(%d) sendEmail error.\n", pLogBlockInfo->m_u4MailID));
			return false;
		}

		free(pMail);

		return true;
	}

	ACE_TString& GetLoggerName()
	{
		return m_StrlogName;
	};

	ACE_TString& GetServerName()
	{
		return m_StrServerName;
	}

	int GetDisPlay()
	{
		return m_nDisplay;
	}

	void SetDisplay(int nDisplay)
	{
		if(nDisplay == 0)
		{
			m_nDisplay = 0;
		}
		else
		{
			m_nDisplay = 1;
		}
	}

	void SetLoggerClass(int nType)
	{
		if(nType == 1)  //�����1��������Ӫ��־��������Ǵ�����־
		{
			m_StrlogType = LOGTYPE_OPERATION;
		}
		else
		{
			m_StrlogType = LOGTYPE_ERROR;
		}
		OUR_DEBUG((LM_INFO,"[ServerLogger](%d)m_StrlogType=%s.\n", nType, m_StrlogType.c_str()));
	}

	void SetLoggerName(const char* szLoggerName)
	{
		m_StrlogName = szLoggerName;
	};

	void SetLoggerType(int nType)
	{
		m_nType = nType;
	}

	int GetLoggerType()
	{
		return m_nType;
	}

	void SetServerName(const char* szServerName)
	{
		m_StrServerName = szServerName;
	}

	bool Run()
	{
		ACE_Date_Time dt;
		char szDateBuff[MAX_TIME_SIZE] = {'\0'};

		dt.update(ACE_OS::gettimeofday());

		CreatePath();       //���Ŀ¼�������򴴽�Ŀ¼

		sprintf_safe(szDateBuff, MAX_TIME_SIZE, "_%04d-%02d-%02d.log", dt.year(), dt.month(), dt.day());
		sprintf_safe(m_szLogTime, MAX_TIME_SIZE, "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());

		ACE_TString strLogModulePath = App_MainConfig::instance()->GetModulePath();
		ACE_TString strLogName       = strLogModulePath + "/Log/" + m_StrlogType + "/" + m_StrlogName + "/" + m_StrServerName + m_StrlogName + szDateBuff;

		m_File.close();

		m_FileAddr.set((ACE_TCHAR *)strLogName.c_str());
		if(m_Connector.connect(m_File, m_FileAddr, 0, ACE_Addr::sap_any, 0, O_WRONLY|O_CREAT|O_APPEND) == -1)
		{
			OUR_DEBUG((LM_INFO,"[ServerLogger]Create file error[%s].\n", strLogName.c_str()));
			return false;
		}

		OUR_DEBUG((LM_INFO,"[ServerLogger]Create file ok[%s].\n", strLogName.c_str()));
		//m_File.truncate(0);
		return true;
	}

	void CheckTime()
	{
		ACE_Date_Time dt;
		dt.update(ACE_OS::gettimeofday());
		char szDate[MAX_TIME_SIZE] = {'\0'};
		sprintf_safe(szDate, MAX_TIME_SIZE, "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());

		if(ACE_OS::strcmp(szDate, m_szLogTime) != 0)
		{
			Run();
		}
	}

	void CreatePath()
	{
		char szPath[MAX_CMD_NUM] = {'\0'};
		sprintf_safe(szPath, MAX_CMD_NUM, "%s/Log/", m_szFilRoot);
		ACE_OS::mkdir(szPath);

		sprintf_safe(szPath, MAX_CMD_NUM, "%s/Log/%s/", m_szFilRoot, m_StrlogType.c_str());
		ACE_OS::mkdir(szPath);

		sprintf_safe(szPath, MAX_CMD_NUM, "%s/Log/%s/%s", m_szFilRoot, m_StrlogType.c_str(), m_StrlogName.c_str());
		ACE_OS::mkdir(szPath);
	}

private:
	ACE_TString         m_StrlogName;         //ģ������
	ACE_TString         m_StrlogType;         //��־����
	ACE_TString         m_StrServerName;      //������ǰ׺
	int                 m_nDisplay;           //��ʾ���Ǽ�¼�ļ�    
	int                 m_nType;              //ģ��ID
	ACE_FILE_Connector  m_Connector;          //I/O����������
	ACE_FILE_IO         m_File;
	ACE_FILE_Addr       m_FileAddr; 
	char                m_szLogTime[MAX_TIME_SIZE];
	char                m_szFilRoot[MAX_BUFF_100];   //·������Ŀ¼
	char*               m_pBuffer;                   //��־����ָ��
	uint32              m_u4BufferSize;              //��־��������С 
};

class CFileLogger : public CServerLogger
{
public:
	CFileLogger();
	~CFileLogger();

	int DoLog(int nLogType, _LogBlockInfo* pLogBlockInfo);
	int GetLogTypeCount();
	int GetLogType(int nIndex);

	bool Init();
	bool ReSet(uint32 u4CurrLogLevel);
	bool Close();

	uint32 GetBlockSize();
	uint32 GetPoolCount();

	uint32 GetCurrLevel();

	uint16 GetLogID(uint16 u2Index);
	char* GetLogInfoByServerName(uint16 u2LogID);
	char* GetLogInfoByLogName(uint16 u2LogID);
	int   GetLogInfoByLogDisplay(uint16 u2LogID);

private:
	typedef map<uint16, CLogFile*> mapLogFile;
	typedef vector<uint16>         vecLogType; 
	mapLogFile                     m_mapLogFile;
	vecLogType                     m_vecLogType;
	char                           m_szLogRoot[MAX_BUFF_100];	
	int                            m_nCount;

	uint32                         m_u4BlockSize;
	uint32                         m_u4PoolCount;
	uint32                         m_u4CurrLogLevel;
};

#endif
