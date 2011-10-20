#include "ProAsynchConnect.h"

CProAsynchConnect::CProAsynchConnect(void)
{
	m_blConnectState    = false;
	m_nServerID         = 0;
}

CProAsynchConnect::~CProAsynchConnect(void)
{
}

CProConnectClient* CProAsynchConnect::make_handler()
{
	CProConnectClient* pProConnectClient = new CProConnectClient();
	if(NULL != pProConnectClient)
	{
		pProConnectClient->SetServerID(m_nServerID);
		SetConnectState(false);
	}
	else
	{
		SetConnectState(false);
	}
	return pProConnectClient;
}

int CProAsynchConnect::validate_connection(const ACE_Asynch_Connect::Result& result, const ACE_INET_Addr &remote, const ACE_INET_Addr& local)
{
	//异步检验链接是否有效，如果有效
	int nRet     = result.success ();
	ACE_HANDLE h = result.connect_handle();
	if (!nRet || h == ACE_INVALID_HANDLE)
	{
		ACE_DEBUG((LM_ERROR, "[CProAsynchConnect::validate_connection]CAsynchConnector::validate_connection; connection fails(%d).\n", nRet));
		SetConnectState(false);
		return 1;
	}
	
	return 0;
}

void CProAsynchConnect::SetConnectState(bool blConnectState)
{
	m_blConnectState = blConnectState;
}

bool CProAsynchConnect::GetConnectState()
{
	return m_blConnectState;
}

void CProAsynchConnect::SetServerID(int nServerID)
{
	m_nServerID = nServerID;
}

