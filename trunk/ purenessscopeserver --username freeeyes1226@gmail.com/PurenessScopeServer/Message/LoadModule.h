#ifndef _LOADMODULE_H
#define _LOADMODULE_H

#include "MapTemplate.h"
#include "IObject.h"
#include "ace/Thread_Mutex.h"
#include "ace/Singleton.h"
#include "ace/OS_NS_dlfcn.h"
#include <string>
#include <vector>

using namespace std;

struct _ModuleInfo
{
	string           strModuleName;
	ACE_SHLIB_HANDLE hModule;
	int (*LoadModuleData)(CServerObject* pServerObject);
	int (*UnLoadModuleData)(void);
	const char* (*GetDesc)(void);
	const char* (*GetName)(void);
	const char* (*GetModuleKey)(void);

	_ModuleInfo()
	{
	}
};

class CLoadModule
{
public:
	CLoadModule(void);
	~CLoadModule(void);

	void Close();

	bool LoadModule(const char* szModulePath, const char* szResourceName);
	bool UnLoadModule(const char* szResourceName);

	int  GetCurrModuleCount();
	_ModuleInfo* GetModuleIndex(int nIndex);


private:
	bool ParseModule(const char* szResourceName, vector<string>& vecModuleName);     //将字符串解析成数组
	bool LoadModuleInfo(string strModuleName, _ModuleInfo* pModuleInfo);             //开始加载模块的接口和数据

private:
	CMapTemplate<string, _ModuleInfo>  m_mapModuleInfo;
	char                               m_szModulePath[MAX_BUFF_200];
	ACE_Recursive_Thread_Mutex         m_tmModule;
};

typedef ACE_Singleton<CLoadModule, ACE_Null_Mutex> App_ModuleLoader; 
#endif
