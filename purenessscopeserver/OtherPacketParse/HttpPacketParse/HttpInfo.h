#include "define.h"

#include <map>

using namespace std;

//������Ǽ�¼���е�Http����״̬��
//add by freeeyes

//���δ���ܰ��ĳ���,�Լ������Ĵ�С��������ȱ���������޸�����
#define MAX_ENCRYPTLENGTH 5*MAX_BUFF_1024
//���������ݰ����ȣ����������ݰ������������չ���ֵ
#define MAX_DECRYPTLENGTH 5*MAX_BUFF_1024

//��¼websokcet������״̬������ǳ������ӣ�������ΪWEBSOCKET_STATE_HANDIN
struct _HttpInfo
{
	uint32                  m_u4ConnectID;                         //���ӵ�ID
	char                    m_szData[MAX_ENCRYPTLENGTH];           //��ǰ���������ݵĳ���
	uint32                  m_u4DataLength;                        //��ǰ������е����ݳ���

	_HttpInfo()
	{
		Init();
	}

	void Init()
	{
		m_u4ConnectID = 0;

		//ACE_OS::memset(m_szData, 0, MAX_DECRYPTLENGTH);
		m_u4DataLength     = 0;
	}
};

//����һ���ڴ�����,�������е�_WebSocketInfoָ��
class CHttpInfoPool
{
public:
	CHttpInfoPool(uint32 u4Size = 1000)
	{
		for(uint32 i = 0; i < u4Size; i++)
		{
			_HttpInfo* pHttpInfo = new _HttpInfo();
			if(NULL != pHttpInfo)
			{
				//��ӵ�Free map����
				mapPacket::iterator f = m_mapPacketFree.find(pHttpInfo);
				if(f == m_mapPacketFree.end())
				{
					m_mapPacketFree.insert(mapPacket::value_type(pHttpInfo, pHttpInfo));
				}
			}
		}
	};

	~CHttpInfoPool()
	{
		Close();
	};

	void Close()
	{
		//���������Ѵ��ڵ�ָ��
		for(mapPacket::iterator itorFreeB = m_mapPacketFree.begin(); itorFreeB != m_mapPacketFree.end(); itorFreeB++)
		{
			_HttpInfo* pHttpInfo = (_HttpInfo* )itorFreeB->second;
			SAFE_DELETE(pHttpInfo);
		}

		for(mapPacket::iterator itorUsedB = m_mapPacketUsed.begin(); itorUsedB != m_mapPacketUsed.end(); itorUsedB++)
		{
			_HttpInfo* pHttpInfo = (_HttpInfo* )itorUsedB->second;
			SAFE_DELETE(pHttpInfo);
		}

		m_mapPacketFree.clear();
		m_mapPacketUsed.clear();

	}

	_HttpInfo* Create()
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

		//���free�����Ѿ�û���ˣ�����ӵ�free���С�
		if(m_mapPacketFree.size() <= 0)
		{
			_HttpInfo* pHttpInfo = new _HttpInfo();

			if(pHttpInfo != NULL)
			{
				//��ӵ�Free map����
				mapPacket::iterator f = m_mapPacketFree.find(pHttpInfo);
				if(f == m_mapPacketFree.end())
				{
					m_mapPacketFree.insert(mapPacket::value_type(pHttpInfo, pHttpInfo));
				}
			}
			else
			{
				return NULL;
			}
		}

		//��free�����ó�һ��,���뵽used����
		mapPacket::iterator itorFreeB = m_mapPacketFree.begin();
		_HttpInfo* pHttpInfo = (_HttpInfo* )itorFreeB->second;
		m_mapPacketFree.erase(itorFreeB);
		//��ӵ�used map����
		mapPacket::iterator f = m_mapPacketUsed.find(pHttpInfo);
		if(f == m_mapPacketUsed.end())
		{
			m_mapPacketUsed.insert(mapPacket::value_type(pHttpInfo, pHttpInfo));
		}

		return (_HttpInfo* )pHttpInfo;
	};

	bool Delete(_HttpInfo* pHttpInfo)
	{
		ACE_Guard<ACE_Recursive_Thread_Mutex> WGuard(m_ThreadWriteLock);

		_HttpInfo* pBuff = (_HttpInfo* )pHttpInfo;
		if(NULL == pBuff)
		{
			return false;
		}

		pHttpInfo->Init();

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

private:
	typedef map<_HttpInfo*, _HttpInfo*> mapPacket;
	mapPacket                  m_mapPacketUsed;                       //��ʹ�õ�
	mapPacket                  m_mapPacketFree;                       //û��ʹ�õ�
	ACE_Recursive_Thread_Mutex m_ThreadWriteLock;
};

//�������е�������
class CHttpInfoManager
{
public:
	CHttpInfoManager() {};
	~CHttpInfoManager() { Close(); };

	void Close()
	{
		m_mapHttpInfo.clear();
	}

	//����һ���µ���������״̬
	bool Insert(uint32 u4ConnectID)
	{
		mapHttpInfo::iterator f = m_mapHttpInfo.find(u4ConnectID);
		if(f != m_mapHttpInfo.end())
		{
			OUR_DEBUG((LM_ERROR, "[CPacketParse::Connect]ConnectID=%d is exist.\n"));
			return false;
		}

		_HttpInfo* pHttpInfo = m_objHttpInfoPool.Create();
		pHttpInfo->m_u4ConnectID  = u4ConnectID;
		m_mapHttpInfo.insert(mapHttpInfo::value_type(u4ConnectID, pHttpInfo));

		return true;
	}

	//ɾ��һ���µ����ݿ�����״̬
	void Delete(uint32 u4ConnectID)
	{
		mapHttpInfo::iterator f = m_mapHttpInfo.find(u4ConnectID);
		if(f != m_mapHttpInfo.end())
		{
			_HttpInfo* pHttpInfo = (_HttpInfo* )f->second;
			m_objHttpInfoPool.Delete(pHttpInfo);
			m_mapHttpInfo.erase(f);
		}
	}

	//����ָ��������״̬
	_HttpInfo* GetWebSocketInfo(uint32 u4ConnectID)
	{
		mapHttpInfo::iterator f = m_mapHttpInfo.find(u4ConnectID);
		if(f == m_mapHttpInfo.end())
		{
			//û���ҵ�
			return NULL;
		}
		else
		{
			//�ҵ���
			return (_HttpInfo* )f->second;
		}
	}

private:
	typedef map<uint32, _HttpInfo*> mapHttpInfo;

	//���ǵ����������µ�������֤������map��������vector
	mapHttpInfo m_mapHttpInfo;

	//���ǵ���������Ƶ�������������������ڴ�ػ��ƹ���_WebSocketInfo*
	CHttpInfoPool m_objHttpInfoPool;
};

typedef ACE_Singleton<CHttpInfoManager, ACE_Null_Mutex> App_HttpInfoManager;
