#ifndef _PROXYCLIENT_H
#define _PROXYCLIENT_H

#include "define.h"
#include "IObject.h"

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"
#include "ace/Reactor.h"
#include "ace/Synch.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"
#include "ace/Reactor_Notification_Strategy.h"

#define MAX_RECV_BUFF           4096
#define COMMAND_RETURN_PROXY    0xf030

#include <map>

using namespace std;

class CProxyClient : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
public:
	CProxyClient(void);
	~CProxyClient(void);

	bool Close();

	virtual int open(void*);
	virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE h, ACE_Reactor_Mask mask);

	bool SendData(char* pData, int nLen);

	void SetServerObject(uint32 u4ConnectID, CServerObject* pServerObject);

private:
	uint32                     m_u4SendAllSize;      //总发送字节数
	uint32                     m_u4RecvAllSize;      //总接收字节数
	CServerObject*             m_pServerObject;      //记录发送指针
	uint32                     m_u4ConnectID;        //客户端与插件的连接ID

	ACE_INET_Addr              m_addrRemote;
	int                        m_nIOCount;           //当前IO操作的个数
	ACE_Recursive_Thread_Mutex m_ThreadLock;
};

typedef ACE_Connector<CProxyClient, ACE_SOCK_CONNECTOR> CProxyClientConnector;

typedef map<CProxyClient*, uint32> mapProxy2ConnectID;
typedef map<uint32, CProxyClient*> mapConnectID2Proxy;

//管理CProxyClient和ConnectID的映射关系
//add by freeeyes
class CProxyClientManager
{
public:
	CProxyClientManager() {};
	~CProxyClientManager() {};

	bool Insert(uint32 u4ConnectID, CProxyClient* pProxyClient)
	{
		mapConnectID2Proxy::iterator f = m_mapConnectID2Proxy.find(u4ConnectID);
		if(f != m_mapConnectID2Proxy.end())
		{
			return false;
		}

		mapProxy2ConnectID::iterator fe = m_mapProxy2ConnectID.find(pProxyClient);
		if(fe != m_mapProxy2ConnectID.end())
		{
			return false;
		}

		m_mapConnectID2Proxy.insert(mapConnectID2Proxy::value_type(u4ConnectID, pProxyClient));
		m_mapProxy2ConnectID.insert(mapProxy2ConnectID::value_type(pProxyClient, u4ConnectID));

		return true;
	};

	uint32 FindConnectID(CProxyClient* pProxyClient)
	{
		mapProxy2ConnectID::iterator f = m_mapProxy2ConnectID.find(pProxyClient);
		if(f == m_mapProxy2ConnectID.end())
		{
			return 0;
		}
		else
		{
			return (uint32)f->second;
		}
	}

	CProxyClient* FindProxyClient(uint32 u4ConnectID)
	{
		mapConnectID2Proxy::iterator f = m_mapConnectID2Proxy.find(u4ConnectID);
		if(f == m_mapConnectID2Proxy.end())
		{
			return NULL;
		}
		else
		{
			return (CProxyClient* )f->second;
		}
	}

	void DeleteByProxyClient(CProxyClient* pProxyClient)
	{
		mapProxy2ConnectID::iterator f = m_mapProxy2ConnectID.find(pProxyClient);
		if(f != m_mapProxy2ConnectID.end())
		{
			uint32 u4ConnectID = (uint32)f->second;

			mapConnectID2Proxy::iterator fb = m_mapConnectID2Proxy.find(u4ConnectID);
			if(fb != m_mapConnectID2Proxy.end())
			{
				m_mapConnectID2Proxy.erase(fb);
			}

			m_mapProxy2ConnectID.erase(f);
		}
	}

	void DeleteByConnectID(uint32 u4ConnectID)
	{
		mapConnectID2Proxy::iterator f = m_mapConnectID2Proxy.find(u4ConnectID);
		if(f != m_mapConnectID2Proxy.end())
		{
			CProxyClient* pProxyClient = (CProxyClient* )f->second;

			mapProxy2ConnectID::iterator fb = m_mapProxy2ConnectID.find(pProxyClient);
			if(fb != m_mapProxy2ConnectID.end())
			{
				m_mapProxy2ConnectID.erase(fb);
			}

			m_mapConnectID2Proxy.erase(f);
		}
	}



private:
	mapProxy2ConnectID m_mapProxy2ConnectID;
	mapConnectID2Proxy m_mapConnectID2Proxy;
};

typedef ACE_Singleton<CProxyClientManager, ACE_Recursive_Thread_Mutex> App_ProxyManager;

#endif
