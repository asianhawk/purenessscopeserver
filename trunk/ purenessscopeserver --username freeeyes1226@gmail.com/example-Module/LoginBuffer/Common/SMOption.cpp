#include "SMOption.h"

CSMOption::CSMOption(void)
{
	m_pShareMemory  = NULL;
	m_pData         = NULL;
	m_u2ObjectCount = 0;
	m_u4BufferSize  = 0;
}

CSMOption::~CSMOption(void)
{
	Close();
}

bool CSMOption::Init(key_t key, uint32 u4Size, uint16 u2ObjectCount, bool& blIsCreate)
{
	bool blIsLinuxFirst = false;
	//�õ������ڴ��С
	//1�ֽڵ�ǰ�����ڴ�״̬��0��δ��ʼ����1���ѳ�ʼ��
	//4�ֽڵĹ����ڴ������Ϣ����watchά������
	uint32 u4MemorySize = sizeof(uint8) + sizeof(uint32) + sizeof(_SMHeader)*u2ObjectCount + u4Size*u2ObjectCount;
#ifdef WIN32
	if(m_pShareMemory == NULL)
	{
		m_pShareMemory = new ACE_Shared_Memory_MM(key, u4MemorySize);
	}
#else
	if(m_pShareMemory == NULL)
	{
		m_pShareMemory = new ACE_Shared_Memory_SV();
		//�ȳ��Դ򿪹����ڴ�
		int nError = m_pShareMemory->open(key, u4MemorySize, ACE_Shared_Memory_SV::ACE_OPEN);
		if(nError != 0)
		{
			//���������ڴ�
			nError = m_pShareMemory->open(key, u4MemorySize, ACE_Shared_Memory_SV::ACE_CREATE);
			if(nError != 0)
			{
				PRINTF("[CSMOption::Init]create share memory fail(%d).\n", errno);
			}
			blIsLinuxFirst = true;
		}
		blIsCreate = true;
	}
#endif
	m_pData = (char *)m_pShareMemory->malloc();
	if(NULL == m_pData)
	{
		//PRINTF("[CSMOption::Init]open share memory fail.\n");
		return false;
	}

	if(true == blIsLinuxFirst)
	{
		ACE_OS::memset(m_pData, 0, u4MemorySize);
	}
	
	m_u2ObjectCount = u2ObjectCount;
	m_u4BufferSize  = u4Size;

	//�Ƿ��ʼ�������ڴ�
	bool blState = GetInitState();
	if(true == blState)
	{
		Init_Memory();
	}
	else
	{
		Init_Memory();
		blIsCreate = false;
	}

	return true;
}

void CSMOption::Close()
{
	if(NULL != m_pShareMemory)
	{
		m_pShareMemory->remove();

		delete m_pShareMemory;
		m_pShareMemory = NULL;
	}

	m_u2ObjectCount = 0;
}

bool CSMOption::GetInitState()
{
	if(m_pData == NULL)
	{
		return false;
	}
	else
	{
		uint8 u1State = 0;
		ACE_OS::memcpy(&u1State, m_pData, sizeof(uint8));
		//printf("[CSMOption::GetInitState]u1State=%d, READERINITSTATE=%d.\n", u1State, READERINITSTATE);
		if(u1State == READERINITSTATE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}	
}

bool CSMOption::Init_Memory()
{
	if(m_pData == NULL)
	{
		//PRINTF("[CSMOption::Init_Memory]m_pData is NULL.\n");
		return false;
	}

	ACE_Time_Value tvNow = ACE_OS::gettimeofday(); 
	char* pBegin = m_pData + sizeof(uint8) + sizeof(uint32);

	int nPos = 0;
	for(uint16 i = 1; i <= m_u2ObjectCount; i++)
	{
		_SMHeader* pSMHeader = (_SMHeader* )(pBegin + nPos*(sizeof(_SMHeader) + m_u4BufferSize));
		pSMHeader->m_HeaderID     = (ID_t)i;
		pSMHeader->m_DataState    = STATE_OPEN;
		pSMHeader->m_u4DataOffset = nPos*(sizeof(_SMHeader) + m_u4BufferSize) + sizeof(_SMHeader);
		pSMHeader->m_tvUpdate     = tvNow;

		m_mapSMHeader.insert(mapSMHeader::value_type(pSMHeader->m_HeaderID, pSMHeader));

		nPos++;
	}

	//��ʼ�������ڴ���ز���
	uint8 u1State = (uint8)READERINITSTATED;
	ACE_OS::memcpy(m_pData, &u1State, sizeof(uint8));
	uint32 u4Version = 0;
	ACE_OS::memcpy((char* )&m_pData[sizeof(uint8)], &u4Version, sizeof(uint32));
	return true;
}

uint16 CSMOption::GetCount()
{
	return m_u2ObjectCount;
}

_SMHeader* CSMOption::GetHeader( ID_t id )
{
	mapSMHeader::iterator f = m_mapSMHeader.find(id);
	if(f == m_mapSMHeader.end())
	{
		return NULL;
	}
	else
	{
		return (_SMHeader* )f->second;
	}
}

void* CSMOption::GetBuffer( ID_t nIndex )
{
	_SMHeader* pSMHeader = GetHeader((ID_t)nIndex);
	if(NULL == pSMHeader)
	{
		return NULL;
	}
	else
	{
		return (void* )(m_pData + pSMHeader->m_u4DataOffset);
	}

}

void CSMOption::SetMemoryState(uint8 u1State)
{
	if(NULL != m_pData)
	{
		ACE_OS::memcpy(m_pData, &u1State, sizeof(uint8));
	}
}

uint8 CSMOption::GetMemoryState()
{
	if(NULL != m_pData)
	{
		return (uint8)m_pData;
	}
	else
	{
		return (uint8)0;
	}
}

void CSMOption::SetMemoryVersion(uint32 u4Version)
{
	if(NULL != m_pData)
	{
		ACE_OS::memcpy(&m_pData[sizeof(uint8)], &u4Version, sizeof(uint32));
	}
}

uint32 CSMOption::GetMemoryVersion()
{
	if(NULL != m_pData)
	{
		return (uint32)&m_pData[sizeof(uint8)];
	}
	else
	{
		return (uint32)0;
	}
}
