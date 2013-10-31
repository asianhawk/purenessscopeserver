#ifndef _USERVALIDMANAGER_H
#define _USERVALIDMANAGER_H

#include "SMOption.h"
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

enum EM_CheckState
{
	CHECKS_HIT = 0,      //����
	CHECKS_UNHIT,        //������
};

struct _UserValid
{
	char   m_szUserName[MAX_BUFF_50];   //�û���
	char   m_szUserPass[MAX_BUFF_50];   //���� 
	uint32 m_u4LoginCount;              //��½����
	uint8  m_u1State;                   //��ǰά��״̬����ά��״̬����
	bool   m_blOnline;                  //���߱�ǣ�trueΪ���ߣ�falseΪ���� 
	bool   m_blDelete;                  //ɾ�����Ϊ��Watch����ά�������ǣ�falseΪ����ʹ�ã�trueΪ�������Ѿ�ɾ�� 

	_UserValid()
	{
		m_szUserName[0] = '\0';
		m_szUserPass[0] = '\0';
		m_u1State       = CHECKS_HIT;
		m_blDelete      = false;
		m_blOnline      = false;
		m_u4LoginCount  = 0;
	}
};

typedef vector<_UserValid*> vecValid;              //����δʹ�õ�vecValid��
typedef map<string, _UserValid*> mapUserValid;     //��ʹ�õ�vecValid

class CUserValidManager
{
public:
	CUserValidManager(uint16 u2Count = MAX_LOGIN_VALID_COUNT);
	~CUserValidManager();

	bool Init();
	void Close();

	_UserValid* GetUserValid(const char* pUserName);      //�������Ƶõ��û�״̬
	void Check_File2Memory();                             //ͬ���ļ��͹����ڴ�
	void GetFreeValid();                                  //�ӿ������ݿ���Ѱ���Ѿ��¼��ص�����

	bool Load_File(const char* pUserName);                //���ļ�����Ѱ��ָ�����û���  

private:
	bool ReadFile();                //���ļ����ع����ڴ棬�������б�
	bool ReadMemory();              //�ӹ����ڴ�����б�
	void BeginCheck();              //��ʼ���
	void EndCheck();                //������� 
	bool GetFileInfo(const char* pLine, char* pUserName, char* pUserPass);
	   
private:
	CSMOption    m_SMOption;
	uint16       m_u2Count;
	mapUserValid m_mapUserValid;
	vecValid     m_vecFreeValid;
};

#endif
