#include "ForbiddenIP.h"

CForbiddenIP::CForbiddenIP()
{

}

CForbiddenIP::~CForbiddenIP()
{
	OUR_DEBUG((LM_INFO, "[CForbiddenIP::~CForbiddenIP].\n"));
}

bool CForbiddenIP::Init(const char* szConfigPath)
{
	OUR_DEBUG((LM_INFO, "[CForbiddenIP::Init]Filename = %s.\n", szConfigPath));
	if(!m_AppConfig.ReadConfig(szConfigPath))
	{
		OUR_DEBUG((LM_INFO, "[CForbiddenIP::Init]Read Filename = %s error.\n", szConfigPath));
		return false;
	}

	m_VecForeverForbiddenIP.clear();
	m_VecTempForbiddenIP.clear();

	ACE_TString strValue;
	char szName1[MAX_BUFF_20]       = {'\0'};
	char szIP[MAX_BUFF_20]          = {'\0'};
	char szConnectType[MAX_BUFF_20] = {'\0'};

	_ForbiddenIP ForbiddenIP;

	//获得反应器个数
	m_AppConfig.GetValue("IPCount", strValue, "\\ForbiddenIP");
	int nCount = ACE_OS::atoi((char*)strValue.c_str());

	for(int i = 0; i < nCount; i++)
	{
		sprintf_safe(szName1, MAX_BUFF_20, "IP%d", i);
		m_AppConfig.GetValue(szName1, strValue, "\\ForbiddenIP");

		if(true == ParseTXT(strValue.c_str(), szIP, szConnectType))
		{
			sprintf_safe(ForbiddenIP.m_szClientIP, MAX_BUFF_20, "%s", szIP);
			if(ACE_OS::strcmp(szConnectType, "TCP") == 0)
			{
				ForbiddenIP.m_u1ConnectType = CONNECT_TCP;
			}
			else
			{
				ForbiddenIP.m_u1ConnectType = CONNECT_UDP;
			}
		}		

		m_VecForeverForbiddenIP.push_back(ForbiddenIP);
	}

	return true;
}

bool CForbiddenIP::CheckIP(const char* pIP, uint8 u1ConnectType)
{
	for(int i = 0; i < (int)m_VecForeverForbiddenIP.size(); i++)
	{
		if(ACE_OS::strcmp(pIP, m_VecForeverForbiddenIP[i].m_szClientIP) == 0 && m_VecForeverForbiddenIP[i].m_u1ConnectType == u1ConnectType)
		{
			return false;
		}
	}

	for(VecForbiddenIP::iterator b = m_VecTempForbiddenIP.begin(); b != m_VecTempForbiddenIP.end(); b++)
	{
		if(ACE_OS::strcmp(pIP, (*b).m_szClientIP) == 0 && (*b).m_u1ConnectType == u1ConnectType)
		{
			//如果是禁止时间段内，则返回false，否则删除定时信息。
			if((*b).m_tvBegin.sec() + (*b).m_u4Second > (uint32)ACE_OS::gettimeofday().sec())
			{
				return false;
			}
			else
			{
				m_VecTempForbiddenIP.erase(b);
				return true;
			}
		}
	}

	return true;
}

bool CForbiddenIP::AddForeverIP(const char* pIP, uint8 u1ConnectType)
{
	_ForbiddenIP ForbiddenIP;
	sprintf_safe(ForbiddenIP.m_szClientIP, MAX_IP_SIZE, "%s", pIP);
	ForbiddenIP.m_u1ConnectType = u1ConnectType;
	m_VecForeverForbiddenIP.push_back(ForbiddenIP);
	SaveConfig();

	return true;
}

bool CForbiddenIP::AddTempIP(const char* pIP, uint32 u4Second, uint8 u1ConnectType)
{
	_ForbiddenIP ForbiddenIP;
	sprintf_safe(ForbiddenIP.m_szClientIP, MAX_IP_SIZE, "%s", pIP);
	ForbiddenIP.m_u1Type        = 1;
	ForbiddenIP.m_tvBegin       = ACE_OS::gettimeofday();
	ForbiddenIP.m_u4Second      = u4Second;
	ForbiddenIP.m_u1ConnectType = u1ConnectType;
	m_VecTempForbiddenIP.push_back(ForbiddenIP);

	return true;
}

bool CForbiddenIP::DelForeverIP(const char* pIP, uint8 u1ConnectType)
{
	for(VecForbiddenIP::iterator b = m_VecForeverForbiddenIP.begin(); b != m_VecForeverForbiddenIP.end(); b++)
	{
		if(ACE_OS::strcmp(pIP, (*b).m_szClientIP) == 0 && (*b).m_u1ConnectType == u1ConnectType)
		{
			m_VecForeverForbiddenIP.erase(b);
			SaveConfig();
			return true;
		}
	}

	return true;
}

bool CForbiddenIP::DelTempIP(const char* pIP, uint8 u1ConnectType)
{
	for(VecForbiddenIP::iterator b = m_VecTempForbiddenIP.begin(); b !=  m_VecTempForbiddenIP.end(); b++)
	{
		if(ACE_OS::strcmp(pIP, (*b).m_szClientIP) == 0 && (*b).m_u1ConnectType == u1ConnectType)
		{
			m_VecTempForbiddenIP.erase(b);
			return true;
		}
	}

	return true;
}

bool CForbiddenIP::SaveConfig()
{

	//将修改的配置信息写入文件
	FILE* pFile = ACE_OS::fopen(FORBIDDENIP_FILE, "wb+");
	if(NULL == pFile)
	{
		OUR_DEBUG((LM_ERROR, "[CForbiddenIP::SaveConfig]Open file fail.\n"));
		return false;
	}

	char szTemp[MAX_BUFF_500] = {'\0'};
	sprintf_safe(szTemp, MAX_BUFF_500, "[ForbiddenIP]\r\nIPCount=%d\r\n", (int)m_VecForeverForbiddenIP.size());

	size_t stSize = ACE_OS::fwrite(szTemp, sizeof(char), ACE_OS::strlen(szTemp), pFile);
	if(stSize !=  ACE_OS::strlen(szTemp))
	{
		OUR_DEBUG((LM_ERROR, "[CForbiddenIP::SaveConfig]Write file fail.\n"));
		ACE_OS::fclose(pFile);
		return false;
	}

	for(int i = 0; i < (int)m_VecForeverForbiddenIP.size(); i++)
	{
		if(m_VecForeverForbiddenIP[i].m_u1ConnectType == CONNECT_TCP)
		{
			sprintf_safe(szTemp, MAX_BUFF_500, "IP%d=%s,TCP\r\n", i, m_VecForeverForbiddenIP[i].m_szClientIP); 
		}
		else
		{
			sprintf_safe(szTemp, MAX_BUFF_500, "IP%d=%s,UDP\r\n", i, m_VecForeverForbiddenIP[i].m_szClientIP); 
		}
		
		size_t stSize = ACE_OS::fwrite(szTemp, sizeof(char), ACE_OS::strlen(szTemp), pFile);
		if(stSize !=  ACE_OS::strlen(szTemp))
		{
			OUR_DEBUG((LM_ERROR, "[CForbiddenIP::SaveConfig]Write file fail.\n"));
			ACE_OS::fclose(pFile);
			return false;
		}
	}

	ACE_OS::fflush(pFile);
	ACE_OS::fclose(pFile);
	return true;
}

bool CForbiddenIP::ParseTXT(const char* pText, char* pIP, char* pConnectType)
{
	int i           = 0;
	pIP[0]          = '\0';
	pConnectType[0] = '\0';
	int nLen        = (int)ACE_OS::strlen(pText);

	bool bState = false;
	for(i = 0; i < nLen; i++)
	{
		if(pText[i] == ',')
		{
			bState = true;
			break;
		}
	}

	if(i >= 20 || nLen >= 40)
	{
		bState = false;
	}

	if(bState == true)
	{
		ACE_OS::memcpy(pIP, pText, i);
		pIP[i] = '\0';
		ACE_OS::memcpy(pConnectType, &pText[i + 1], nLen - i - 1);
		pConnectType[nLen - i - 1] = '\0';
	}

	return bState;
}

VecForbiddenIP* CForbiddenIP::ShowForeverIP() const
{
	return (VecForbiddenIP*)&m_VecForeverForbiddenIP;
}

VecForbiddenIP* CForbiddenIP::ShowTemoIP() const
{
	return (VecForbiddenIP*)&m_VecTempForbiddenIP;
}
