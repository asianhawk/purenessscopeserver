#include "ProConnectAccept.h"

CProConnectHandle* ProConnectAcceptor::make_handler (void)
{
	CProConnectHandle* pProConnectHandle = App_ProConnectHandlerPool::instance()->Create();
	if(NULL != pProConnectHandle)
	{
		pProConnectHandle->SetLocalIPInfo(m_szListenIP, m_u4Port);
	}

	return pProConnectHandle;
}

int ProConnectAcceptor::validate_connection (const ACE_Asynch_Accept::Result& result,
						   		             const ACE_INET_Addr &remote,
								             const ACE_INET_Addr& local)
{
	if(App_ForbiddenIP::instance()->CheckIP(remote.get_host_addr()) == false)
	{
		//�ڽ�ֹ�б��У����������
		return -1;
	}

	//������ڴ�������ӳ����˷������趨����ֵ�����������Ӽ������ӷ�����
	if(App_ProConnectHandlerPool::instance()->GetUsedCount() > App_MainConfig::instance()->GetMaxHandlerCount())
	{
		OUR_DEBUG((LM_ERROR, "[ProConnectAcceptor::validate_connection]Connect is more MaxHandlerCount(%d > %d).\n", App_ProConnectHandlerPool::instance()->GetUsedCount(), App_MainConfig::instance()->GetMaxHandlerCount()));
		//����������
		return -1;
	}
	else
	{
		//��������
		return 0;
	}
}

char* ProConnectAcceptor::GetListenIP()
{
	return m_szListenIP;
}

uint32 ProConnectAcceptor::GetListenPort()
{
	return m_u4Port;
}

void ProConnectAcceptor::SetListenInfo(const char* pIP, uint32 u4Port)
{
	sprintf_safe(m_szListenIP, MAX_BUFF_20, "%s", pIP);
	m_u4Port = u4Port;
}

CProConnectAcceptManager::CProConnectAcceptManager(void)
{
	m_nAcceptorCount = 0;
	m_szError[0]     = '\0';
}

CProConnectAcceptManager::~CProConnectAcceptManager(void)
{
	Close();
}

bool CProConnectAcceptManager::InitConnectAcceptor(int nCount)
{
	try
	{
		Close();

		for(int i = 0; i < nCount; i++)
		{
			ProConnectAcceptor* pConnectAcceptor = new ProConnectAcceptor();
			if(NULL == pConnectAcceptor)
			{
				throw "[CProConnectAcceptManager::InitConnectAcceptor]pConnectAcceptor new is fail.";
			}

			m_vecConnectAcceptor.push_back(pConnectAcceptor);
		}

		return true;
	}
	catch (const char* szError)
	{
		sprintf_safe(m_szError, MAX_BUFF_500, "%s", szError);
		return false;
	}
}

void CProConnectAcceptManager::Close()
{
	for(int i = 0; i < (int)m_vecConnectAcceptor.size(); i++)
	{
		ProConnectAcceptor* pConnectAcceptor = (ProConnectAcceptor* )m_vecConnectAcceptor[i];
		if(NULL != pConnectAcceptor)
		{
			pConnectAcceptor->cancel();
			SAFE_DELETE(pConnectAcceptor);
		}
	}

	m_vecConnectAcceptor.clear();
	m_nAcceptorCount = 0;
}

bool CProConnectAcceptManager::Close( const char* pIP, uint32 n4Port )
{
	//�ҵ���������ָ���Ķ˿�ֹͣ����
	for(vecProConnectAcceptor::iterator b = m_vecConnectAcceptor.begin(); b != m_vecConnectAcceptor.end(); b++)
	{
		ProConnectAcceptor* pConnectAcceptor = (ProConnectAcceptor*)(*b);

		if (NULL != pConnectAcceptor)
		{
			if(ACE_OS::strcmp(pConnectAcceptor->GetListenIP(), pIP) == 0 
				&& pConnectAcceptor->GetListenPort() == n4Port)
			{
				pConnectAcceptor->cancel();
				SAFE_DELETE(pConnectAcceptor);
				m_vecConnectAcceptor.erase(b);
				break;
			}
		}
	}

	return true;
}

int CProConnectAcceptManager::GetCount()
{
	return (int)m_vecConnectAcceptor.size();
}

ProConnectAcceptor* CProConnectAcceptManager::GetConnectAcceptor(int nIndex)
{
	if(nIndex < 0 || nIndex >= (int)m_vecConnectAcceptor.size())
	{
		return NULL;
	}

	return (ProConnectAcceptor* )m_vecConnectAcceptor[nIndex];
}

const char* CProConnectAcceptManager::GetError()
{
	return m_szError;
}

bool CProConnectAcceptManager::CheckIPInfo(const char* pIP, uint32 n4Port)
{
	//�ҵ���������ָ���Ķ˿�ֹͣ����
	for(vecProConnectAcceptor::iterator b = m_vecConnectAcceptor.begin(); b != m_vecConnectAcceptor.end(); b++)
	{
		ProConnectAcceptor* pConnectAcceptor = (ProConnectAcceptor*)(*b);

		if (NULL != pConnectAcceptor)
		{
			if(ACE_OS::strcmp(pConnectAcceptor->GetListenIP(), pIP) == 0 
				&& pConnectAcceptor->GetListenPort() == n4Port)
			{
				return true;
			}
		}
	}

	return false;
}

ProConnectAcceptor* CProConnectAcceptManager::GetNewConnectAcceptor()
{
	ProConnectAcceptor* pConnectAcceptor = new ProConnectAcceptor();
	if(NULL == pConnectAcceptor)
	{
		return NULL;
	}

	m_vecConnectAcceptor.push_back(pConnectAcceptor);
	return pConnectAcceptor;
}
