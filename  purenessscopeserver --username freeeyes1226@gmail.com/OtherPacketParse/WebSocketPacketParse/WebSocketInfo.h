#include "define.h"

#include <map>

using namespace std;

//������Ǽ�¼���е�websocket����״̬��
//add by freeeyes

//��¼����websocket������״̬�����ֲ�ͬ״̬�����ӣ��������ֻ��Ǵ������ݰ�
enum WEBSOCKET_CONNECT_STATE
{
	WEBSOCKET_STATE_HANDIN = 0,      //��Ҫ���ְ�״̬
	WEBSOCKET_STATE_DATAIN,          //��Ҫ���ݰ�״̬
};

//��¼websokcet������״̬������ǳ������ӣ�������ΪWEBSOCKET_STATE_HANDIN
struct _WebSocketInfo
{
	uint32                  m_u4ConnectID;
	WEBSOCKET_CONNECT_STATE m_emState;

	_WebSocketInfo()
	{
		m_u4ConnectID = 0;
		m_emState     = WEBSOCKET_STATE_HANDIN;
	}
};

//(��ʱ����) ��Ҫ��ʱ�������ʱ���ϣ����new��delete
//����һ���ڴ�����,�������е�_WebSocketInfoָ��
class CWebSocketInfoPool
{
public:
	CWebSocketInfoPool(uint32 u4Size = 10000)
	{
		for(uint32 i = 0; i < u4Size; i++)
		{
			_WebSocketInfo* pWebSocketInfo = new _WebSocketInfo();
			if(NULL != pWebSocketInfo)
			{
				//��ӵ�Free map����
				mapPacket::iterator f = m_mapPacketFree.find(pWebSocketInfo);
				if(f == m_mapPacketFree.end())
				{
					m_mapPacketFree.insert(mapPacket::value_type(pWebSocketInfo, pWebSocketInfo));
				}
			}
		}
	};

	~CWebSocketInfoPool()
	{
		Close();
	};

	_WebSocketInfo* Create()
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

		//���free�����Ѿ�û���ˣ�����ӵ�free���С�
		if(m_mapPacketFree.size() <= 0)
		{
			_WebSocketInfo* pWebSocketInfo = new _WebSocketInfo();

			if(pWebSocketInfo != NULL)
			{
				//��ӵ�Free map����
				mapPacket::iterator f = m_mapPacketFree.find(pWebSocketInfo);
				if(f == m_mapPacketFree.end())
				{
					m_mapPacketFree.insert(mapPacket::value_type(pWebSocketInfo, pWebSocketInfo));
				}
			}
			else
			{
				return NULL;
			}
		}

		//��free�����ó�һ��,���뵽used����
		mapPacket::iterator itorFreeB = m_mapPacketFree.begin();
		_WebSocketInfo* pWebSocketInfo = (_WebSocketInfo* )itorFreeB->second;
		m_mapPacketFree.erase(itorFreeB);
		//��ӵ�used map����
		mapPacket::iterator f = m_mapPacketUsed.find(pWebSocketInfo);
		if(f == m_mapPacketUsed.end())
		{
			m_mapPacketUsed.insert(mapPacket::value_type(pWebSocketInfo, pWebSocketInfo));
		}

		return (_WebSocketInfo* )pWebSocketInfo;
	};

	bool Delete(_WebSocketInfo* pWebSocketInfo)
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

		_WebSocketInfo* pBuff = (_WebSocketInfo* )pWebSocketInfo;
		if(NULL == pBuff)
		{
			return false;
		}

		mapPacket::iterator f = m_mapPacketUsed.find(pBuff);
		if(f != m_mapPacketUsed.end())
		{
			m_mapPacketUsed.erase(f);

			//��ӵ�Free map����
			mapPacket::iterator f = m_mapPacketFree.find(pBuff);
			if(f == m_mapPacketFree.end())
			{
				m_mapPacketFree.insert(mapPacket::value_type(pBuff, pBuff));
			}
		}

		return true;
	};

	void Close()
	{
		//���������Ѵ��ڵ�ָ��
		for(mapPacket::iterator itorFreeA = m_mapPacketFree.begin(); itorFreeA != m_mapPacketFree.end(); itorFreeA++)
		{
			_WebSocketInfo* pWebSocketInfo = (_WebSocketInfo* )itorFreeA->second;
			SAFE_DELETE(pWebSocketInfo);
		}

		for(mapPacket::iterator itorUsedB = m_mapPacketUsed.begin(); itorUsedB != m_mapPacketUsed.end(); itorUsedB++)
		{
			_WebSocketInfo* pWebSocketInfo = (_WebSocketInfo* )itorUsedB->second;
			SAFE_DELETE(pWebSocketInfo);
		}

		m_mapPacketFree.clear();
		m_mapPacketUsed.clear();
	};

private:
	typedef map<_WebSocketInfo*, _WebSocketInfo*> mapPacket;
	mapPacket                  m_mapPacketUsed;                       //��ʹ�õ�
	mapPacket                  m_mapPacketFree;                       //û��ʹ�õ�
	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;
};

//�������е�������
class CWebSocketInfoManager
{
public:
	CWebSocketInfoManager() {};
	~CWebSocketInfoManager() { Close(); };

	void Close()
	{
		for(mapWebSocketInfo::iterator b = m_mapWebSocketInfo.begin(); b != m_mapWebSocketInfo.end(); b++)
		{
			_WebSocketInfo* pWebSocketInfo = (_WebSocketInfo* )b->second;
			SAFE_DELETE(pWebSocketInfo);
		}

		m_mapWebSocketInfo.clear();
	}

	//����һ���µ���������״̬
	bool Insert(uint32 u4ConnectID)
	{
		mapWebSocketInfo::iterator f = m_mapWebSocketInfo.find(u4ConnectID);
		if(f != m_mapWebSocketInfo.end())
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::Connect]ConnectID=%d is exist.\n"));
			return false;
		}

		_WebSocketInfo* pWebSocketInfo = new _WebSocketInfo();
		pWebSocketInfo->m_u4ConnectID = u4ConnectID;
		m_mapWebSocketInfo.insert(mapWebSocketInfo::value_type(u4ConnectID, pWebSocketInfo));

		return true;
	}

	//ɾ��һ���µ����ݿ�����״̬
	void Delete(uint32 u4ConnectID)
	{
		mapWebSocketInfo::iterator f = m_mapWebSocketInfo.find(u4ConnectID);
		if(f != m_mapWebSocketInfo.end())
		{
			_WebSocketInfo* pWebSocketInfo = (_WebSocketInfo* )f->second;
			SAFE_DELETE(pWebSocketInfo);
			m_mapWebSocketInfo.erase(f);
		}
	}

	//����ָ��������״̬
	_WebSocketInfo* GetWebSocketInfo(uint32 u4ConnectID)
	{
		mapWebSocketInfo::iterator f = m_mapWebSocketInfo.find(u4ConnectID);
		if(f == m_mapWebSocketInfo.end())
		{
			//û���ҵ�
			return NULL;
		}
		else
		{
			//�ҵ���
			return (_WebSocketInfo* )f->second;
		}
	}

private:
	typedef map<uint32, _WebSocketInfo*> mapWebSocketInfo;

	//���ǵ����������µ�������֤������map��������vector
	mapWebSocketInfo m_mapWebSocketInfo;
};

typedef ACE_Singleton<CWebSocketInfoManager, ACE_Null_Mutex> App_WebSocketInfoManager;
