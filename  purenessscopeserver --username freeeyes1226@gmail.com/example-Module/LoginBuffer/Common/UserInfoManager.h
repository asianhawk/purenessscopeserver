#ifndef _USERINFOMANAGER_H
#define _USERINFOMANAGER_H

#include "CacheManager.h"
#include <string>
#include <iostream>

//�û�����
//add by freeeyes
#ifdef WIN32
#define SHM_USERINFO_KEY (key_t) "67892"
#define PRINTF printf_s
#else
#define SHM_USERINFO_KEY (key_t) 67892
#define PRINTF printf
#endif

#define MAX_LOGIN_VALID_COUNT  10
#define SOURCE_FILE_INFO_PATH "./LoginBufferFile/UserLogin.txt"

//������������û��幦�ܣ�����̳�_CacheBlock����
struct _UserInfo : public _CacheBlock
{
	//��������ݽṹ
	uint32 m_u4UserID;      //�û�ID
	uint32 m_u4Life;        //life  
	uint32 m_u4Magic;       //magic 

	_UserInfo()
	{
		m_u4UserID = 0;
		m_u4Life   = 0;
		m_u4Magic  = 0;
	}
};

typedef vector<_UserInfo*> vecUserInfo;          //����δʹ�õ�vecValid��
typedef map<uint32, _UserInfo*> mapUserInfo;     //��ʹ�õ�vecValid

class CUserInfoManager : public CCacheManager
{
public:
	CUserInfoManager();
	~CUserInfoManager();

	void Close();
	void Display();

	bool Init(uint32 u4CachedCount, key_t objMemorykey, uint32 u4CheckSize);

	_UserInfo* GetUserInfo(uint32 u4UserID);            //�������Ƶõ��û�״̬
	void Sync_Memory_To_DataReaource();                 //ͬ���ļ��͹����ڴ�
	void GetFreeUserInfo();                             //�ӿ������ݿ���Ѱ���Ѿ��¼��ص�����

	bool Load_From_DataResouce(uint32 u4SeachUserID, uint32& u4CacheIndex);    //���ļ�����Ѱ��ָ�����û���

	bool Reload_Map_CacheMemory(uint32 u4CacheIndex);     //����ָ����cachedλ�õ����ݵ�mapӳ���

private:
	bool Read_All_Init_DataResoure();         //���ļ����ع����ڴ棬�������б�
	bool Read_All_From_CacheMemory();         //�ӹ����ڴ�����б�
	void Begin_Sync_DataReaource_To_Memory(); //��ʼͬ��
	void End_Sync_DataReaource_To_Memory();   //����ͬ�� 
	bool GetFileInfo(const char* pLine, uint32& u4UserID, uint32& u4Life, uint32& u4Magic); //���ļ�ͬ��
	bool WriteFileInfo(FILE* pFile, _UserInfo* pUserInfo);

private:
	mapUserInfo             m_mapUserInfo;
	vecUserInfo             m_vecFreeUserInfo;
	CCachedLRUList<uint32>  m_objLRU;
};


#endif
