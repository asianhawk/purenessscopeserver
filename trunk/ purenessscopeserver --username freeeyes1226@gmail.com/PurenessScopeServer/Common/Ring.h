#ifndef _RING_H
#define _RING_H

//ʵ�ֻ�״���ݵ�������д
//add by freeeyes
#include <stdio.h>

template <class T>
class CRingLink
{
public:
	CRingLink()
	{
		m_pRingLink = NULL;
	};

	CRingLink(int nMaxCount)
	{
		m_pRingLink = NULL;
		Init(nMaxCount);
	};

	void Init(int nMaxCount)
	{
		Close();

		m_nMaxCount = nMaxCount;
		m_nIndex    = 0;
		m_pRingLink = new T[nMaxCount];
	}

	~CRingLink()
	{
		Close();
	};

	void Close()
	{
		if(NULL != m_pRingLink)
		{
			delete[] m_pRingLink;
			m_pRingLink = NULL;
		}
	}

	T* GetFreeData()
	{
		if(NULL != m_pRingLink)
		{
			return (T* )(m_pRingLink + m_nIndex);
		}
		else
		{
			return NULL;
		}
	}

	void Clear()
	{
		memset(m_pRingLink, 0, sizeof(T)*m_nMaxCount);
	}

	void Add()
	{
		if(m_nIndex == m_nMaxCount - 1)
		{
			m_nIndex = 0; 
		}
		else
		{
			m_nIndex++;
		}
	}

	T* GetLinkData(int nIndex)
	{
		if(nIndex >= m_nMaxCount)
		{
			return NULL;
		}
		else
		{
			int nCurrIndex = (m_nIndex - 1 - nIndex) % m_nMaxCount;

			if(nCurrIndex < 0)
			{
				nCurrIndex = m_nMaxCount + nCurrIndex;
			}

			return (T* )(m_pRingLink + nCurrIndex);
		}
	}

	T* GetBase()
	{
		return m_pRingLink;
	}

	int GetCount()
	{
		return m_nMaxCount;
	}

	int GetCurrIndex()
	{
		return m_nIndex;
	}

private:
	int m_nMaxCount;
	T*  m_pRingLink;
	int m_nIndex;
};

#endif
