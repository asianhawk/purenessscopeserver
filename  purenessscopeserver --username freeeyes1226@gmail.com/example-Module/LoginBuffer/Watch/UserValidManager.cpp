#include "UserValidManager.h"

CUserValidManager::CUserValidManager(uint16 u2Count)
{
	m_u2Count = u2Count;
}

CUserValidManager::~CUserValidManager()
{
	Close();
}

bool CUserValidManager::Init()
{
	bool blIsCreate = true;
	bool blIsOpen   = true;
	
	blIsOpen = m_SMOption.Init(SHM_USERVALID_KEY, sizeof(_UserValid), (uint16)MAX_LOGIN_VALID_COUNT, blIsCreate);
	if(false == blIsOpen)
	{
		return false;
	}

	if(blIsCreate == true)
	{
		//共享内存第一次创建，需要从文件里面重建共享内存
		ReadFile();
	}
	else
	{
		//共享内存已经存在，遍历获得列表
		ReadMemory();
	}

	return true;
}

bool CUserValidManager::ReadFile()
{
	//从文件里面获取当前可以支持的用户名和密码，放入共享内存中去
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_PATH);
	FILE* pFile = fopen((char* )szFileName, "r");
	if(pFile == NULL)
	{
		//文件不存在
		return false;
	}

	fseek(pFile, 0l, SEEK_END);

	uint32 u4FileSize = ftell(pFile);

	if(u4FileSize == 0)
	{
		//文件长度为0
		return false;
	}

	fclose(pFile);

	m_mapUserValid.clear();
	m_vecFreeValid.clear();

	pFile = fopen((char* )szFileName, "r");
	fseek(pFile, 0l, SEEK_SET);

	char* pFileBuffer = new char[u4FileSize + 1];
	memset(pFileBuffer, 0, u4FileSize);

	//读取文件,一次读不完循环读，直到全部读完为止
	uint32 u4ReadSize = (uint32)fread((char* )pFileBuffer, sizeof(char), u4FileSize, pFile);
	if(u4ReadSize >= u4FileSize)
	{
		//读完了
	}
	else if(u4ReadSize == 0)
	{
		//读取文件失败了
		SAFE_DELETE_ARRAY(pFileBuffer);
		fclose(pFile);
		return false;
	}
	
	ID_t nIndex = 1;
	char szUserName[MAX_BUFF_50] = {'\0'};
	char szUserPass[MAX_BUFF_50] = {'\0'};

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	while(pLine != NULL) 
	{
		if(nIndex > (uint32)m_u2Count)
		{
			break;
		}

		bool blState = GetFileInfo(pLine, szUserName, szUserPass);
		if(true == blState)
		{
			_UserValid* pUserValid = (_UserValid* )m_SMOption.GetBuffer(nIndex);
			if(NULL != pUserValid)
			{
				//初始化共享内存数据
				sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
				sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
				pUserValid->m_blDelete     = false;
				pUserValid->m_blOnline     = false;
				pUserValid->m_u1State      = CHECKS_HIT;
				pUserValid->m_u4LoginCount = 0;

				string strUserName;
				strUserName = (string)pUserValid->m_szUserName;

				m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));

				nIndex++;
			}
		}
		pLine = strtok(NULL, szFind);
	}

	//剩下的没有使用的共享内存块放在一起
	if(nIndex <= (uint32)m_u2Count)
	{
		for(ID_t i = nIndex; i <= (ID_t)m_u2Count; i++)
		{
			_UserValid* pUserValid = (_UserValid* )m_SMOption.GetBuffer(i);
			if(NULL != pUserValid)
			{
				pUserValid->m_blDelete     = true;
				pUserValid->m_blOnline     = false;
				pUserValid->m_u1State      = CHECKS_UNHIT;
				pUserValid->m_u4LoginCount = 0;
				m_vecFreeValid.push_back(pUserValid);
			}
		}
	}

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);

	m_SMOption.SetMemoryState(READERINITSTATED);

	return true;
}

bool CUserValidManager::GetFileInfo( const char* pLine, char* pUserName, char* pUserPass )
{
	bool blRet = false;

	int nSplit = 0;
	int nLineSize = (int)strlen(pLine);
	for(int i = 0; i < nLineSize; i++)
	{
		if(pLine[i] == ',')
		{
			nSplit = i;
		}
	}

	if(pLine[0] != 13)
	{
		ACE_OS::memcpy(pUserName, pLine, nSplit);
		pUserName[nSplit] = '\0';
		ACE_OS::memcpy(pUserPass, &pLine[nSplit + 1], nLineSize - nSplit - 1);
		pUserPass[nLineSize - nSplit - 1] = '\0';
	}
	else
	{
		ACE_OS::memcpy(pUserName, pLine + 1, nSplit - 1);
		pUserName[nSplit - 1] = '\0';
		ACE_OS::memcpy(pUserPass, &pLine[nSplit + 1], nLineSize - nSplit - 1);
		pUserPass[nLineSize - nSplit - 1] = '\0';
	}


	if(nSplit > 0)
	{
		blRet = true;
	}

	return blRet;
}

void CUserValidManager::Close()
{
	m_mapUserValid.clear();
	m_vecFreeValid.clear();
}

bool CUserValidManager::ReadMemory()
{
	m_mapUserValid.clear();
	m_vecFreeValid.clear();

	for(int i = 1; i <= m_u2Count; i++)
	{
		_UserValid* pUserValid = (_UserValid* )m_SMOption.GetBuffer(i);
		if(NULL != pUserValid)
		{
			//符合条件的数据才插入当前遍历列表
			if(ACE_OS::strlen(pUserValid->m_szUserName) != 0 
				&& ACE_OS::strlen(pUserValid->m_szUserPass) != 0
				&& pUserValid->m_blDelete == false)
			{
				string strUserName = (string)pUserValid->m_szUserName;
				m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));
			}
			else
			{
				//属于空闲的数据块，存在一起
				m_vecFreeValid.push_back(pUserValid);
			}
		}
	}

#if 0
	//测试代码
	string strData = "rabbit";
	mapUserValid::iterator f = m_mapUserValid.find(strData);
	if(f == m_mapUserValid.end())
	{
		OUR_DEBUG((LM_INFO, "no find.\n"));
	}
	else
	{
		OUR_DEBUG((LM_INFO, "find.\n"));
	}

#endif
	return true;
}

_UserValid* CUserValidManager::GetUserValid( const char* pUserName )
{
	string strUserName = (string)pUserName;
	mapUserValid::iterator f = m_mapUserValid.find(strUserName);
	if(f == m_mapUserValid.end())
	{
		return NULL;
	}
	else
	{
		string strMapUserName = (string)f->first;
		_UserValid* pUserValid = (_UserValid* )f->second;
		if(NULL != pUserValid)
		{
			if(strUserName != (string)pUserValid->m_szUserName || pUserValid->m_blDelete == true)
			{
				//说明当前数据已经无效了，从当前map里面清除
				m_mapUserValid.erase(f);
				m_vecFreeValid.push_back(pUserValid);

				//如果新数据不是删除的数据，重新添加新加载的数据
				if(pUserValid->m_blDelete == false)
				{
					m_mapUserValid.insert(mapUserValid::value_type((string)pUserValid->m_szUserName, pUserValid));
				}
				return NULL;
			}
			else
			{
				if(pUserValid->m_blDelete == false)
				{
					return pUserValid;
				}
				else
				{
					return NULL;
				}
				
			}
		}
		else
		{
			return NULL;
		}

	}
}

void CUserValidManager::Check_File2Memory()
{
	bool m_blUpdate = false; 
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_PATH);
	FILE* pFile = fopen((char* )szFileName, "r");
	if(pFile == NULL)
	{
		//文件不存在
		return;
	}

	fseek(pFile, 0l, SEEK_END);

	uint32 u4FileSize = ftell(pFile);

	if(u4FileSize == 0)
	{
		//文件长度为0
		return;
	}

	fclose(pFile);

	//m_mapUserValid.clear();
	//m_vecFreeValid.clear();

	pFile = fopen((char* )szFileName, "r");
	fseek(pFile, 0l, SEEK_SET);

	char* pFileBuffer = new char[u4FileSize + 1];
	memset(pFileBuffer, 0, u4FileSize);

	//读取文件,一次读不完循环读，直到全部读完为止
	uint32 u4ReadSize = (uint32)fread((char* )pFileBuffer, sizeof(char), u4FileSize, pFile);
	if(u4ReadSize >= u4FileSize)
	{
		//读完了
	}
	else if(u4ReadSize == 0)
	{
		//读取文件失败了
		SAFE_DELETE_ARRAY(pFileBuffer);
		fclose(pFile);
		return;
	}

	char szUserName[MAX_BUFF_50] = {'\0'};
	char szUserPass[MAX_BUFF_50] = {'\0'};

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	BeginCheck();

	while(pLine != NULL) 
	{
		bool blState = GetFileInfo(pLine, szUserName, szUserPass);
		if(true == blState)
		{
			_UserValid* pUserValid = (_UserValid* )GetUserValid(szUserName);
			if(NULL != pUserValid)
			{
				//初始化共享内存数据
				sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
				sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
				pUserValid->m_blDelete     = false;
				pUserValid->m_u1State      = CHECKS_HIT;
				pUserValid->m_u4LoginCount = 0;
			}
			else
			{
				//如果没有命中，则是新数据，从空闲池里取出数据放在里面
				if(m_vecFreeValid.size() <= 0)
				{
					continue;
				}

				_UserValid* pUserValid = (_UserValid* )m_vecFreeValid[0];
				if(NULL != pUserValid)
				{
					//初始化共享内存数据
					sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
					sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
					pUserValid->m_blDelete     = false;
					pUserValid->m_blOnline     = false;
					pUserValid->m_u1State      = CHECKS_HIT;
					pUserValid->m_u4LoginCount = 0;

					string strUserName;
					strUserName = (string)pUserValid->m_szUserName;

					m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));

					vecValid::iterator b = m_vecFreeValid.begin();
					m_vecFreeValid.erase(b);
				}
			}
		}
		pLine = strtok(NULL, szFind);
	}

	EndCheck();

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);
}

void CUserValidManager::BeginCheck()
{
	for(mapUserValid::iterator b = m_mapUserValid.begin(); b != m_mapUserValid.end(); b++)
	{
		_UserValid* pUserValid = (_UserValid* )b->second;
		if(NULL != pUserValid)
		{
			pUserValid->m_u1State = CHECKS_UNHIT;
		}
	}
}

void CUserValidManager::EndCheck()
{
	for(mapUserValid::iterator b = m_mapUserValid.begin(); b != m_mapUserValid.end();)
	{
		_UserValid* pUserValid = (_UserValid* )b->second;
		if(NULL != pUserValid)
		{
			if(pUserValid->m_u1State != CHECKS_HIT)
			{
				//说明这个数据已经在遍历中不存在了，回收之
				pUserValid->m_blDelete = true;
				m_mapUserValid.erase(b++);

				m_vecFreeValid.push_back(pUserValid);
			}
			else
			{
				b++;
			}
		}
	}

	OUR_DEBUG((LM_INFO, "[CUserValidManager::EndCheck]map is(%d), free is (%d).\n", m_mapUserValid.size(), m_vecFreeValid.size()));
}

void CUserValidManager::GetFreeValid()
{
	//如果当前map不存在，则去空闲内存块寻找，有可能从后台加载数据已经加载进来了。
	//这里的map和Watch的监控是不一致的，如果当前找不到才去当前以为空闲的去寻找。
	for(uint32 i = 0; i < (uint32)m_vecFreeValid.size(); i++)
	{
		if(m_vecFreeValid[i]->m_blDelete == false)
		{
			mapUserValid::iterator f = m_mapUserValid.find((string)m_vecFreeValid[i]->m_szUserName);
			if(f == m_mapUserValid.end())
			{
				m_mapUserValid.insert(mapUserValid::value_type((string)m_vecFreeValid[i]->m_szUserName, (_UserValid* )m_vecFreeValid[i]));
			}
		}
	}

	for(vecValid::iterator b = m_vecFreeValid.begin(); b != m_vecFreeValid.end();)
	{
		_UserValid* pUserValid = (_UserValid* )(*b);
		if(pUserValid->m_blDelete == false)
		{
			b = m_vecFreeValid.erase(b);
		}
		else
		{
			b++;
		}
	}
}
