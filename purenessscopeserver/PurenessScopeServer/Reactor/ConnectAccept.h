#ifndef _CONNECTACCEPT_H
#define _CONNECTACCEPT_H

#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/INET_Addr.h"
#include "ConnectHandler.h"
#include "ConsoleHandler.h"

#include <vector>

using namespace std;

//���ں�̨����ӿ�
class ConnectConsoleAcceptor : public ACE_Acceptor<CConsoleHandler, ACE_SOCK_ACCEPTOR>
{
protected:
	virtual int make_svc_handler(CConsoleHandler*& sh);
};

//������ͨ�ͻ���
class ConnectAcceptor : public ACE_Acceptor<CConnectHandler, ACE_SOCK_ACCEPTOR>
{
protected:
	virtual int make_svc_handler(CConnectHandler*& sh);

public:
	//��Ϊ������open����������������������������һ��open�ӿ�
	//�ṩ����backlog�ķ���
	int open2(ACE_INET_Addr& local_addr,
		ACE_Reactor* reactor,
		int flags,
		int backlog);

};

class CConnectAcceptorManager
{
public:
	CConnectAcceptorManager(void);
	~CConnectAcceptorManager(void);

	bool InitConnectAcceptor(int nCount);
	void Close();
	int GetCount();
	ConnectAcceptor* GetConnectAcceptor(int nIndex);
	const char* GetError();

private:
	typedef vector<ConnectAcceptor*> vecConnectAcceptor;
	vecConnectAcceptor m_vecConnectAcceptor;
	int                m_nAcceptorCount;
	char               m_szError[MAX_BUFF_500];
};



#endif
