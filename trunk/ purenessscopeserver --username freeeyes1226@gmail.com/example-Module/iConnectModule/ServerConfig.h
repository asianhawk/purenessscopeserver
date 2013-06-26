/*
 * �������б����ع�����
 * by w1w
 */
#pragma once

#include "IObject.h"
#include "XmlOpeation.h"
#include "CommandDefine.h"

#include "ace/Singleton.h"
#include <vector>
#include <map>

using namespace std;

//������Ϣ
struct _icConnectInfo
{
	uint32 m_ConnectID;         //����ID
	uint16 m_Appid;             //ע������ӵ�APPid
	ACE_Time_Value m_atvTime;   //�ʱ��
	_icConnectInfo()
	{
		m_ConnectID=-1;
		m_Appid=-1;
	}
};

typedef map<int, _icConnectInfo> mapConnectInfo;

//Ӧ����Ϣ
struct _icAppInfo
{
	uint16 m_Appid;                //APPid
	char m_AppName[MAX_BUFF_20];   //APP����
	ACE_Time_Value m_atvTime;      //�ʱ��

	_icAppInfo()
	{
		m_Appid = -1;
		iniChar(m_AppName,MAX_BUFF_20);
	}
};
typedef map<int, _icAppInfo> mapAppInfo;

//ͨѶ��������Ϣ
class _icServerInfo
{
public:
	uint16  m_Serverid;                      //������ID
	char m_szServerIP[MAX_BUFF_20];          //������IP
	char m_szServerName[MAX_BUFF_20];        //����������
	char m_szServerVersion[MAX_BUFF_20];     //�������汾
	char m_szStationID[33];                  //������StationID
	uint16  m_nPort;                         //�������˿�
	mapAppInfo m_mapAppInfo;                 //�����������ŵ�APP�б�
	bool m_NeedRefeshApp;                    //��������APP�б��Ƿ���Ҫ��������
	bool m_preonline;                        //��������һ������״̬

	_icServerInfo()
	{
		m_Serverid = 0;
		iniChar(m_szServerIP,MAX_BUFF_20);
		iniChar(m_szStationID,33);
		iniChar(m_szServerName,MAX_BUFF_20);
		iniChar(m_szServerVersion,MAX_BUFF_20);
		m_nPort         = 0;
		m_pClientManager = NULL;
		m_NeedRefeshApp = true;
		m_preonline = false;
	}

	bool isonline()
	{
		if (m_pClientManager!=NULL)
			return m_pClientManager->GetConnectState(m_Serverid);
		else
			return false;
	}
	void SetClientManager(IClientManager* pClientManager) { m_pClientManager = pClientManager; }
private:
	IClientManager*        m_pClientManager;
};


typedef map<int, _icServerInfo> mapServerInfo;

class CServerConfig
{
public:
	CServerConfig();
	~CServerConfig();
	_icServerInfo m_Server;          //��ǰ������
	mapServerInfo m_mapServerInfo;   //Զ�̷������б�
	mapConnectInfo m_mapConnectInfo; //��ǰ�����б�
	bool Init();                     //��ʼ��
	void Display();                  //��ʾԶ�̷������б�
	const char* GetError();
	void SetClientManager(IClientManager* pClientManager);
private:
	CXmlOpeation m_ServerConfig;
	char       m_szError[MAX_BUFF_500];
};

typedef ACE_Singleton<CServerConfig, ACE_Null_Mutex> icServerConfig;