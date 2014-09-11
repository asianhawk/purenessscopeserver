#include "ProConsoleAccept.h"

CProConsoleHandle* CProConsoleConnectAcceptor::make_handler (void)
{
	validate_new_connection(true);
	CProConsoleHandle* pProConsoleHandle = new CProConsoleHandle();
	return pProConsoleHandle;
}

int CProConsoleConnectAcceptor::validate_connection (const ACE_Asynch_Accept::Result& result,
													 const ACE_INET_Addr &remote,
													 const ACE_INET_Addr& local)
{
	//������ڷ����������IP��Χ�У���������
	if(App_MainConfig::instance()->CompareConsoleClinetIP(remote.get_host_addr()) == false)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
