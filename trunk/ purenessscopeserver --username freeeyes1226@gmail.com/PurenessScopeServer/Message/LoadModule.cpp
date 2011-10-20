// BuffPacket.h
// 这里实现模块加载
// 一步步，便可聚少为多，便能实现目标。
// add by freeeyes
// 2009-02-20

#include "LoadModule.h"

CLoadModule::CLoadModule(void)
{
	m_szModulePath[0] = '\0';
}

CLoadModule::~CLoadModule(void)
{
	Close();
}

void CLoadModule::Close()
{
	m_mapModuleInfo.Clear();
}

bool CLoadModule::LoadModule(const char* szModulePath, const char* szResourceName)
{
	vector<string> vecModuleName;
	string strModuleName;

	//将字符串数组解析成单一的模块名称
	ParseModule(szResourceName, vecModuleName);

	for(uint16 i = 0; i < (uint16)vecModuleName.size(); i++)
	{
		strModuleName = vecModuleName[i];
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadMoudle] Begin Load ModuleName[%s]!\n", strModuleName.c_str()));

		//确定这个模块是否被注册过
		_ModuleInfo* pCurr = m_mapModuleInfo.SearchMapData(strModuleName);
		if(NULL != pCurr)
		{
			//如果被注册过，先卸载现有的，再重新装载
			UnLoadModule(strModuleName.c_str());
		}

		_ModuleInfo* pModuleInfo = new _ModuleInfo();

		if(NULL == pModuleInfo)
		{
			OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadMoudle] new _ModuleInfo is error!\n"));
			return false;
		}

		//开始注册模块
		if(false == LoadModuleInfo(strModuleName, pModuleInfo))
		{
			SAFE_DELETE(pModuleInfo);
			return false;
		}

        //将注册成功的模块，加入到map中
		if(false == m_mapModuleInfo.AddMapData(strModuleName, pModuleInfo))
		{
			OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadMoudle] m_mapModuleInfo.AddMapData error!\n"));
			SAFE_DELETE(pModuleInfo);
			return false;
		}

		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadMoudle] Begin Load ModuleName[%s] OK!\n", strModuleName.c_str()));
	}

	sprintf_safe(m_szModulePath, MAX_BUFF_200, "%s", szModulePath);

	return true;
}

bool CLoadModule::UnLoadModule(const char* szResourceName)
{
	string strModuleName = szResourceName;
	_ModuleInfo* pModuleInfo = m_mapModuleInfo.SearchMapData(strModuleName);
	if(NULL == pModuleInfo)
	{
		return false;
	}
	else
	{
		//清除和此关联的所有订阅
		pModuleInfo->UnLoadModuleData();

		//这里延迟一下，因为有可能正在处理当前信息，所以必须在这里延迟一下，防止报错
		ACE_Time_Value tvSleep(MAX_LOADMODEL_CLOSE, 0);
		ACE_OS::sleep(tvSleep);

		//清除模块相关索引和数据
		ACE_OS::dlclose(pModuleInfo->hModule);
		m_mapModuleInfo.DelMapData(strModuleName);
		return true;
	}
}

int CLoadModule::GetCurrModuleCount()
{
	return m_mapModuleInfo.GetSize();
}

_ModuleInfo* CLoadModule::GetModuleIndex(int nIndex)
{
	return m_mapModuleInfo.GetMapData(nIndex);
}

bool CLoadModule::ParseModule(const char* szResourceName, vector<string>& vecModuleName)
{
	char szResource[MAX_BUFF_1024]  = {'\0'};
	char szModuleName[MAX_BUFF_100] = {'\0'};
	int  nPos = 0;

	if(szResourceName[ACE_OS::strlen(szResourceName) - 1] != ',')
	{
		sprintf_safe(szResource, MAX_BUFF_1024, "%s,", szResourceName);
	}
	else
	{
		sprintf_safe(szResource, MAX_BUFF_1024, "%s", szResourceName);
	}

	for(uint16 i = 0; i < (uint16)ACE_OS::strlen(szResource); i++)
	{
		if(szResource[i] == ',')
		{
			szModuleName[nPos] = '\0';
			nPos = 0;
			vecModuleName.push_back((string)szModuleName);
		}
		else
		{
			szModuleName[nPos] = szResource[i];
			nPos++;
		}
	}

	return true;
}

bool CLoadModule::LoadModuleInfo(string strModuleName, _ModuleInfo* pModuleInfo)
{
	char szModuleFile[MAX_BUFF_200] = {'\0'};
	if(NULL == pModuleInfo)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, pModuleInfo is NULL!\n", strModuleName.c_str()));
		return false;
	}

	sprintf_safe(szModuleFile, MAX_BUFF_200, "%s%s", m_szModulePath, strModuleName.c_str());

	m_tmModule.acquire();

	pModuleInfo->hModule = ACE_OS::dlopen(szModuleFile, RTLD_NOW);
	if(NULL == pModuleInfo->hModule || !pModuleInfo->hModule)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, pModuleInfo->hModule is NULL(%s)!\n", strModuleName.c_str(), ACE_OS::dlerror()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->LoadModuleData = (int(*)(CServerObject*))ACE_OS::dlsym(pModuleInfo->hModule, "LoadModuleData");
	if(NULL == pModuleInfo->LoadModuleData || !pModuleInfo->LoadModuleData)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Function LoadMoudle is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->UnLoadModuleData = (int(*)())ACE_OS::dlsym(pModuleInfo->hModule, "UnLoadModuleData");
	if(NULL == pModuleInfo->UnLoadModuleData || !pModuleInfo->UnLoadModuleData)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Function UnloadModule is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->GetDesc = (const char*(*)())ACE_OS::dlsym(pModuleInfo->hModule, "GetDesc");
	if(NULL == pModuleInfo->GetDesc || !pModuleInfo->GetDesc)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Function GetDesc is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->GetName = (const char*(*)())ACE_OS::dlsym(pModuleInfo->hModule, "GetName");
	if(NULL == pModuleInfo->GetName || !pModuleInfo->GetName)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Function GetName is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->GetModuleKey = (const char*(*)())ACE_OS::dlsym(pModuleInfo->hModule, "GetModuleKey");
	if(NULL == pModuleInfo->GetModuleKey || !pModuleInfo->GetModuleKey)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Function GetModuleKey is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	//加载模块代码
	int nRet = pModuleInfo->LoadModuleData(App_ServerObject::instance());
	if(nRet != 0)
	{
		OUR_DEBUG((LM_ERROR, "[CLoadModule::LoadModuleInfo] strModuleName = %s, Execute Function LoadModuleData is error!\n", strModuleName.c_str()));
		m_tmModule.release();
		return false;
	}

	pModuleInfo->strModuleName = strModuleName;
	m_tmModule.release();
	return true;
}
