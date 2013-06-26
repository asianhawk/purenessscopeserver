/*
 * ���Է���������Ϣ����
 * by w1w
 */
#pragma once

#include "IBuffPacket.h"
#include "IObject.h"
#include "CommandDefine.h"

#include <string>
using namespace std;
class CPostServerCommand : public IClientMessage
{
public:
	CPostServerCommand();
	~CPostServerCommand();

	bool RecvData(IClientParse* pClientParse);
	bool ConnectError(int nError);

	void SetServerObject(CServerObject* pServerObject);
private:
	CServerObject* m_pServerObject;
};
