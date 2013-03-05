#include "ForbiddenIP.h"
#include "strtk.hpp"

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
	if(!LoadListCommon(szConfigPath, m_VecForeverForbiddenIP))
	{
		OUR_DEBUG((LM_INFO, "[CForbiddenIP::Init]Read Filename = %s error.\n", szConfigPath));
		return false;
	}

	return true;
}

bool CForbiddenIP::CheckIP(uint32 u4ClientIp, uint8 u1ConnectType)
{
	uint32 u4TmpIpValue = 0;

	for(int i = 0; i < (int)m_VecForeverForbiddenIP.size(); i++)
	{
		if (m_VecForeverForbiddenIP[i].m_u1ConnectType != u1ConnectType)
			continue;

		u4TmpIpValue = m_VecForeverForbiddenIP[i].m_u4Pattern & u4ClientIp;

		if(u4TmpIpValue == m_VecForeverForbiddenIP[i].m_u4ClientIp)
		{
			return false;
		}
	}

	for(VecForbiddenIP::iterator b = m_VecTempForbiddenIP.begin(); b != m_VecTempForbiddenIP.end(); b++)
	{
		if ((*b).m_u1ConnectType != u1ConnectType)
			continue;
		if((*b).m_u4ClientIp == u4ClientIp)
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

	ParseIp(pIP, ForbiddenIP);
	//sprintf_safe(ForbiddenIP.m_szClientIP, MAX_IP_SIZE, "%s", pIP);
	ForbiddenIP.m_u1ConnectType = u1ConnectType;
	m_VecForeverForbiddenIP.push_back(ForbiddenIP);
	SaveConfig();

	return true;
}

bool CForbiddenIP::AddTempIP(const char* pIP, uint32 u4Second, uint8 u1ConnectType)
{
	_ForbiddenIP ForbiddenIP;
	//sprintf_safe(ForbiddenIP.m_szClientIP, MAX_IP_SIZE, "%s", pIP);

	if (!ParseIp(pIP, ForbiddenIP))
		return false;

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

	size_t stSize = 0;

	for(int i = 0; i < (int)m_VecForeverForbiddenIP.size(); i++)
	{
		if(m_VecForeverForbiddenIP[i].m_u1ConnectType == CONNECT_TCP)
		{
			sprintf_safe(szTemp, MAX_BUFF_500, "TCP,%s\r\n", i, m_VecForeverForbiddenIP[i].m_szClientIP); 
		}
		else
		{
			sprintf_safe(szTemp, MAX_BUFF_500, "UDP,%s\r\n", i, m_VecForeverForbiddenIP[i].m_szClientIP); 
		}
		
		stSize = ACE_OS::fwrite(szTemp, sizeof(char), ACE_OS::strlen(szTemp), pFile);
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

VecForbiddenIP* CForbiddenIP::ShowForeverIP() const
{
	return (VecForbiddenIP*)&m_VecForeverForbiddenIP;
}

VecForbiddenIP* CForbiddenIP::ShowTemoIP() const
{
	return (VecForbiddenIP*)&m_VecTempForbiddenIP;
}

bool CForbiddenIP::LoadListCommon( LPCTSTR pszFileName, VecForbiddenIP& iplist )
{
	iplist.clear();

	_tifstream infile;

	infile.open(pszFileName, ios::_Nocreate);

	if (!infile.is_open())
		return false;

	_ForbiddenIP ipadr;

	TCHAR szIpLine[MAX_PATH] = {0};

	while (!infile.eof())
	{
		infile.getline(szIpLine, MAX_PATH);

		ACE_OS::memset(&ipadr, 0, sizeof(ipadr));

		if (!ParseIp(szIpLine, ipadr))
			continue;

		iplist.push_back(ipadr);
	}

	return true;
}

bool CForbiddenIP::ParseIp( const char* pszIp, _ForbiddenIP& ipadr )
{
	if (0 == pszIp)
		return false;

	int breakpos = -1;
	int loc = 0;

	_tstring s;
	_tstring delimiters = TEXT(".");
	std::vector<_tstring> strvector;

	LPCSTR pszDelimiter = 0;

	pszDelimiter = ACE_OS::strstr(pszIp, TEXT(","));

	if(ACE_OS::strstr(pszIp, "udp") != 0 ||
		ACE_OS::strstr(pszIp, "UDP") != 0)
	{
		ipadr.m_u1ConnectType = CONNECT_UDP;
	}
	else
	{
		ipadr.m_u1ConnectType = CONNECT_TCP;
	}

	if (pszDelimiter == 0)
	{
		s = pszIp;
	}
	else
	{
		pszDelimiter++;
		s = pszDelimiter;
	}

	sprintf_safe(ipadr.m_szClientIP, MAX_BUFF_20, "%s", s.c_str());
	strtk::parse(s, delimiters, strvector);

	breakpos = -1;

	for (int i = 0; i < 4 && i < strvector.size(); i++)
	{
		loc = strvector[i].find(TEXT('*'));

		if (loc != _tstring::npos)
		{
			breakpos = i;
			break;
		}
		else
		{
			ipadr.m_u1ClientIpSection[i] = ACE_OS::atoi(strvector[i].c_str());
		}
	}

	if (ipadr.m_u4ClientIp == 0)
		return false;

	uint32 u4Pattern = 0xffffffff;

	if (-1 != breakpos)
	{
		uint8* pPattern = (uint8*)&u4Pattern;

		breakpos = 4 - breakpos;

		for (int i = 0; i < breakpos; i++)
		{
			pPattern[i] = 0;
		}

		if (u4Pattern == 0)
			u4Pattern = 0xFFFFFFFF;
	}

	ipadr.m_u4Pattern = u4Pattern;

	return true;
}