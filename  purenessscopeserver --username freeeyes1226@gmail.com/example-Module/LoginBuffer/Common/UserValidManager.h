#ifndef _USERVALIDMANAGER_H
#define _USERVALIDMANAGER_H

#include "SMOption.h"
#include <string>
#include <iostream>

//设置共享内存key值，此key对应Valid数据
#ifdef WIN32
#define SHM_USERVALID_KEY (key_t) "67891"
#define PRINTF printf_s
#else
#define SHM_USERVALID_KEY (key_t) 67891
#define PRINTF printf
#endif

#define MAX_LOGIN_VALID_COUNT  200
#define SOURCE_FILE_PATH "./LoginBufferFile/UserValid.txt"

enum EM_CheckState
{
	CHECKS_HIT = 0,      //命中
	CHECKS_UNHIT,        //无命中
};

struct _UserValid
{
	char   m_szUserName[MAX_BUFF_50];   //用户名
	char   m_szUserPass[MAX_BUFF_50];   //密码 
	uint32 m_u4LoginCount;              //登陆次数
	uint8  m_u1State;                   //当前维护状态，当维护状态出来
	bool   m_blOnline;                  //在线标记，true为在线，false为离线 
	bool   m_blDelete;                  //删除标记为，Watch进程维护这个标记，false为正在使用，true为该数据已经删除 

	_UserValid()
	{
		m_szUserName[0] = '\0';
		m_szUserPass[0] = '\0';
		m_u1State       = CHECKS_HIT;
		m_blDelete      = false;
		m_blOnline      = false;
		m_u4LoginCount  = 0;
	}
};

typedef vector<_UserValid*> vecValid;              //空闲未使用的vecValid块
typedef map<string, _UserValid*> mapUserValid;     //已使用的vecValid

class CUserValidManager
{
public:
	CUserValidManager(uint16 u2Count = MAX_LOGIN_VALID_COUNT);
	~CUserValidManager();

	bool Init();
	void Close();

	_UserValid* GetUserValid(const char* pUserName);      //根据名称得到用户状态
	void Check_File2Memory();                             //同步文件和共享内存
	void GetFreeValid();                                  //从空闲数据块中寻找已经新加载的数据

	bool Load_File(const char* pUserName);                //从文件里面寻找指定的用户名  

private:
	bool ReadFile();                //从文件加载共享内存，并加载列表
	bool ReadMemory();              //从共享内存加载列表
	void BeginCheck();              //开始检测
	void EndCheck();                //结束检测 
	bool GetFileInfo(const char* pLine, char* pUserName, char* pUserPass);
	   
private:
	CSMOption    m_SMOption;
	uint16       m_u2Count;
	mapUserValid m_mapUserValid;
	vecValid     m_vecFreeValid;
};

#endif
