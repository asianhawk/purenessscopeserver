#include "WorkThreadAI.h"

CWorkThreadAI::CWorkThreadAI()
{
	m_u1WTAI                  = (uint8)0;   //AIĬ��Ϊ�ر�
	m_u4WTCheckTime           = 0;
	m_u4WTTimeoutCount        = 0;
	m_u4WTStopTime            = 0;
	m_u1WTReturnDataType      = 0;
	m_u4DisposeTime           = 0;
	m_u2ReturnDataLen         = 0;
	m_szWTReturnData[0]       = '\0';
}

CWorkThreadAI::~CWorkThreadAI()
{
	Close();
}

void CWorkThreadAI::Init(uint8 u1AI, uint32 u4DisposeTime, uint32 u4WTCheckTime, uint32 u4WTTimeoutCount, uint32 u4WTStopTime, uint8 u1WTReturnDataType, const char* pReturnData)
{
	m_u1WTAI             = u1AI;
	m_u4WTCheckTime      = u4WTCheckTime;
	m_u4WTTimeoutCount   = u4WTTimeoutCount;
	m_u4WTStopTime       = u4WTStopTime;
	m_u1WTReturnDataType = u1WTReturnDataType;
	m_u4DisposeTime      = u4DisposeTime;

	if(m_u1WTReturnDataType == 1)
	{
		//����Ƕ����ƣ��任Ϊ�����ƴ洢
		CConvertBuffer objConvertBuffer;

		int nSize = MAX_BUFF_1024;
		m_u2ReturnDataLen = objConvertBuffer.GetBufferSize(pReturnData, (int)strlen(pReturnData));
		objConvertBuffer.Convertstr2charArray(pReturnData, (int)strlen(pReturnData), (unsigned char*)m_szWTReturnData, nSize);
	}
	else
	{
		//������ı�ֱ��copy
		sprintf_safe(m_szWTReturnData, MAX_BUFF_1024, "%s", pReturnData);
		m_u2ReturnDataLen = (uint16)ACE_OS::strlen(m_szWTReturnData);
	}
}

bool CWorkThreadAI::SaveTimeout(uint16 u2CommandID, uint32 u4TimeCost)
{
	if(m_u1WTAI == 1)
	{
		//����ڴ���ʱ���ڣ�����涼������
		if(m_u4DisposeTime > u4TimeCost)
		{
			return false;
		}

		//UR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]CommandiD=%d,cost=%d.\n", u2CommandID, u4TimeCost));

		uint32 u4Now = (uint32)ACE_OS::gettimeofday().sec();

		//����Ѿ��ڽ�ֹ�б�������
		//if(true == CheckCurrTimeout(u2CommandID, u4Now))
		//{
		//	return false;
		//}

		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]m_vecCommandTime.size=%d.\n", (int)m_vecCommandTime.size()));

		//�������AI����
		for(uint16 i = 0; i < (uint16)m_vecCommandTime.size(); i++)
		{
			if(u2CommandID == m_vecCommandTime[i]->m_u2CommandID)
			{
				//�������һ����������
				_CommandTimeout* pCommandTimeout = m_vecCommandTime[i]->m_objTime.GetFreeData();
				//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]pCommandTimeout=0x%08x, CurrIndex=%d.\n", (int)&pCommandTimeout,  m_vecCommandTime[i]->m_objTime.GetCurrIndex()));
				if(NULL != pCommandTimeout)
				{
					pCommandTimeout->m_u2CommandID = u2CommandID;
					pCommandTimeout->m_u4Second    = u4Now;
				}
				else
				{
					//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]pCommandTimeout is NULL.\n"));
				}

				m_vecCommandTime[i]->m_objTime.Add();

				//���жϵ�ǰ���������һ��ʱ���Ƿ��ڼ��ʱ����
				_CommandTimeout* pCommandLastTimeout = m_vecCommandTime[i]->m_objTime.GetLinkData(m_u4WTTimeoutCount - 1);
				if(NULL != pCommandLastTimeout)
				{
					//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]TimeCost(%d) --> (%d).\n", (int)(u4Now - pCommandLastTimeout->m_u4Second), (int)m_u4WTCheckTime));
					if(u4Now - pCommandLastTimeout->m_u4Second <= m_u4WTCheckTime)
					{
						//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]1 m_vecCommandTime=0x%08x, m_vecCommandTimeout=0x%08x.\n", (int)&m_vecCommandTime, (int)&m_vecCommandTimeout));
						//��Ҫ�ر���
						_CommandTimeout objCommandTimeout;
						objCommandTimeout.m_u2CommandID = u2CommandID;
						objCommandTimeout.m_u4Second    = u4Now + m_u4WTStopTime;
						m_vecCommandTimeout.push_back(objCommandTimeout);

						//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]size=%d.\n", (int)m_vecCommandTimeout.size()));
						return true;
					}
					else
					{
						//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]1 pCommandTimeout is NULL.\n"));
						return false;
					}
				}
				else
				{
					return false;
				}
			}
		}

		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]Bgein m_vecCommandTime=0x%08x, m_vecCommandTimeout=0x%08x.\n", (int)&m_vecCommandTime, (int)&m_vecCommandTimeout));
		//������ڼ���б��У����һ������б�
		_CommandTime* pCheckCommandTime = new _CommandTime();
		pCheckCommandTime->m_u2CommandID = u2CommandID;
		pCheckCommandTime->m_objTime.Init(m_u4WTTimeoutCount);
		_CommandTimeout* pCommandTimeout = pCheckCommandTime->m_objTime.GetFreeData();
		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]pCommandTimeout=0x%08x, CurrIndex=%d.\n", (int)&pCommandTimeout,  pCheckCommandTime->m_objTime.GetCurrIndex()));
		if(NULL != pCommandTimeout)
		{
			pCommandTimeout->m_u2CommandID = u2CommandID;
			pCommandTimeout->m_u4Second    = u4Now;
		}
		pCheckCommandTime->m_objTime.Add();
		m_vecCommandTime.push_back(pCheckCommandTime);

		//�����ֻ��Ҫ1�����͹رգ���ô�������˵������б���
		if(m_u4WTTimeoutCount <= 1)
		{
			//��Ҫ�ر���
			_CommandTimeout objCommandTimeout;
			objCommandTimeout.m_u2CommandID = u2CommandID;
			objCommandTimeout.m_u4Second    = u4Now + m_u4WTStopTime;
			m_vecCommandTimeout.push_back(objCommandTimeout);
		}

		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::SaveTimeout]End m_vecCommandTime=0x%08x, m_vecCommandTimeout=0x%08x.\n", (int)&m_vecCommandTime, (int)&m_vecCommandTimeout));

		return false;
	}

	return false;
}

bool CWorkThreadAI::CheckCurrTimeout(uint16 u2CommandID, uint32 u4Now)
{
	if(m_u1WTAI == 1)
	{
		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::CheckCurrTimeout]1 size=%d.\n", (int)m_vecCommandTimeout.size()));

		if(m_vecCommandTimeout.size() == 0)
		{
			return false;
		}

		//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::CheckCurrTimeout]size=%d.\n", (int)m_vecCommandTimeout.size()));

		//�����Ҫ������ѭ��
		for(vector<_CommandTimeout>::iterator b = m_vecCommandTimeout.begin(); b != m_vecCommandTimeout.end(); b++)
		{
			_CommandTimeout objCommandTimeout = (_CommandTimeout)(*b);

			//OUR_DEBUG((LM_INFO, "[CWorkThreadAI::CheckCurrTimeout]objCommandTimeout.m_u2CommandID=%d,u2CommandID=%d.\n", (int)objCommandTimeout.m_u2CommandID, u2CommandID));
			if(objCommandTimeout.m_u2CommandID == u2CommandID)
			{
				if(objCommandTimeout.m_u4Second >= u4Now)
				{
					//�ڽ�ֹʱ���ڣ����ص�ǰ���Ӧ����������
					return true;
				}
				else
				{
					//�Ѿ������ֹʱ�䣬�ӽ�ֹ�б���ɾ��
					m_vecCommandTimeout.erase(b);
					return false;
				}
			}
		}

		return false;
	}

	return false;
}

char* CWorkThreadAI::GetReturnData()
{
	return (char* )m_szWTReturnData;
}

uint16 CWorkThreadAI::GetReturnDataLength()
{
	return m_u2ReturnDataLen;
}

void CWorkThreadAI::Close()
{
	for(uint16 i = 0; i < (uint16)m_vecCommandTime.size(); i++)
	{
		_CommandTime* pCommandTime = (_CommandTime* )m_vecCommandTime[i];
		SAFE_DELETE(pCommandTime);
	}

	m_vecCommandTime.clear();
	m_vecCommandTimeout.clear();
}
