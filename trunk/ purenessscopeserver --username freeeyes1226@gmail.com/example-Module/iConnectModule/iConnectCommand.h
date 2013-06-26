/*
 * ��Ϣ����
 * by w1w
 */
#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"
#include "CommandDefine.h"
#include "PostServerCommand.h"

#include <string>

using namespace std;

class CiConnectCommand :  public ACE_Task<ACE_MT_SYNCH>, public CClientCommand
{
public:
	CiConnectCommand(void);
	~CiConnectCommand(void);

	void InitConfig();                                                        //��ʼ������
	void InitServer();                                                        //��ʼ������
	int DoMessage(IMessage* pMessage, bool& bDeleteFlag);

	void SetServerObject(CServerObject* pServerObject);
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);    //��ʱ���������
private:
	CServerObject* m_pServerObject;
	CPostServerCommand m_PostServerCommand;
	ActiveTimer                   m_ActiveTimer;       //��ʱ�����̳п�ܵĶ�ʱ��
	int                            m_nTimerID;
	int            m_nCount;
	void CancelCheckServerConnect();                                          //ȡ����ʱ��
	bool StartCheckServerConnect(int nIntervalTime);                          //������ʱ��

	bool CheckpMessage(IMessage* pMessage);                                   //��������ֵ
	uint32 getAppconnect( uint16 DestAppID );                                 //��ȡAPP��Ӧ������
	void PostMessage(uint32 u4ConnectID, IBuffPacket* pBuffPacket, uint8 u1SendType, uint16 u2CommandID, bool blSendState);          //�ͻ��˷�������
	void PostMessage(vector<uint32> vecConnectID, IBuffPacket* pBuffPacket, uint8 u1SendType, uint16 u2CommandID, bool blSendState); //�ͻ���Ⱥ������
	void SendData(int nServerID, const char* pData, int nSize, bool blIsDelete);                                                     //�������䷢������

	void handle_ClientDisConnect(uint32 ConnectID);                                           //���ӶϿ�
	bool handle_AppReg(uint32 ConnectID,uint16 AppID,VCHARB_STR strsModule);                  //ע��App
	void BroadcastAppChange2Server(uint16 Appid, bool isdelete);                              //�����з�����ͬ��APP״̬
	void BroadcastAppChange2LocalApp(uint16 Serverid, uint16 Appid, bool isdelete);           //����������APPͬ����APP״̬
	void BroadCastServerState();                                                              //�㲥������״̬

	IBuffPacket* MakeQueryServerRespPacket();                                                                     //��������ѯ��Ӧ��
	IBuffPacket* MakeRegAcceptRespPacket();                                                                       //Appע��ɹ���Ӧ��
	IBuffPacket* MakeRegRejectRespPacket();                                                                       //Appע��ʧ�ܻ�Ӧ��
	IBuffPacket* MakeRegRespPacket(uint8 RegResult);                                                              //Appע���Ӧ��
	int MakeQueryAppPacket(char*& pdata,uint16 Serverid);                                                         //App��ѯ��
	IBuffPacket* MakeQueryAppPacket(uint16 Serverid);                                                             //App��ѯ��
	int MakeQueryAppRespPacket(char*& pdata);                                                                     //App��ѯ��Ӧ��
	IBuffPacket* MakeQueryAppRespPacket();                                                                        //App��ѯ��Ӧ��
	IBuffPacket* MakeQueryAppRespPacket(uint16 Serverid);                                                         //App��ѯ��Ӧ��
	int MakeSynAppPacket(char*& pdata,uint16 Appid, bool isdelete);                                               //Appͬ����
	IBuffPacket* MakeSynAppPacket(uint16 Serverid, uint16 Appid, bool isdelete);                                  //Appͬ����
	int MakeHeartPacket(char*& pdata);                                                                            //������
	IBuffPacket* MakeHeartRespPacket();                                                                           //������Ӧ��
	IBuffPacket* MakeServerDownPacket( uint16 DestServerID ,uint32 dataid);                                       //�����������߰�
	IBuffPacket* MakeAppDownPacket( uint16 DestServerID,uint16 SourAppID, uint16 DestAppID, uint32 dataid );      //App�����߰�
	int MakeAppDownPacket(char*& pdata, uint16 DestServerID,uint16 SourAppID, uint16 DestAppID, uint32 dataid );  //App�����߰�
};
