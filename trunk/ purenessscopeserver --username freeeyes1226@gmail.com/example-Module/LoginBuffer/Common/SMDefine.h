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
		mapKey::iterator f = m_mapKey.find(lrukey);
		if(f == m_mapKey.end())
		{
			return false;
		}

		uint32 u4Index = (uint32)f->second;
		m_mapKey.erase(f);

		mapTimeStamp::iterator ft = m_mapTimeStamp.find(u4Index);
		if(ft == m_mapTimeStamp.end())
		{
			return false;
		}

		m_mapTimeStamp.erase(ft);
		return true;
	};

	//���һ��key������Ѿ�������������������ǰ��ȥ
	EM_LRUReturn Add_Cached_Lru(K lrukey)
	{
		mapKey::iterator f = m_mapKey.find(lrukey);
		if(f == m_mapKey.end())
		{
			//�������key,�ж��Ƿ���Ҫ����LRU���
			if(m_mapKey.size() > m_u4MaxCachedCount)
			{
				return LRU_NEED_CHECK;
			}

			//����µ�key
			uint32 u4Index = m_u4CheckIndex++;
			m_mapKey.insert(mapKey::value_type(lrukey, u4Index));
			m_mapTimeStamp.insert(mapTimeStamp::value_type(u4Index, lrukey));

			return LRU_UNNEED_CHECK;
		}
		else
		{
			//���key�Ѿ����ڣ������key��ʱ�����Ҳ����u4Index;
			uint32& u4CurrIndex = (uint32&)f->second;

			//ɾ���ɵ�key������µ�key
			mapTimeStamp::iterator ft = m_mapTimeStamp.find(u4CurrIndex);
			m_mapTimeStamp.erase(ft);

			u4CurrIndex = m_u4CheckIndex++;
			m_mapTimeStamp.insert(mapTimeStamp::value_type(u4CurrIndex, lrukey));


			return LRU_UNNEED_CHECK;
		}
	}


private:
	typedef map<K, uint32> mapKey;
	typedef map<uint32, K> mapTimeStamp;

	mapKey       m_mapKey;
	mapTimeStamp m_mapTimeStamp;
	uint32       m_u4CheckIndex; 
	uint32       m_u4MaxCachedCount;
};

#endif
