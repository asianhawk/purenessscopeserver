#ifndef _USERVALIDMANAGER_H
#define _USERVALIDMANAGER_H

#include "CacheManager.h"
#include <string>
#include <iostream>

//���ù����ڴ�keyֵ����key��ӦValid����
#ifdef WIN32
#define SHM_USERVALID_KEY (key_t) "67891"
#define PRINTF printf_s
#else
#define SHM_USERVALID_KEY (key_t) 67891
#define PRINTF printf
#endif

#define MAX_LOGIN_INFO_COUNT  10
#define SOURCE_FILE_PATH "./LoginBufferFile/UserValid.txt"

//������������û��幦�ܣ�����̳�_CacheBlock����
struct _UserValid : public _CacheBlock
{
	//��������ݽṹ
	char   m_szUserName[MAX_BUFF_50];   //�û���
	char   m_szUserPass[MAX_BUFF_50];   //���� 
	uint32 m_u4LoginCount;              //��½����
};

typedef vector<_UserValid*> vecValid;              //����δʹ�õ�vecValid��
typedef map<string, _UserValid*> mapUserValid;     //��ʹ�õ�vecValid

class CUserValidManager : public CCacheManager
{
public:
	CUserValidManager();
	~CUserValidManager();

	void Close();
	void Display();

	bool Init(uint32 u4CachedCount, key_t objMemorykey, uint32 u4CheckSize);

	_UserValid* GetUserValid(const char* pUserName);      //�������Ƶõ��û�״̬
	void Sync_DataReaource_To_Memory();                   //ͬ���ļ��͹����ڴ�
	void GetFreeValid();                                  //�ӿ������ݿ���Ѱ���Ѿ��¼��ص�����

	bool Load_From_DataResouce(const char* pUserName, uint32& u4CacheIndex);    //���ļ�����Ѱ��ָ�����û���

	bool Reload_Map_CacheMemory(uint32 u4CacheIndex);     //����ָ����cachedλ�õ����ݵ�mapӳ���

private:
	bool Read_All_Init_DataResoure();         //���ļ����ع����ڴ棬�������б�
	bool Read_All_From_CacheMemory();         //�ӹ����ڴ�����б�
	void Begin_Sync_DataReaource_To_Memory(); //��ʼͬ��
	void End_Sync_DataReaource_To_Memory();   //����ͬ�� 
	bool GetFileInfo(const char* pLine, char* pUserName, char* pUserPass); //���ļ�ͬ��
	   
private:
	mapUserValid            m_mapUserValid;
	vecValid                m_vecFreeValid;
	CCachedLRUList<string>  m_objLRU;
};

#endif
