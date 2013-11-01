#include "UserValidManager.h"

CUserValidManager::CUserValidManager()
{
}

CUserValidManager::~CUserValidManager()
{
	Close();
}

bool CUserValidManager::Read_All_Init_DataResoure()
{
	//���ļ������ȡ��ǰ����֧�ֵ��û��������룬���빲���ڴ���ȥ
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_PATH);
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

	m_mapUserValid.clear();
	m_vecFreeValid.clear();

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
	char szUserName[MAX_BUFF_50] = {'\0'};
	char szUserPass[MAX_BUFF_50] = {'\0'};

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	while(pLine != NULL) 
	{
		if(nIndex > (uint32)Get_Cache_Count())
		{
			break;
		}

		bool blState = GetFileInfo(pLine, szUserName, szUserPass);
		if(true == blState)
		{
			_UserValid* pUserValid = (_UserValid* )Get_CacheBlock_By_Index(nIndex);
			if(NULL != pUserValid)
			{
				//��ʼ�������ڴ�����
				sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
				sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
				
				pUserValid->m_u4LoginCount  = 0;
				pUserValid->SetHit();

				string strUserName;
				strUserName = (string)pUserValid->m_szUserName;

				m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));

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
			_UserValid* pUserValid = (_UserValid* )Get_CacheBlock_By_Index(i);
			if(NULL != pUserValid)
			{
				pUserValid->m_u4LoginCount                 = 0;
				pUserValid->SetUnHit();

				m_vecFreeValid.push_back(pUserValid);
			}
		}
	}

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);

	Set_Memory_Init_Success();

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

bool CUserValidManager::Read_All_From_CacheMemory()
{
	m_mapUserValid.clear();
	m_vecFreeValid.clear();

	for(uint32 i = 1; i <= Get_Cache_Count(); i++)
	{
		_UserValid* pUserValid = (_UserValid* )Get_CacheBlock_By_Index(i);
		if(NULL != pUserValid)
		{
			//�������������ݲŲ��뵱ǰ�����б�
			if(ACE_OS::strlen(pUserValid->m_szUserName) != 0 
				&& ACE_OS::strlen(pUserValid->m_szUserPass) != 0
				&& pUserValid->GetDelete() == false)
			{
				string strUserName = (string)pUserValid->m_szUserName;
				m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));
			}
			else
			{
				//���ڿ��е����ݿ飬����һ��
				m_vecFreeValid.push_back(pUserValid);
			}
		}
	}

#if 0
	//���Դ���
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
			if(strUserName != (string)pUserValid->m_szUserName || pUserValid->GetDelete() == true)
			{
				//˵����ǰ�����Ѿ���Ч�ˣ��ӵ�ǰmap�������
				m_mapUserValid.erase(f);
				m_vecFreeValid.push_back(pUserValid);

				//��������ݲ���ɾ�������ݣ����������¼��ص�����
				if(pUserValid->GetDelete() == false)
				{
					m_mapUserValid.insert(mapUserValid::value_type((string)pUserValid->m_szUserName, pUserValid));
				}
				return NULL;
			}
			else
			{
				if(pUserValid->GetDelete() == false)
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

void CUserValidManager::Sync_DataReaource_To_Memory()
{
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_PATH);
	FILE* pFile = fopen((char* )szFileName, "r");
	if(pFile == NULL)
	{
		//�ļ�������
		return;
	}

	fseek(pFile, 0l, SEEK_END);

	uint32 u4FileSize = ftell(pFile);

	if(u4FileSize == 0)
	{
		//�ļ�����Ϊ0
		return;
	}

	fclose(pFile);

	//m_mapUserValid.clear();
	//m_vecFreeValid.clear();

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
		return;
	}

	char szUserName[MAX_BUFF_50] = {'\0'};
	char szUserPass[MAX_BUFF_50] = {'\0'};

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	Begin_Sync_DataReaource_To_Memory();

	while(pLine != NULL) 
	{
		bool blState = GetFileInfo(pLine, szUserName, szUserPass);
		if(true == blState)
		{
			_UserValid* pUserValid = (_UserValid* )GetUserValid(szUserName);
			if(NULL != pUserValid)
			{
				//��ʼ�������ڴ�����
				sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
				sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
				pUserValid->SetHit();
			}
			else
			{
				//���û�����У����������ݣ��ӿ��г���ȡ�����ݷ�������
				if(m_vecFreeValid.size() <= 0)
				{
					continue;
				}

				_UserValid* pUserValid = (_UserValid* )m_vecFreeValid[0];
				if(NULL != pUserValid)
				{
					//��ʼ�������ڴ�����
					sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
					sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
					
					pUserValid->m_u4LoginCount                 = 0;
					pUserValid->SetHit();

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

	End_Sync_DataReaource_To_Memory();

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);
}

void CUserValidManager::Begin_Sync_DataReaource_To_Memory()
{
	for(mapUserValid::iterator b = m_mapUserValid.begin(); b != m_mapUserValid.end(); b++)
	{
		_UserValid* pUserValid = (_UserValid* )b->second;
		if(NULL != pUserValid)
		{
			pUserValid->SetCheckState(CHECKS_UNHIT);
		}
	}
}

void CUserValidManager::End_Sync_DataReaource_To_Memory()
{
	for(mapUserValid::iterator b = m_mapUserValid.begin(); b != m_mapUserValid.end();)
	{
		_UserValid* pUserValid = (_UserValid* )b->second;
		if(NULL != pUserValid)
		{
			if(pUserValid->GetCheckState() != CHECKS_HIT)
			{
				//˵����������Ѿ��ڱ����в������ˣ�����֮
				pUserValid->SetDelete(true);
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
	//�����ǰmap�����ڣ���ȥ�����ڴ��Ѱ�ң��п��ܴӺ�̨���������Ѿ����ؽ����ˡ�
	//�����map��Watch�ļ���ǲ�һ�µģ������ǰ�Ҳ�����ȥ��ǰ��Ϊ���е�ȥѰ�ҡ�
	for(uint32 i = 0; i < (uint32)m_vecFreeValid.size(); i++)
	{
		if(m_vecFreeValid[i]->GetDelete() == false)
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
		if(pUserValid->GetDelete() == false)
		{
			b = m_vecFreeValid.erase(b);
		}
		else
		{
			b++;
		}
	}
}

bool CUserValidManager::Load_From_DataResouce( const char* pUserName )
{
	char szFileName[MAX_BUFF_200] = {'\0'};
	sprintf_safe(szFileName, MAX_BUFF_200, "%s", SOURCE_FILE_PATH);
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

	//m_mapUserValid.clear();
	//m_vecFreeValid.clear();

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

	char szUserName[MAX_BUFF_50] = {'\0'};
	char szUserPass[MAX_BUFF_50] = {'\0'};

	char szFind[2] = {'\0'};
	sprintf_safe(szFind, 2, ";");

	char* pLine = strtok((char* )pFileBuffer, szFind);

	while(pLine != NULL) 
	{
		bool blState = GetFileInfo(pLine, szUserName, szUserPass);
		
		//������Դ��Ѱ��ָ�����û����������ڹ����ڴ���
		if(true == blState && ACE_OS::strcmp(szUserName, pUserName) == 0)
		{
			_UserValid* pUserValid = (_UserValid* )GetUserValid(szUserName);
			if(NULL != pUserValid)
			{
				//��ʼ�������ڴ�����
				sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
				sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
				pUserValid->SetHit();
				pUserValid->m_u4LoginCount                 = 0;
			}
			else
			{
				//���û�����У����������ݣ��ӿ��г���ȡ�����ݷ�������
				if(m_vecFreeValid.size() <= 0)
				{
					SAFE_DELETE_ARRAY(pFileBuffer);
					fclose(pFile);
					return false;
				}

				_UserValid* pUserValid = (_UserValid* )m_vecFreeValid[0];
				if(NULL != pUserValid)
				{
					//��ʼ�������ڴ�����
					sprintf_safe(pUserValid->m_szUserName, MAX_BUFF_50, "%s", szUserName);
					sprintf_safe(pUserValid->m_szUserPass, MAX_BUFF_50, "%s", szUserPass);
					pUserValid->SetHit();
					pUserValid->m_u4LoginCount                 = 0;

					string strUserName;
					strUserName = (string)pUserValid->m_szUserName;

					m_mapUserValid.insert(mapUserValid::value_type(strUserName, pUserValid));

					vecValid::iterator b = m_vecFreeValid.begin();
					m_vecFreeValid.erase(b);
				}
			}

			break;
		}
		pLine = strtok(NULL, szFind);
	}

	SAFE_DELETE_ARRAY(pFileBuffer);
	fclose(pFile);

	return true;
}