#ifndef _CACHEMANAGER_H
#define _CACHEMANAGER_H

//����Cache��Ĺ���
//���󹫹���
//add by freeeyes

#include "SMOption.h"

class CCacheManager
{
public:
	CCacheManager() {};

	~CCacheManager() {}; 

	//***********************************************
	//���̵�һ�δ򿪻���
	//u4CachedCount : �����ĸ���
	//objMemorykey  �������ڴ�key
	//u4CheckSize   �����黺���С
	//***********************************************
	bool Init(uint32 u4CachedCount, key_t objMemorykey, uint32 u4CheckSize)
	{
		bool blIsCreate = true;
		bool blIsOpen   = true;

		Set_Cache_Count(u4CachedCount);

		blIsOpen = m_SMOption.Init(objMemorykey, u4CheckSize, (uint16)Get_Cache_Count(), blIsCreate);
		if(false == blIsOpen)
		{
			return false;
		}

		if(blIsCreate == true)
		{
			//�����ڴ��һ�δ�������Ҫ���ļ������ؽ������ڴ�
			Read_All_Init_DataResoure();
		}
		else
		{
			//�����ڴ��Ѿ����ڣ���������б�
			Read_All_From_CacheMemory();
		}

		return true;
	};


	//***********************************************
	//�رջ������������Ҫ�̳�����ȥʵ��֮
	//***********************************************	
	virtual void Close() {};

	//***********************************************
	//��ʱͬ������Դ�ͻ����е����ݣ���Ҫ�̳�����ȥʵ��֮(����Դ -> �ڴ�)
	//***********************************************	
	virtual void Sync_DataReaource_To_Memory() {};

	//***********************************************
	//��ʱͬ������Դ�ͻ����е����ݣ���Ҫ�̳�����ȥʵ��֮(�ڴ� -> ����Դ)
	//***********************************************	
	virtual void Sync_Memory_To_DataReaource() {};

protected:
	//***********************************************
	//�����岻���ڣ���һ�γ�ʼ�����壬��Ҫ�̳�����ȥʵ��֮
	//***********************************************	
	virtual bool Read_All_Init_DataResoure() { return true; }; 

	//***********************************************
	//��������ڣ��ӻ����л�ԭ��Ӧ��ϵ����Ҫ�̳�����ȥʵ��֮
	//***********************************************	
	virtual bool Read_All_From_CacheMemory() { return true; };

	//***********************************************
	//��ʼͬ����һЩ��ʼ������
	//***********************************************
	virtual void Begin_Sync_DataReaource_To_Memory() {};

	//***********************************************
	//ͬ����ɺ��һЩ����
	//***********************************************
	virtual void End_Sync_DataReaource_To_Memory() {};

	//***********************************************
	//���û������
	//***********************************************
	void Set_Cache_Count(uint32 u4CacheCount)
	{
		m_u4Count = u4CacheCount;
	};

	//***********************************************
	//�õ���ǰ�������
	//***********************************************
	uint32 Get_Cache_Count()
	{
		return m_u4Count;
	};

	//***********************************************
	//�õ�ָ��λ�õĻ�������
	//***********************************************
	_CacheBlock* Get_CacheBlock_By_Index(uint32 u4Index) 
	{
		return (_CacheBlock* )m_SMOption.GetBuffer(u4Index);
	};

	//***********************************************
	//���û�����ȫ�����سɹ����λ
	//***********************************************
	void Set_Memory_Init_Success()                       
	{
		m_SMOption.SetMemoryState(READERINITSTATED);
	};

private:
	CSMOption    m_SMOption;
	uint32       m_u4Count;

};
#endif

