#include "ConsoleAccept.h"

int ConnectConsoleAcceptor::make_svc_handler(CConsoleHandler*& sh)
{
	//������ڴ�������ӳ����˷������趨����ֵ�����������Ӽ������ӷ�����
	CConsoleHandler* pConsoleHandler = new CConsoleHandler();

	if (NULL != pConsoleHandler)
	{
		pConsoleHandler->reactor(this->reactor());
		sh = pConsoleHandler;
		return 0;
	}
	else
	{
		return -1;
	}
}

