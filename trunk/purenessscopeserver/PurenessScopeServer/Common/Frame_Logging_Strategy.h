
#pragma once

#include "ace/Logging_Strategy.h"
#include <string>
using namespace std;

#define LOG_CONFIG_ARGV_COUNT	6


class Logging_Config_Param
{
public:
	Logging_Config_Param();
	~Logging_Config_Param();

	//��־�ļ���ȫ·��
	char m_strLogFile[256];
	
	//�ļ���С���ʱ��(Secs)
	int m_iChkInterval;

	//ÿ����־�ļ�����С(KB)
	int m_iLogFileMaxSize;

	//��־�ļ�������
	int m_iLogFileMaxCnt;

	//�Ƿ����ն˷���
	int m_bSendTerminal;

    char m_strLogLevel[128];
};

class Frame_Logging_Strategy
{
public:
	Frame_Logging_Strategy();
	~Frame_Logging_Strategy();

    //��ȡ��־�ļ������ļ�
    int Read_Log_Config(Logging_Config_Param &cfgParam);

    //��־����
    string GetLogLevel(const string &strLogLevel);

	
	int InitLogStrategy();

    //��ʼ����־����
    int InitLogStrategy(Logging_Config_Param &ConfigParam);
	
    //��������
    int EndLogStrategy();

private:
	ACE_Reactor *pLogStraReactor;
	ACE_Logging_Strategy *pLogStrategy; 
};

