#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"
#include "UserValidManager.h"

#include <string>
#include <map>

//����ͻ�������(TCP)
#define COMMAND_LOGIN                  0x2100     //��½
#define COMMAND_LOGOUT                 0x2101     //�˳� 
#define COMMAND_USERINFO               0x2102     //���������Ϣ  
#define COMMAND_RETURN_LOGIN           0xe100     //��½Ӧ�� 
#define COMMAND_RETURN_LOGOUT          0xe101     //�ǳ�Ӧ��
#define COMMAND_RETURN_USERINFO        0xe102     //���������Ϣ  

//���峣�ñ���
#define LOGIN_SUCCESS            0
#define LOGIN_FAIL_NOEXIST       1
#define LOGIN_FAIL_ONLINE        2
#define LOGIN_FAIL_PASSWORD      3

#define OP_OK                    0
#define OP_FAIL                  1

using namespace std;

//�û���½����ṹ��
struct _UserInfo
{
	char   m_szUserPass[MAX_BUFF_100];   //�û�����
	bool   m_blOnline;                   //�Ƿ����� falseΪ�����ߣ�trueΪ����
	uint32 m_u4ConnectID;                //ConnectID

	_UserInfo()
	{
		m_szUserPass[0] = '\0';
		m_blOnline      = false;
		m_u4ConnectID   = 0;
	}
};


typedef map<string, _UserInfo*> mapUserList; //�û��б�

class CBaseCommand : public CClientCommand
{
public:
  CBaseCommand(void);
  ~CBaseCommand(void);

  void InitUserList();
  void ClearUserList();
  int DoMessage(IMessage* pMessage, bool& bDeleteFlag);
  void SetServerObject(CServerObject* pServerObject);

private:
	void Do_User_Login(IMessage* pMessage);        //�����½
	void Do_User_Logout(IMessage* pMessage);       //����ǳ�
	void Do_User_Info(IMessage* pMessage);         //����ļ��б�

private:
  CServerObject*    m_pServerObject;
  CUserValidManager m_UserValidManager;            //�û������֤
};
