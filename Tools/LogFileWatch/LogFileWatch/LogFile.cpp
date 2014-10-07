#include "LogFile.h"

#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#endif

CLogFile::CLogFile(void)
{
}

CLogFile::~CLogFile(void)
{
}

void CLogFile::GetFileName(char* pFileName, int nLen)
{
	//���ViewWorkThread����־�ļ�
	//�õ���ǰʱ��
	ACE_Date_Time dtnow;
	dtnow.year();

	ACE_OS::snprintf(pFileName, nLen, "127_%s_%04d-%02d-%02d.log", Get_File_Key(), 
		(int)dtnow.year(), 
		(int)dtnow.month(), 
		(int)dtnow.day());
	ACE_DEBUG((LM_INFO, "[CLogFile::GetFileName]pFileName=%s.\n", pFileName));
}

bool CLogFile::Check_Log_File_State()
{
	//�������ж��ļ�״̬
	ACE_DEBUG((LM_INFO, "[CLogFile::Check_Log_File_State]Begin Check.\n"));
	bool blState = Check_File_Update_time();
	ACE_DEBUG((LM_INFO, "[CLogFile::Check_Log_File_State]End.\n"));
	return blState;
}

int CLogFile::handle_timeout( const ACE_Time_Value &tv, const void *arg )
{
	//��ʱ����ʱ�������㷨�ж���־�Ƿ���Ϲ淶

	//������Լ����ӿ�
	//Check_Log_File_State();
	//Check_File_Size();
	//Check_File_Last_Line();

	//����ļ�������ʱ��͵�ǰʱ������3���ӣ�Ҳ����180��
	//�Զ����ýű�����PSS
	if(false == Check_Log_File_State())
	{
		ACE_DEBUG((LM_INFO, "[CLogFile::handle_timeout]Begin Run Shell.\n"));
		Exec_Shell_Command();
		ACE_DEBUG((LM_INFO, "[CLogFile::handle_timeout]Begin Run End.\n"));
	}

	return 0;
}

//����ļ�������ʱ��
bool CLogFile::Check_File_Update_time()
{
	ACE_stat objStat;
	char szFileName[200] = {'\0'};
	char szPathName[300] = {'\0'};

	GetFileName(szFileName, 200);

	//ƴ��Log·�����ļ���
	ACE_OS::snprintf(szPathName, 300, "%s/%s", Get_Log_Path(), szFileName);

	int nerr = ACE_OS::stat(szPathName, &objStat);
	if(nerr != 0)
	{
		ACE_DEBUG((LM_INFO, "[Check_File_Update_time]error=%d.\n", errno));
		return false;
	}

	time_t ttUpdateTime = objStat.st_mtime;

	time_t ttNow = ACE_OS::time(NULL);

	//�жϸ���ʱ���Ƿ�͵�ǰʱ���೬��30�롣
	if(ttNow - ttUpdateTime > 180)
	{
		struct tm *pUpdateTime = NULL;

		pUpdateTime = ACE_OS::localtime(&ttUpdateTime);

		ACE_DEBUG((LM_INFO, "[Check_File_Update_time]File UpDate Time is[%02d-%02d-%02d %02d:%02d:%02d] more (%d).\n",
			pUpdateTime->tm_year + 1900, 
			pUpdateTime->tm_mon + 1,
			pUpdateTime->tm_mday,
			pUpdateTime->tm_hour,
			pUpdateTime->tm_min,
			pUpdateTime->tm_sec,
			ttNow - ttUpdateTime));

		return false;
	}
	else
	{
		return true;
	}
}

//����ļ�����С
bool CLogFile::Check_File_Size()
{
	ACE_stat objStat;
	char szFileName[200] = {'\0'};
	char szPathName[300] = {'\0'};

	GetFileName(szFileName, 200);

	//ƴ��Log·�����ļ���
	ACE_OS::snprintf(szPathName, 300, "%s/%s", Get_Log_Path(), szFileName);

	int nerr = ACE_OS::stat(szPathName, &objStat);
	if(nerr != 0)
	{
		ACE_DEBUG((LM_INFO, "[CLogFile::Check_File_Size]error=%d.\n", errno));
		return false;
	}

	ACE_DEBUG((LM_INFO, "[CLogFile::Check_File_Size]File size is[%d].\n", objStat.st_size));

	return true;
}

//�õ��ļ������һ������
bool CLogFile::Check_File_Last_Line()
{
	char szFileName[200] = {'\0'};
	char szPathName[300] = {'\0'};

	GetFileName(szFileName, 200);

	//ƴ��Log·�����ļ���
	ACE_OS::snprintf(szPathName, 300, "%s/%s", Get_Log_Path(), szFileName);

#ifdef WIN32
	ACE_DEBUG((LM_INFO, "[CLogFile::Check_File_Last_Line]windows not support.\n"));
#else
	char szCmd[300] = {'\0'};
	ACE_OS::snprintf(szCmd, 300, "tail -1 %s", szPathName);
	FILE * out = popen(szCmd, "r");
	if(out == NULL)
	{
		ACE_DEBUG((LM_INFO, "[CLogFile::Check_File_Last_Line]No Find File.\n"));
	}
	else
	{
		char szBuff[100] = {'\0'};
		fread(szBuff, sizeof(char), 100 - 1, out);
		ACE_DEBUG((LM_INFO, "Buff=%s", szBuff));
		pclose(out);
	}
#endif

	return true;
}

bool CLogFile::Exec_Shell_Command()
{
	int nErr = ACE_OS::system((ACE_TCHAR* )Get_Shell_Command());
	if(nErr != 0)
	{
		ACE_DEBUG((LM_INFO, "[CLogFile::Exec_Shell_Command]error=%d.\n", errno));
		return false;
	}
	else
	{
		ACE_DEBUG((LM_INFO, "[CLogFile::Exec_Shell_Command]exec success.\n", errno));
		return true;
	}
}