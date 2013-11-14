#include "UserInfoManager.h"

CUserInfoManager::CUserInfoManager()
{

}

CUserInfoManager::~CUserInfoManager()
{

}

void CUserInfoManager::Close()
{
	m_mapUserInfo.clear();
	m_vecFreeUserInfo.clear();
}

void CUserInfoManager::Display()
{
	OUR_DEBUG((LM_INFO, "[CUserInfoManager::Display]m_mapUserInfo count=%d, m_vecFreeUserInfo=%d.\n", m_mapUserInfo.size(), m_vecFreeUserInfo.size()));
	for(mapUserInfo::iterator b = m_mapUserInfo.begin(); b!= m_mapUserInfo.end(); b++)
	{
		_UserInfo* pUserInfo = (_UserInfo* )b->second;

		OUR_DEBUG((LM_INFO, "[CUserValidManager::Display]UserID=%d, life=%d, magic=%d.\n", pUserInfo->m_u4UserID, pUserInfo->m_u4Life, pUserInfo->m_u4Magic));
	}
}

bool CUserInfoManager::Init( uint32 u4CachedCount, key_t objMemorykey, uint32 u4CheckSize )
{
	//����Lru��������
	m_objLRU.Set_Lru_Max_Count(u4CachedCount);

	//���ø��෽��
	return CCacheManager::Init(u4CachedCount, objMemorykey, u4CheckSize);
}

_UserInfo* CUserInfoManager::GetUserInfo( uint32 u4UserID )
{
	mapUserInfo::iterator f = m_mapUserInfo.find(u4UserID);
	if(f == m_mapUserInfo.end())
	{
		return NULL;
	}
	else
	{
		uint32 u4UserID = (uint32)f->first;
		_UserInfo* pUserInfo = (_UserInfo* )f->second;
		if(NULL != pUserInfo)
		{
			if(u4UserID != (uint32)pUserInfo->m_u4UserID || pUserInfo->GetDelete() == true)
			{
				//˵����ǰ�����Ѿ���Ч�ˣ��ӵ�ǰmap�������
				m_mapUserInfo.erase(f);
				m_vecFreeUserInfo.push_back(pUserInfo);

				//ɾ��Lru�������
				m_objLRU.Delete_Cached_Lru(u4UserID);

				//��������ݲ���ɾ�������ݣ���������¼��ص�����
				if(pUserInfo->GetDelete() == false)
				{
					m_mapUserInfo.insert(mapUserInfo::value_type((uint32)pUserInfo->m_u4UserID, pUserInfo));
				}
				return NULL;
			}
			else
			{
				//�������
				m_objLRU.Add_Cached_Lru(u4UserID, pUserInfo->GetCacheIndex());

				return pUserInfo;
			}
		}
		else
		{
			return NULL;
		}
	}
}

void CUserInfoManager::Sync_Memory_To_DataReaource()
{
	//ɾ������Դ
	remove(SOURCE_FILE_INFO_PATH);

	//����ͬ�����ļ�
	FILE* pFile = fopen((char* )SOURCE_FILE_INFO_PATH, "ab+");
	if(pFile == NULL)
	{
		//�ļ�������
		return;
	}

	for(mapUserInfo::iterator b = m_mapUserInfo.begin(); b != m_mapUserInfo.end(); b++)
	{
		WriteFileInfo(pFile, (_UserInfo* )b->second);
	}

	for(uint32 i = 0; i < m_vecFreeUserInfo.size(); i++)
	{
		if(m_vecFreeUserInfo[i]->m_u4UserID > 0)
		{
			WriteFileInfo(pFile, (_UserInfo* )m_vecFreeUserInfo[i]);
		}
	}

	fclose(pFile);
}

bool CUserInfoManager::WriteFileInfo( FILE* pFile, _UserInfo* pUserInfo )
{
	char szUserInfoText[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szUserInfoText, MAX_BUFF_200, "%d,%d,%d;\n", pUserInfo->m_u4UserID, pUserInfo->m_u4Life, pUserInfo->m_u4Magic);
	fwrite(szUserInfoText, sizeof(char), strlen(szUserInfoText), pFile);
	return true;
}

void CUserInfoManager::GetFreeUserInfo()
{
	//�����ǰmap�����ڣ���ȥ�����ڴ��Ѱ�ң��п��ܴӺ�̨���������Ѿ����ؽ����ˡ�
	//�����map��Watch�ļ���ǲ�һ�µģ������ǰ�Ҳ�����ȥ��ǰ��Ϊ���е�ȥѰ�ҡ�
	for(uint32 i = 0; i < (uint32)m_vecFreeUserInfo.size(); i++)
	{
		if(m_vecFreeUserInfo[i]->GetDelete() == false)
		{
			mapUserInfo::iterator f = m_mapUserInfo.find(m_vecFreeUserInfo[i]->m_u4UserID);
			if(f == m_mapUserInfo.end())
			{
				m_mapUserInfo.insert(mapUserInfo::value_type(m_vecFreeUserInfo[i]->m_u4UserID, (_UserInfo* )m_vecFreeUserInfo[i]));
			}
		}
	}

	for(vecUserInfo::iterator b = m_vecFreeUserInfo.begin(); b != m_vecFreeUserInfo.end();)
	{
		_UserInfo* pUserInfo = (_UserInfo* )(*b);
		if(pUserInfo->GetDelete() == false)
		{
			b = m_vecFreeUserInfo.erase(b);
		}
		else
		{
			b++;
		}
	}
}

bool CUserInfoManager::Load_From_DataResouce( uint32 u4SeachUserID, uint32& u4CacheIndex )
{
	u4CacheIndex = 0;
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_INFO_PATH);
	FILE* pFile = fopen((char* )szFileName, "r");
	if(pFile == NULL)
	{
		//�ļ�������
		return false;
	}

	fseek(pFile, 0l, SEEK_END);

	uint32 u4FileSize = ftell(pFile);

	if(u4FileSize == 0)
	{
		//�ļ�����Ϊ0
		return false;
	}

	fclose(pFile);

	pFile = fopen((char* )szFileName, "r");
	fseek(pFile, 0l, SEEK_SET);

	char* pFileBuffer = new char[u4FileSize + 1];
	memset(pFileBuffer, 0, u4FileSize);

	//��ȡ�ļ�,һ�ζ�����ѭ������ֱ��ȫ������Ϊֹ
	uint32 u4ReadSize = (uint32)fread((char* )pFileBuffer, sizeof(char), u4FileSize, pFile);
	if(u4ReadSize >= u4FileSize)
	{
		//������
	}
	else if(u4ReadSize == 0)
	{
		//��ȡ�ļ�ʧ����
		SAFE_DELETE_ARRAY(pFileBuffer);
		fclose(pFile);
		return false;
	}

	uint32 u4UserID    = 0;
	uint32 u4UserLife  = 0;
	uint32 u4UserMagic = 0;

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	while(pLine != NULL) 
	{
		bool blState = GetFileInfo(pLine, u4UserID, u4UserLife, u4UserMagic);

		//������Դ��Ѱ��ָ�����û����������ڹ����ڴ���
		if(true == blState && u4SeachUserID == u4UserID)
		{
			_UserInfo* pUserInfo = (_UserInfo* )GetUserInfo(u4UserID);
			if(NULL != pUserInfo)
			{
				//��ʼ�������ڴ�����
				pUserInfo->m_u4UserID = u4UserID;
				pUserInfo->m_u4Life   = u4UserLife;
				pUserInfo->m_u4Magic  = u4UserLife;
				pUserInfo->SetHit();
			}
			else
			{
				//���û�����У����������ݣ��ӿ��г���ȡ�����ݷ�������
				if(m_vecFreeUserInfo.size() <= 0)
				{
					//������г��Ѿ��������ˣ���̭�ʱ��δ�����ʵ�����
					uint32 u4DeleteUserID = 0;
					bool blIsLast = m_objLRU.Check_Cached_Lru(u4DeleteUserID);
					if(blIsLast == false)
					{
						//û���ҵ�����̭�����ݣ�����϶��ǲ��Ϸ��ġ�
						SAFE_DELETE_ARRAY(pFileBuffer);
						fclose(pFile);
						return false;
					}

					mapUserInfo::iterator f = m_mapUserInfo.find(u4DeleteUserID);
					if(f == m_mapUserInfo.end())
					{
						SAFE_DELETE_ARRAY(pFileBuffer);
						fclose(pFile);
						return false;
					}
					else
					{
						_UserInfo* pUserInfo = (_UserInfo* )f->second;

						//���������LRU�滻֮
						m_objLRU.Delete_Cached_Lru(u4DeleteUserID);
						m_objLRU.Add_Cached_Lru(u4UserID, pUserInfo->GetCacheIndex());

						m_mapUserInfo.erase(f);

						//��ʼ�������ڴ�����
						pUserInfo->m_u4UserID = u4UserID;
						pUserInfo->m_u4Life   = u4UserLife;
						pUserInfo->m_u4Magic  = u4UserLife;
						pUserInfo->SetHit();

						m_mapUserInfo.insert(mapUserInfo::value_type(pUserInfo->m_u4UserID, pUserInfo));
						u4CacheIndex = pUserInfo->GetCacheIndex();
					}
				}
				else
				{
					_UserInfo* pUserInfo = (_UserInfo* )m_vecFreeUserInfo[0];
					if(NULL != pUserInfo)
					{
						//��ʼ�������ڴ�����
						pUserInfo->m_u4UserID = u4UserID;
						pUserInfo->m_u4Life   = u4UserLife;
						pUserInfo->m_u4Magic  = u4UserLife;
						pUserInfo->SetHit();

						m_mapUserInfo.insert(mapUserInfo::value_type(pUserInfo->m_u4UserID, pUserInfo));

						vecUserInfo::iterator b = m_vecFreeUserInfo.begin();
						m_vecFreeUserInfo.erase(b);
					}
				}
			}

			break;
		}
		pLine = strtok(NULL, szFind);
	}

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);

	Display();

	return true;
}

bool CUserInfoManager::GetFileInfo( const char* pLine, uint32& u4UserID, uint32& u4Life, uint32& u4Magic )
{
	bool blRet = false;

	char szTemp[MAX_BUFF_200] = {'\0'};

	if(pLine[0] != 13)
	{
		int nIndex    = 0;
		int nBeginPos = 0;
		int nEndPos   = 0;
		for(int i = 0; i < (int)strlen(pLine); i++)
		{
			if(pLine[i]  == ',')
			{
				if(nIndex == 0)
				{
					//�û�ID
					ACE_OS::memcpy(szTemp, &pLine[nBeginPos], nEndPos - nBeginPos);
					szTemp[nEndPos - nBeginPos] = '\0';
					u4UserID = ACE_OS::atoi(szTemp);
					nBeginPos = i + 1;
					nIndex++;
				}
				else if(nIndex == 1)
				{
					//Life
					ACE_OS::memcpy(szTemp, &pLine[nBeginPos], nEndPos - nBeginPos);
					szTemp[nEndPos - nBeginPos] = '\0';
					u4Life = ACE_OS::atoi(szTemp);
					nBeginPos = i + 1;
					nIndex++;
				}
			}

			nEndPos++;
		}

		if(nIndex == 2)
		{
			//Magic
			ACE_OS::memcpy(szTemp, &pLine[nBeginPos], (int)strlen(pLine) - nBeginPos);
			szTemp[(int)strlen(pLine) - nBeginPos] = '\0';
			u4Magic = ACE_OS::atoi(szTemp);
			blRet = true;
		}
	}
	else
	{
		int nIndex    = 0;
		int nBeginPos = 1;
		int nEndPos   = 0;
		for(int i = 1; i < (int)strlen(pLine); i++)
		{
			if(pLine[i]  == ',')
			{
				if(nIndex == 0)
				{
					//�û�ID
					ACE_OS::memcpy(szTemp, &pLine[nBeginPos], nEndPos - nBeginPos);
					szTemp[nEndPos - nBeginPos] = '\0';
					u4UserID = ACE_OS::atoi(szTemp);
					nBeginPos = i + 1;
					nIndex++;
				}
				else if(nIndex == 1)
				{
					//Life
					ACE_OS::memcpy(szTemp, &pLine[nBeginPos], nEndPos - nBeginPos);
					szTemp[nEndPos - nBeginPos] = '\0';
					u4Life = ACE_OS::atoi(szTemp);
					nBeginPos = i + 1;
					nIndex++;
				}
			}

			nEndPos++;
		}

		if(nIndex == 2)
		{
			//Magic
			ACE_OS::memcpy(szTemp, &pLine[nBeginPos], (int)strlen(pLine) - nBeginPos - 1);
			szTemp[(int)strlen(pLine) - nBeginPos - 1] = '\0';
			u4Magic = ACE_OS::atoi(szTemp);
			blRet = true;
		}
	}

	return blRet;
}

bool CUserInfoManager::Reload_Map_CacheMemory( uint32 u4CacheIndex )
{
	uint32 u4OldUserID = 0;
	uint32 u4NewUserID = 0;

	bool blRet = m_objLRU.Get_Cached_KeyByIndex(u4CacheIndex, u4OldUserID);

	mapUserInfo::iterator f = m_mapUserInfo.find(u4OldUserID);
	if(f != m_mapUserInfo.end())
	{
		_UserInfo* pUserInfo = (_UserInfo* )f->second;

		//����Index�б��Ӧ��ϵ
		m_objLRU.Reload_Cached_IndexList((uint32)pUserInfo->m_u4UserID, u4OldUserID, u4CacheIndex);

		m_mapUserInfo.erase(f);
		m_mapUserInfo.insert(mapUserInfo::value_type((uint32)pUserInfo->m_u4UserID, pUserInfo));
		return true;
	}
	else
	{
		return false;
	}
}

bool CUserInfoManager::Read_All_Init_DataResoure()
{
	//���ļ������ȡ��ǰ����֧�ֵ��û��������룬���빲���ڴ���ȥ
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_INFO_PATH);
	FILE* pFile = fopen((char* )szFileName, "r");
	if(pFile == NULL)
	{
		//�ļ�������
		return false;
	}

	fseek(pFile, 0l, SEEK_END);

	uint32 u4FileSize = ftell(pFile);

	if(u4FileSize == 0)
	{
		//�ļ�����Ϊ0
		return false;
	}

	fclose(pFile);

	m_mapUserInfo.clear();
	m_vecFreeUserInfo.clear();

	pFile = fopen((char* )szFileName, "r");
	fseek(pFile, 0l, SEEK_SET);

	char* pFileBuffer = new char[u4FileSize + 1];
	memset(pFileBuffer, 0, u4FileSize);

	//��ȡ�ļ�,һ�ζ�����ѭ������ֱ��ȫ������Ϊֹ
	uint32 u4ReadSize = (uint32)fread((char* )pFileBuffer, sizeof(char), u4FileSize, pFile);
	if(u4ReadSize >= u4FileSize)
	{
		//������
	}
	else if(u4ReadSize == 0)
	{
		//��ȡ�ļ�ʧ����
		SAFE_DELETE_ARRAY(pFileBuffer);
		fclose(pFile);
		return false;
	}

	ID_t nIndex = 1;

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	while(pLine != NULL) 
	{
		if(nIndex > (uint32)Get_Cache_Count())
		{
			break;
		}

		uint32 u4UserID = 0;
		uint32 u4Life   = 0;
		uint32 u4Magic  = 0;

		bool blState = GetFileInfo(pLine, u4UserID, u4Life, u4Magic);
		if(true == blState)
		{
			_UserInfo* pUserInfo = (_UserInfo* )Get_CacheBlock_By_Index(nIndex);
			if(NULL != pUserInfo)
			{
				//��ʼ����ID
				pUserInfo->SetCacheIndex(nIndex);

				//��ʼ�������ڴ�����
				pUserInfo->m_u4UserID = u4UserID;
				pUserInfo->m_u4Life   = u4Life;
				pUserInfo->m_u4Magic  = u4Magic;

				//����Lru�б�
				//���ﲻ�ü��Lru�Ƿ���������Ϊ�ǵ�һ�μ���
				//���ᳬ��Get_Cache_Count()����
				m_objLRU.Add_Cached_Lru(u4UserID, pUserInfo->GetCacheIndex());

				pUserInfo->SetHit();

				m_mapUserInfo.insert(mapUserInfo::value_type(u4UserID, pUserInfo));

				nIndex++;
			}
		}
		pLine = strtok(NULL, szFind);
	}

	//ʣ�µ�û��ʹ�õĹ����ڴ�����һ��
	if(nIndex <= (uint32)Get_Cache_Count())
	{
		for(ID_t i = nIndex; i <= (ID_t)Get_Cache_Count(); i++)
		{
			_UserInfo* pUserInfo = (_UserInfo* )Get_CacheBlock_By_Index(i);
			if(NULL != pUserInfo)
			{
				pUserInfo->SetUnHit();

				m_vecFreeUserInfo.push_back(pUserInfo);
			}
		}
	}

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);

	Set_Memory_Init_Success();

	return true;
}

bool CUserInfoManager::Read_All_From_CacheMemory()
{
	m_mapUserInfo.clear();
	m_vecFreeUserInfo.clear();

	for(uint32 i = 1; i <= Get_Cache_Count(); i++)
	{
		_UserInfo* pUserInfo = (_UserInfo* )Get_CacheBlock_By_Index(i);
		if(NULL != pUserInfo)
		{
			//�������������ݲŲ��뵱ǰ�����б�
			if(pUserInfo->m_u4UserID > 0
				&& pUserInfo->GetDelete() == false)
			{
				//����Lru�б�
				//���ﲻ�ü��Lru�Ƿ���������Ϊ�ǵ�һ�δ������ڴ����
				//���ᳬ��Get_Cache_Count()����
				m_objLRU.Add_Cached_Lru(pUserInfo->m_u4UserID, pUserInfo->GetCacheIndex());

				m_mapUserInfo.insert(mapUserInfo::value_type(pUserInfo->m_u4UserID, pUserInfo));
			}
			else
			{
				//���ڿ��е����ݿ飬����һ��
				m_vecFreeUserInfo.push_back(pUserInfo);
			}
		}
	}

	return true;
}

void CUserInfoManager::Begin_Sync_DataReaource_To_Memory()
{
	for(mapUserInfo::iterator b = m_mapUserInfo.begin(); b != m_mapUserInfo.end(); b++)
	{
		_UserInfo* pUserInfo = (_UserInfo* )b->second;
		if(NULL != pUserInfo)
		{
			pUserInfo->SetCheckState(CHECKS_UNHIT);
		}
	}
}

void CUserInfoManager::End_Sync_DataReaource_To_Memory()
{
	for(mapUserInfo::iterator b = m_mapUserInfo.begin(); b != m_mapUserInfo.end();)
	{
		_UserInfo* pUserInfo = (_UserInfo* )b->second;
		if(NULL != pUserInfo)
		{
			if(pUserInfo->GetCheckState() != CHECKS_HIT)
			{
				//˵����������Ѿ��ڱ����в������ˣ�����֮
				pUserInfo->SetDelete(true);

				//��Lru��ɾ��
				m_objLRU.Delete_Cached_Lru((uint32)pUserInfo->m_u4UserID);

				m_mapUserInfo.erase(b++);
				m_vecFreeUserInfo.push_back(pUserInfo);
			}
			else
			{
				b++;
			}
		}
	}

	OUR_DEBUG((LM_INFO, "[CUserValidManager::EndCheck]map is(%d), free is (%d).\n", m_mapUserInfo.size(), m_vecFreeUserInfo.size()));
}