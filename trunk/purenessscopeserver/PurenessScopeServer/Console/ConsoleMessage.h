#ifndef _CONSOLEMESSAGE_H
#define _CONSOLEMESSAGE_H

#include <ace/OS_NS_sys_resource.h>

#include "define.h"
#include "IBuffPacket.h"
#include "LoadModule.h"
#include "MessageManager.h"
#include "MessageService.h"
#include "MakePacket.h"
#include "ForbiddenIP.h"
#include "ace/Message_Block.h"
#include "IPAccount.h"

#ifdef WIN32
#include "ProConnectHandle.h"
#include "ClientProConnectManager.h"
#include "ProUDPManager.h"
#include "WindowsCPU.h"
#include "ProControlListen.h"
#else
#include "ConnectHandler.h"
#include "ClientReConnectManager.h"
#include "ReUDPManager.h"
#include "LinuxCPU.h"
#include "ControlListen.h"
#endif

//�������ֵ���Ͷ���
enum
{
	CONSOLE_MESSAGE_SUCCESS = 0,
	CONSOLE_MESSAGE_FAIL    = -1,
};

//���ö�Ӧ�����������ƣ����ڽ��ն˰�����
#define CONSOLE_COMMAND_UNKNOW             0x1000
#define CONSOLE_COMMAND_LOADMOUDLE         0x1001
#define CONSOLE_COMMAND_UNLOADMOUDLE       0x1002
#define CONSOLE_COMMAND_RELOADMOUDLE       0x1003
#define CONSOLE_COMMAND_SHOWMOUDLE         0x1004
#define CONSOLE_COMMAND_CLIENTCOUNT        0x1005
#define CONSOLE_COMMAND_COMMANDINFO        0x1005
#define CONSOLE_COMMAND_COMMANDTIMEOUT     0x1007
#define CONSOLE_COMMAND_COMMANDTIMEOUTCLR  0x1008
#define CONSOLE_COMMAND_COMMANDDATALOG     0x1009
#define CONSOLE_COMMAND_THREADINFO         0x100A
#define CONSOLE_COMMAND_CLIENTINFO         0x100B
#define CONSOLE_COMMAND_UDPCONNECTINFO     0x100C
#define CONSOLE_COMMAND_COLSECLIENT        0x100D
#define CONSOLE_COMMAND_FORBIDDENIP        0x100E
#define CONSOLE_COMMAND_FORBIDDENIPSHOW    0x100F
#define CONSOLE_COMMAND_LIFTED             0x1010
#define CONSOLE_COMMAND_SERVERCONNECT_TCP  0x1011
#define CONSOLE_COMMAND_SERVERCONNECT_UDP  0x1012
#define CONSOLE_COMMAND_PROCESSINFO        0x1013
#define CONSOLE_COMMAND_CLIENTHISTORY      0x1014
#define CONSOLE_COMMAND_ALLCOMMANDINFO     0x1015
#define CONSOLE_COMMAND_SERVERINFO         0x1016
#define CONSOLE_COMMAND_SERVERRECONNECT    0x1017
#define CONSOLE_COMMAND_SETDEBUG           0x1018
#define CONSOLE_COMMAND_SHOWDEBUG          0x1019
#define CONSOLE_COMMAND_SETTRACKIP         0x101A
#define CONSOLE_COMMAND_DELTRACKIP         0x101B
#define CONSOLE_COMMAND_GETTRACKIPINFO     0x101C
#define CONSOLE_COMMAND_GETCONNECTIPINFO   0x101D
#define CONSOLE_COMMAND_GETLOGINFO         0x101E
#define CONSOLE_COMMAND_SETLOGLEVEL        0x101F
#define CONSOLE_COMMAND_GETWTAI            0x1020
#define CONSOLE_COMMAND_GETWTTIMEOUT       0x1021
#define CONSOLE_COMMAND_SETWTAI            0x1022
#define CONSOLE_COMMAND_GETNICKNAMEINFO    0x1023
#define CONSOLE_COMMAND_SETCONNECTLOG      0x1024
#define CONSOLE_COMMAND_SETMAXCONNECTCOUNT 0x1025
#define CONSOLE_COMMAND_ADD_LISTEN         0x1026
#define CONSOLE_COMMAND_DEL_LISTEN         0x1027
#define CONSOLE_COMMAND_SHOW_LISTEN        0x1028
#define CONSOLE_COMMAND_MONITOR_INFO       0x1029

//Ŀǰ֧�ֵ�����
#define CONSOLEMESSAHE_LOADMOUDLE         "LoadModule"          //����ģ��
#define CONSOLEMESSAHE_UNLOADMOUDLE       "UnLoadModule"        //ж��ģ��
#define CONSOLEMESSAHE_RELOADMOUDLE       "ReLoadModule"        //���¼���ģ��
#define CONSOLEMESSAHE_SHOWMOUDLE         "ShowModule"          //��ʾ���������Ѿ����ص�ģ��
#define CONSOLEMESSAHE_CLIENTCOUNT        "ClientCount"         //��ǰ�ͻ���������
#define CONSOLEMESSAHE_COMMANDINFO        "CommandInfo"         //��ǰĳһ�������״̬��Ϣ
#define CONSOLEMESSAHE_COMMANDTIMEOUT     "CommandTimeout"      //���г�ʱ�������б�
#define CONSOLEMESSAHE_COMMANDTIMEOUTCLR  "CommandTimeoutclr"   //�����ʱ�������б�
#define CONSOLEMESSAGE_COMMANDDATALOG     "CommandDataLog"      //�洢CommandDataLog
#define CONSOLEMESSAHE_THREADINFO         "WorkThreadState"     //��ǰ�����̺߳͹����߳�״̬
#define CONSOLEMESSAHE_CLIENTINFO         "ConnectClient"       //��ǰ�ͻ������ӵ���Ϣ
#define CONSOLEMESSAHE_UDPCONNECTINFO     "UDPConnectClient"    //��ǰUDP�ͻ��˵�������Ϣ
#define CONSOLEMESSAHE_COLSECLIENT        "CloseClient"         //�رտͻ���
#define CONSOLEMESSAHE_FORBIDDENIP        "ForbiddenIP"         //��ֹIP����
#define CONSOLEMESSAHE_FORBIDDENIPSHOW    "ShowForbiddenIP"     //�鿴��ֹ����IP�б�
#define CONSOLEMESSAHE_LIFTED             "LiftedIP"            //���ĳIP
#define CONSOLEMESSAHE_SERVERCONNECT_TCP  "ServerConnectTCP"    //��������ͨѶ(TCP)
#define CONSOLEMESSAHE_SERVERCONNECT_UDP  "ServerConnectUDP"    //��������ͨѶ(UDP)
#define CONSOLEMESSAGE_PROCESSINFO        "ShowCurrProcessInfo" //�鿴��ǰ������������״̬
#define CONSOLEMESSAGE_CLIENTHISTORY      "ShowConnectHistory"  //�鿴��������ʷ����״̬
#define CONSOLEMESSAGE_ALLCOMMANDINFO     "ShowAllCommand"      //�鿴����������ע��ģ��������Ϣ
#define CONSOLEMESSAGE_SERVERINFO         "ShowServerInfo"      //�鿴������������Ϣ
#define CONSOLEMESSAGE_SERVERRECONNECT    "ReConnectServer"     //Զ�˿�������ĳһ��Զ�˷�����
#define CONSOLEMESSAGE_SETDEBUG           "SetDebug"            //���õ�ǰDEBUG״̬
#define CONSOLEMESSAGE_SHOWDEBUG          "ShowDebug"           //�鿴��ǰDEBUG״̬
#define CONSOLEMESSAGE_SETTRACKIP         "SetTrackIP"          //����Ҫ��ص�IP
#define CONSOLEMESSAGE_DELTRACKIP         "DelTrackIP"          //ɾ��Ҫ��ص�IP
#define CONSOLEMESSAGE_GETTRACKIPINFO     "GetTrackIPInfo"      //�õ����IP����ʷ��¼
#define CONSOLEMESSAGE_GETCONNECTIPINFO   "GetConnectIPInfo"    //ͨ��COnnectID�����ص�IP��Ϣ
#define CONSOLEMESSAGE_GETLOGINFO         "GetLogInfo"          //�õ���־�ȼ�
#define CONSOLEMESSAGE_SETLOGLEVEL        "SetLogLevel"         //������־�ȼ�
#define CONSOLEMESSAGE_GETWTAI            "GetWorkThreadAI"     //�õ�Thread��AI������Ϣ
#define CONSOLEMESSAGE_GETWTTIMEOUT       "GetWorkThreadTO"     //�õ�Thread�����г�ʱ���ݰ���Ϣ
#define CONSOLEMESSAGE_SETWTAI            "SetWorkThreadAI"     //����ThreadAI��������Ϣ
#define CONSOLEMESSAGE_GETNICKNAMEINFO    "GetNickNameInfo"     //�õ�������Ϣ
#define CONSOLEMESSAGE_SETCONNECTLOG      "SetConnectLog"       //����������־����״̬ 
#define CONSOLEMESSAGE_SETMAXCONNECTCOUNT "SetMaxConnectCount"  //�������������
#define CONSOLEMESSAGE_ADD_LISTEN         "AddListen"           //���һ���µļ����˿�
#define CONSOLEMESSAGE_DEL_LISTEN         "DelListen"           //ɾ��һ���µļ����˿�
#define CONSOLEMESSATE_SHOW_LISTEN        "ShowListen"          //�鿴���ڴ򿪵ļ����˿� 
#define CONSOLEMESSATE_MONITOR_INFO       "Monitor"             //���������в����ӿ�

//��������
struct _CommandInfo
{
	char m_szCommandTitle[MAX_BUFF_100];  //��������ͷ
	char m_szCommandExp[MAX_BUFF_100];    //����������չ����

	_CommandInfo()
	{
		m_szCommandTitle[0] = '\0';
		m_szCommandExp[0]   = '\0';
	}
};

//�ļ����ṹ
struct _FileInfo
{
	char m_szFilePath[MAX_BUFF_100];
	char m_szFileName[MAX_BUFF_100];

	_FileInfo()
	{
		m_szFilePath[0] = '\0';
		m_szFileName[0] = '\0';
	}
};

//�����˿���Ϣ
struct _ListenInfo
{
	char   m_szListenIP[MAX_BUFF_20];
	uint32 m_u4Port;
	uint8  m_u1IPType;

	_ListenInfo()
	{
		m_szListenIP[0] = '\0';
		m_u4Port        = 0;
		m_u1IPType      = TYPE_IPV4;
	}
};

class CConsoleMessage
{
public:
	CConsoleMessage();
	~CConsoleMessage();

	int Dispose(ACE_Message_Block* pmb, IBuffPacket* pBuffPacket);             //Ҫ����������ֽ���, pBuffPacketΪ����Ҫ���͸��ͻ��˵�����

	//��ʼ������
	bool SetConsoleKey(vecConsoleKey* pvecConsoleKey);       //�����֤�����keyֵ

	//�������ݲ���
private:
	int ParseCommand(const char* pCommand, IBuffPacket* pBuffPacket);                        //ִ������
	bool GetCommandInfo(const char* pCommand, _CommandInfo& CommandInfo);                    //�������и��Ӧ���е����ݸ�ʽ
	bool GetFileInfo(const char* pFile, _FileInfo& FileInfo);                                //��һ��ȫ·���зֳ��ļ���
	bool GetForbiddenIP(const char* pCommand, _ForbiddenIP& ForbiddenIP);                    //�õ���ֹ��IP�б�
	bool GetConnectServerID(const char* pCommand, int& nServerID);                           //�õ�һ��ָ���ķ�����ID
	bool GetDebug(const char* pCommand, uint8& u1Debug);                                     //�õ���ǰ���õ�BUDEG
	bool CheckConsoleKey(const char* pKey);                                                  //��֤key
	bool GetTrackIP(const char* pCommand, _ForbiddenIP& ForbiddenIP);                        //�õ����õ�׷��IP
	bool GetLogLevel(const char* pCommand, int& nLogLevel);                                  //�õ���־�ȼ�
	bool GetAIInfo(const char* pCommand, int& nAI, int& nDispose, int& nCheck, int& nStop);  //�õ�AI����
	bool GetNickName(const char* pCommand, char* pName);                                     //�õ����ӱ���
	bool GetConnectID(const char* pCommand, uint32& u4ConnectID, bool& blFlag);              //�õ�ConnectID
	bool GetMaxConnectCount(const char* pCommand, uint16& u2MaxConnectCount);                //�õ�������������
	bool GetListenInfo(const char* pCommand, _ListenInfo& objListenInfo);                    //�õ������˿���Ϣ

	//�������ʵ�ֲ���
private:
	bool DoMessage_LoadModule(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_UnLoadModule(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ReLoadModule(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ClientMessageCount(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowModule(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_CommandInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_WorkThreadState(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ClientInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessgae_CloseClient(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ForbiddenIP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowForbiddenList(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_LifedIP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_UDPClientInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ServerConnectTCP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ServerConnectUDP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowProcessInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowClientHisTory(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowAllCommandInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowServerInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ReConnectServer(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_CommandTimeout(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_CommandTimeoutclr(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_CommandDataLog(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetDebug(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowDebug(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetTrackIP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_DelTrackIP(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetTrackIPInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetConnectIPInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetLogLevelInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetLogLevelInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetThreadAI(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetWorkThreadTO(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetWorkThreadAI(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_GetNickNameInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetConnectLog(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_SetMaxConnectCount(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_AddListen(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_DelListen(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_ShowListen(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);
	bool DoMessage_MonitorInfo(_CommandInfo& CommandInfo, IBuffPacket* pBuffPacket, uint16& u2ReturnCommandID);

private:
	vecConsoleKey* m_pvecConsoleKey;
};

#endif
