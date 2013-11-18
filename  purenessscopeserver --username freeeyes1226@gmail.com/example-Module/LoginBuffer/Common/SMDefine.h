#ifndef DEFINE_H
#define DEFINE_H

#include "define.h"
#include <ace/Basic_Types.h>
#include <ace/Time_Value.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/OS_NS_string.h>

#include <map>

using namespace std;

typedef ACE_UINT32 ID_t;

#define INVALID_ID     0
#define INVALID_OFFSET 0

enum EM_ReaderInitState
{
	READERINITSTATE = 0,   //�����ڴ�δ��ʼ������
	READERINITSTATED,      //�����ڴ��Ѿ���ʼ������
};

enum EM_CACHED_USE_STATE
{
	CACHEDUSED = 0,         //��cached���ڱ���Ľ���ʹ��
	CACHEDUNUSED,           //��cachedû�б���Ľ���ʹ��
};

enum EM_CheckState
{
	CHECKS_HIT = 0,      //���У������Լ�����Ѿ��ѵ���ǰ�����ڴ����ݣ�
	CHECKS_UNHIT,        //�����У������Լ����û���ѵ���ǰ�����ڴ����ݣ�
};

enum EM_LRUReturn
{
	LRU_NEED_CHECK = 0,   //��Ҫ����LRU��̭
	LRU_UNNEED_CHECK,     //����Ҫ����LRU��̭
};

//�����ڴ�ͷ
struct _SMHeader
{
	ID_t           m_HeaderID;          //ͷID
	uint32         m_u4DataOffset;      //�׵�ַƫ�ƣ�Ҳ����������T*�ĵ�ַ
	ACE_Time_Value m_tvUpdate;          //���ݱ���ȡ����ʱ�� 

	_SMHeader()
	{
		m_HeaderID     = INVALID_ID;
		m_u4DataOffset = INVALID_OFFSET;
	}
};

//���ڻ��嵱ǰ״̬�����ݽṹ
struct _CacheBlock
{
private:
	EM_CheckState       m_emState;           //��ǰά��״̬����ӦEM_CheckState
	EM_CACHED_USE_STATE m_emIsUsed;          //���߱�ǣ���ӦEM_CACHED_USE_STATE״̬
	bool                m_blDelete;          //ɾ�����Ϊ��Watch����ά�������ǣ�falseΪ����ʹ�ã�trueΪ�������Ѿ�ɾ��
	uint32              m_u4CacheIndex;      //��ǰ���ݿ�Ŀ�ID

public:
	_CacheBlock()
	{
		m_emState       = CHECKS_HIT;
		m_blDelete      = false;
		m_emIsUsed      = CACHEDUNUSED;
		m_u4CacheIndex  = 0;
	}

	//���ÿ�ID
	void SetCacheIndex(uint32 u4CacheIndex)
	{
		m_u4CacheIndex = u4CacheIndex;
	}

	//��ÿ�ID
	uint32 GetCacheIndex()
	{
		return m_u4CacheIndex;
	}
	
	//������Դ����
	void SetHit()
	{
		m_blDelete      = false;
		m_emState       = CHECKS_HIT;
	}

	//û�б�����Դ����
	void SetUnHit()
	{
		m_blDelete     = true;
		m_emState      = CHECKS_UNHIT;
	}

	//����ɾ��״̬
	void SetDelete(bool blDelete)
	{
		m_blDelete = blDelete;
	}

	//���ù����ڴ�û�б��߼�����ʹ��
	void SetUsed()
	{
		m_emIsUsed = CACHEDUSED;
	}

	//���ù����ڴ�û�б��߼�����ʹ��
	void SetUnUsed()
	{
		m_emIsUsed = CACHEDUNUSED;
	}

	//�õ�ɾ��״̬
	bool GetDelete()
	{
		return m_blDelete;
	}

	//�õ��Ƿ�����״̬
	EM_CheckState GetCheckState()
	{
		return m_emState;
	}

	//��������״̬
	void SetCheckState(EM_CheckState objCheckState)
	{
		m_emState = objCheckState;
	}
};

//����LRU���㷨��
//�����ڴ�������̭����
template<class K>
class CCachedLRUList
{
public:
	CCachedLRUList()
	{
		m_u4CheckIndex     = 0;
		m_u4MaxCachedCount = 0;
	};

	~CCachedLRUList()
	{
	};

	void Set_Lru_Max_Count(uint32 u4MaxCachedCount)
	{
		m_u4MaxCachedCount = u4MaxCachedCount;
	}

	//����Ƿ���Ҫ��ʼLRU�������Ҫ�򷵻����Ҫɾ����
	bool Check_Cached_Lru(K& lrukey)
	{
		if(m_mapKey.size() < m_u4MaxCachedCount)
		{
			return false;
		}
		else
		{
			//�ҵ�LruӦ��������key
			lrukey = (K&)m_mapTimeStamp.begin()->second;
			return true;
		}
	};

	//�Ӷ�����ɾ��ָ����key
	bool Delete_Cached_Lru(K lrukey)
	{
		typename mapKey::iterator f = m_mapKey.find(lrukey);
		if(f == m_mapKey.end())
		{
			return false;
		}

		uint32 u4Index = (uint32)f->second;
		m_mapKey.erase(f);

		typename mapTimeStamp::iterator ft = m_mapTimeStamp.find(u4Index);
		if(ft == m_mapTimeStamp.end())
		{
			return false;
		}

		m_mapTimeStamp.erase(ft);

		//ɾ��Index��key֮��Ķ�Ӧ��ϵ
		typename mapKey2Index::iterator fi = m_mapKey2Index.find(lrukey);
		if(fi != m_mapKey2Index.end())
		{
			uint32 u4CachedIndex = (uint32)fi->second;

			//ɾ����Ӧ��ϵ
			m_mapKey2Index.erase(fi);

			//ɾ����һ��map
			typename mapIndex2Key::iterator fii = m_mapIndex2Key.find(u4CachedIndex);
			if(fii != m_mapIndex2Key.end())
			{
				m_mapIndex2Key.erase(fii);
			}
		}

		return true;
	};

	//���һ��key������Ѿ�������������������ǰ��ȥ
	EM_LRUReturn Add_Cached_Lru(K lrukey, uint32 u4CachedIndex)
	{
		typename mapKey::iterator f = m_mapKey.find(lrukey);
		if(f == m_mapKey.end())
		{
			//�������key,�ж��Ƿ���Ҫ����LRU���
			if(m_mapKey.size() > m_u4MaxCachedCount)
			{
				return LRU_NEED_CHECK;
			}

			//����µ�key
			uint32 u4Index = m_u4CheckIndex++;
			m_mapKey.insert(typename mapKey::value_type(lrukey, u4Index));
			m_mapTimeStamp.insert(typename mapTimeStamp::value_type(u4Index, lrukey));

			m_mapKey2Index.insert(typename mapKey2Index::value_type(lrukey, u4CachedIndex));
			m_mapIndex2Key.insert(typename mapIndex2Key::value_type(u4CachedIndex, lrukey));

			return LRU_UNNEED_CHECK;
		}
		else
		{
			//���key�Ѿ����ڣ������key��ʱ�����Ҳ����u4Index;
			uint32& u4CurrIndex = (uint32&)f->second;

			//ɾ���ɵ�key������µ�key
			typename mapTimeStamp::iterator ft = m_mapTimeStamp.find(u4CurrIndex);
			m_mapTimeStamp.erase(ft);

			u4CurrIndex = m_u4CheckIndex++;
			m_mapTimeStamp.insert(typename mapTimeStamp::value_type(u4CurrIndex, lrukey));

			return LRU_UNNEED_CHECK;
		}
	}

	//����ʵ���������Lru�Ķ�Ӧindex��key���б�
	bool Reload_Cached_IndexList(K lrukey, K& lruBeforekey, uint32 u4CachedIndex)
	{
		//Ѱ��֮ǰ��Index��Ӧ��key���޸�֮
		typename mapIndex2Key::iterator fii = m_mapIndex2Key.find(u4CachedIndex);
		if(fii == m_mapIndex2Key.end())
		{
			return false;
		}

		lruBeforekey = (K)fii->second;

		typename mapKey2Index::iterator fi = m_mapKey2Index.find(lruBeforekey);
		if(fi != m_mapKey2Index.end())
		{
			m_mapKey2Index.erase(fi);

			//����µ�key��Ӧ��ϵ
			m_mapKey2Index.insert(typename mapKey2Index::value_type(lrukey, u4CachedIndex));
		}

		m_mapIndex2Key[u4CachedIndex] = lrukey;

		//���Դ���
		//DisPlay_Index2Key();
		//DisPlay_Key2Index();

		return true;
	}

	//���ָ��λ�õ�Index��Ӧ��Ϣ
	bool Get_Cached_KeyByIndex(uint32 u4CachedIndex, K& lrukey)
	{
		typename mapIndex2Key::iterator fii = m_mapIndex2Key.find(u4CachedIndex);
		if(fii == m_mapIndex2Key.end())
		{
			return false;
		}
		else
		{
			lrukey = (K )fii->second;
			return true;
		}
	}

private:
	//���ڲ�����ʾ����ӳ������
	void DisPlay_Index2Key()
	{
		OUR_DEBUG((LM_INFO, "[DisPlay_Index2Key]*****Begin DisPlay*****\n"));
		for(typename mapIndex2Key::iterator b = m_mapIndex2Key.begin(); b != m_mapIndex2Key.end(); b++)
		{
			OUR_DEBUG((LM_INFO, "[DisPlay_Index2Key]key=%s.\n", ((string)(b->second)).c_str()));
		}
		OUR_DEBUG((LM_INFO, "[DisPlay_Index2Key]*****End DisPlay*****\n"));
	}

	//���ڲ�����ʾ����ӳ������
	void DisPlay_Key2Index()
	{
		OUR_DEBUG((LM_INFO, "[DisPlay_Key2Index]*****Begin DisPlay*****\n"));
		for(typename mapKey2Index::iterator b = m_mapKey2Index.begin(); b != m_mapKey2Index.end(); b++)
		{
			OUR_DEBUG((LM_INFO, "[DisPlay_Key2Index]key=%s.\n", ((string)(b->first)).c_str()));
		}
		OUR_DEBUG((LM_INFO, "[DisPlay_Key2Index]*****End DisPlay*****\n"));
	}

private:
	typedef map<K, uint32> mapKey;       //key��Version��Ӧ��ϵ
	typedef map<uint32, K> mapTimeStamp; //Version��key�Ķ�Ӧ��ϵ

	typedef map<K, uint32> mapKey2Index; //��¼Index��key�Ĺ�ϵ
	typedef map<uint32, K> mapIndex2Key; //��¼key��Index�Ĺ�ϵ

	mapKey       m_mapKey;
	mapTimeStamp m_mapTimeStamp;

	mapKey2Index m_mapKey2Index;
	mapIndex2Key m_mapIndex2Key;

	uint32       m_u4CheckIndex; 
	uint32       m_u4MaxCachedCount;
};

#endif
