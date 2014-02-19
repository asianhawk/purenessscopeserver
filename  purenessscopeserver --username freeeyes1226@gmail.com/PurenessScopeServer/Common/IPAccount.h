#ifndef _IPACCOUNT_H
#define _IPACCOUNT_H

//��ӶԿͻ������ӵ�ͳ����Ϣ
//add by freeeyes
//2011-10-08
//������IP�Ķ�̬׷�٣�������������׷��


#include <string>
#include "Ring.h"
#include "MapTemplate.h"
#include "ace/Date_Time.h"

//IP����ͳ��ģ��
struct _IPAccount
{
	string         m_strIP;              //��ǰ���ӵ�ַ
	int            m_nCount;             //��ǰ���Ӵ���
	int            m_nAllCount;          //ָ��IP���Ӵ����ܺ� 
	int            m_nMinute;            //��ǰ������
	ACE_Date_Time  m_dtLastTime;         //�������ʱ��

	_IPAccount()
	{
		m_strIP      = "";
		m_nCount     = 0;
		m_nAllCount  = 0;
		m_dtLastTime.update();
		m_nMinute    = (int)m_dtLastTime.minute();
	}

	void Add()
	{
		m_dtLastTime.update();
		if(m_dtLastTime.minute() != m_nMinute)
		{
			m_nMinute  = (int)m_dtLastTime.minute();
			m_nCount   = 1;
			m_nAllCount++;
		}
		else
		{
			m_nCount++;
			m_nAllCount++;
		}
	}
};

//IP׷�ټ�¼������Ϣģ��
struct _IPTrackInfo
{
	char           m_szClientIP[MAX_BUFF_20]; //����IP
	int            m_nPort;                   //���Ӷ˿�
	uint32         m_u4RecvByteSize;          //�����ֽ���
	uint32         m_u4SendByteSize;          //�����ֽ���  
	ACE_Date_Time  m_dtConnectStart;          //���ӽ���ʱ��
	ACE_Date_Time  m_dtConnectEnd;            //���ӶϿ�ʱ��
	uint8          m_u1State;                 //��ǰ����״̬��0Ϊ�������ӣ�1Ϊ�ѹر� 

	_IPTrackInfo()
	{
		m_szClientIP[0]  = '\0';
		m_nPort          = 0;
		m_u4RecvByteSize = 0;
		m_u4SendByteSize = 0;
		m_u1State        = 0;
	}
};

typedef vector<_IPAccount> vecIPAccount;

class CIPAccount
{
public:
	CIPAccount() 
	{ 
		m_nNeedCheck       = 0;
		m_nMaxConnectCount = 100;  //Ĭ��ÿ�����100�� 
		m_szTrackIP[0]     = '\0';
	};

	~CIPAccount() 
	{
		OUR_DEBUG((LM_INFO, "[CIPAccount::~CIPAccount].\n"));
		Close(); 
		OUR_DEBUG((LM_INFO, "[CIPAccount::~CIPAccount]End.\n"));
	};

	void Close()
	{
		m_mapIPAccount.Clear();
	}

	void Init(int nNeedCheck, int nMaxConnectCount, uint32 u4TrackLogCount)
	{
		m_nNeedCheck       = nNeedCheck;
		m_nMaxConnectCount = nMaxConnectCount;

		m_objRing.Init(u4TrackLogCount);
	};

	bool AddIP(string strIP, int nPort)
	{
		bool blRet = false;
		//������Ҫ����Ҫ�ж��������Ҫ�������IPͳ��
		if(m_nNeedCheck == 0)
		{
			_IPAccount* pIPAccount = m_mapIPAccount.SearchMapData(strIP);
			if(NULL == pIPAccount)
			{
				//û���ҵ������
				pIPAccount = new _IPAccount();
				if(NULL == pIPAccount)
				{
					blRet = true;
				}
				else
				{
					pIPAccount->m_strIP     = strIP;
					pIPAccount->Add();
					m_mapIPAccount.AddMapData(strIP, pIPAccount);
				}
			}
			else
			{
				pIPAccount->Add();
				if(pIPAccount->m_nCount >= m_nMaxConnectCount)
				{
					blRet = false;
				}
			}

			blRet = true;
		}
		else
		{
			blRet = true;
		}

		//�鿴���IP�Ƿ�����Ҫ׷�ٵ�������
		if(ACE_OS::strlen(m_szTrackIP) > 0)
		{
			if(ACE_OS::strcmp(m_szTrackIP, strIP.c_str()) == 0)
			{
				//����µ�����
				_IPTrackInfo* pIPTrackInfo = m_objRing.GetFreeData();
				if(NULL != pIPTrackInfo)
				{
					sprintf_safe(pIPTrackInfo->m_szClientIP, MAX_BUFF_20, "%s", strIP.c_str());
					pIPTrackInfo->m_nPort = nPort;
					pIPTrackInfo->m_dtConnectStart.update();
					m_objRing.Add();
				}
			}
		}

		return blRet;
	};

	bool CloseIP(string strIP, int nPort, uint32 u4RecvSize, uint32 u4SendSize)
	{
		if(ACE_OS::strlen(m_szTrackIP) > 0)
		{
			for(int i = 0; i < m_objRing.GetCount(); i++)
			{
				_IPTrackInfo* pIPTrackInfo = m_objRing.GetLinkData(i);
				if(ACE_OS::strcmp(pIPTrackInfo->m_szClientIP, strIP.c_str()) == 0 && pIPTrackInfo->m_nPort == nPort)
				{
					//�ҵ���ƥ���IP����¼��ֹʱ��
					pIPTrackInfo->m_dtConnectEnd.update();
					pIPTrackInfo->m_u4RecvByteSize = u4RecvSize;
					pIPTrackInfo->m_u4SendByteSize = u4SendSize;
					pIPTrackInfo->m_u1State        = 1;
					return true;
				}
			}

			return false;
		}

		return true;
	}

	bool UpdateIP(string strIP, int nPort, uint32 u4RecvSize, uint32 u4SendSize)
	{
		if(ACE_OS::strlen(m_szTrackIP) > 0)
		{
			for(int i = 0; i < m_objRing.GetCount(); i++)
			{
				_IPTrackInfo* pIPTrackInfo = m_objRing.GetLinkData(i);
				if(ACE_OS::strcmp(pIPTrackInfo->m_szClientIP, strIP.c_str()) == 0 && pIPTrackInfo->m_nPort == nPort)
				{
					//�ҵ���ƥ���IP����¼��ֹʱ��
					pIPTrackInfo->m_u4RecvByteSize = u4RecvSize;
					pIPTrackInfo->m_u4SendByteSize = u4SendSize;
					return true;
				}
			}

			return false;
		}

		return true;
	}

	int GetCount()
	{
		return m_mapIPAccount.GetSize();
	}

	void GetInfo(vecIPAccount& VecIPAccount)
	{
		for(int i = 0; i < m_mapIPAccount.GetSize(); i++)
		{
			_IPAccount* pIPAccount = m_mapIPAccount.GetMapData(i);
			if(NULL != pIPAccount)
			{
				VecIPAccount.push_back(*pIPAccount);
			}
		}
	}

	void SetTrackIP(const char* pIP)
	{
		//������ǰ����ʷ��¼
		m_objRing.Clear();

		sprintf_safe(m_szTrackIP, MAX_BUFF_20, "%s", pIP);
	}

	void ClearTrackIP()
	{
		//������ǰ����ʷ��¼
		m_objRing.Clear();

		m_szTrackIP[0] = '\0';
	}

	int GetTrackIPInfoCount()
	{
		//�õ�TrackIP��ʷ�ĸ���
		return m_objRing.GetCount();
	}

	_IPTrackInfo* GetBase()
	{
		return m_objRing.GetBase();
	}

private:
	CMapTemplate<string, _IPAccount> m_mapIPAccount;                       //IPͳ����Ϣ
	vector<_IPTrackInfo>             m_vecIPTrack;                         //Ҫ׷�ٵ�IP������Ϣ
	char                             m_szTrackIP[MAX_BUFF_20];             //Ҫ׷�ٵ�������IP��Ŀǰֻ���Ƕ�̬׷һ�������������ĺ������ڴ棬Ҳ�ޱ�Ҫ��
	int                              m_nMaxConnectCount;                   //ÿ������������������ǰ����m_nNeedCheck = 0;�Ż���Ч
	int                              m_nNeedCheck;                         //�Ƿ���Ҫ��֤��0Ϊ��Ҫ��1Ϊ����Ҫ
	CRingLink<_IPTrackInfo>          m_objRing;                            //����������־����¼���IP�Ļ 
};

typedef ACE_Singleton<CIPAccount, ACE_Recursive_Thread_Mutex> App_IPAccount;

#endif
