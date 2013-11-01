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

#define MAX_LOGIN_VALID_COUNT  200
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

	_UserValid* GetUserValid(const char* pUserName);      //�������Ƶõ��û�״̬
	void Sync_DataReaource_To_Memory();                   //ͬ���ļ��͹����ڴ�
	void GetFreeValid();                                  //�ӿ������ݿ���Ѱ���Ѿ��¼��ص�����

	bool Load_From_DataResouce(const char* pUserName);    //���ļ�����Ѱ��ָ�����û���  

private:
	bool Read_All_Init_DataResoure();         //���ļ����ع����ڴ棬�������б�
	bool Read_All_From_CacheMemory();         //�ӹ����ڴ�����б�
	void Begin_Sync_DataReaource_To_Memory(); //��ʼͬ��
	void End_Sync_DataReaource_To_Memory();   //����ͬ�� 
	bool GetFileInfo(const char* pLine, char* pUserName, char* pUserPass); //���ļ�ͬ��
	   
private:
	mapUserValid m_mapUserValid;
	vecValid     m_vecFreeValid;
};

#endif
