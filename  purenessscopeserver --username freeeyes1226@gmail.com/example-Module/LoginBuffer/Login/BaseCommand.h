#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"
#include "UserValidManager.h"

#include <string>
#include <map>

//定义客户端信令(TCP)
#define COMMAND_LOGIN                  0x2100     //登陆
#define COMMAND_LOGOUT                 0x2101     //退出 
#define COMMAND_USERINFO               0x2102     //设置玩家信息  
#define COMMAND_RETURN_LOGIN           0xe100     //登陆应答 
#define COMMAND_RETURN_LOGOUT          0xe101     //登出应答
#define COMMAND_RETURN_USERINFO        0xe102     //设置玩家信息  

//定义常用变量
#define LOGIN_SUCCESS            0
#define LOGIN_FAIL_NOEXIST       1
#define LOGIN_FAIL_ONLINE        2
#define LOGIN_FAIL_PASSWORD      3

#define OP_OK                    0
#define OP_FAIL                  1

using namespace std;

//用户登陆情况结构体
struct _UserInfo
{
	char   m_szUserPass[MAX_BUFF_100];   //用户密码
	bool   m_blOnline;                   //是否在线 false为不在线，true为在线
	uint32 m_u4ConnectID;                //ConnectID

	_UserInfo()
	{
		m_szUserPass[0] = '\0';
		m_blOnline      = false;
		m_u4ConnectID   = 0;
	}
};


typedef map<string, _UserInfo*> mapUserList; //用户列表

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
	void Do_User_Login(IMessage* pMessage);        //处理登陆
	void Do_User_Logout(IMessage* pMessage);       //处理登出
	void Do_User_Info(IMessage* pMessage);         //获得文件列表

private:
  CServerObject*    m_pServerObject;
  CUserValidManager m_UserValidManager;            //用户身份验证
};
