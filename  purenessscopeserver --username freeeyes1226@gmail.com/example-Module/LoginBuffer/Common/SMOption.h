#ifndef _SMOPTION
#define _SMOPTION

#include "ace/OS_main.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_stdlib.h"
//
#ifdef WIN32
#include "ace/Shared_Memory_MM.h"      //windwos��ʹ��
#else
#include "ace/Shared_Memory_SV.h"    //linux��ʹ��
#endif
#include "ace/Log_Msg.h"
#include "ace/OS_NS_unistd.h"

#include "SMDefine.h"
#include <map>
#include <vector>

using namespace std;

typedef map<ID_t, _SMHeader*>         mapSMHeader;

class CSMOption
{
public:
	CSMOption(void);
	~CSMOption(void);

	bool Init(key_t key, uint32 u4Size, uint32 u4ObjectCount, bool& blIsCreate);
	void Close();
	uint16 GetCount();

	_SMHeader* GetHeader(ID_t id);
	void* GetBuffer(ID_t nIndex);

	void   SetMemoryState(uint8 u1State);        //���õ�ǰ�����ڴ�״̬
	uint8  GetMemoryState();                     //�õ���ǰ�����ڴ�״̬
	void   SetMemoryVersion(uint32 u4Version);   //���õ�ǰ�����ڴ�汾
	uint32 GetMemoryVersion();                   //�õ���ǰ�����ڴ�汾

private:
	bool GetInitState();
	bool Init_Memory();

private:
	uint32                m_u4ObjectCount;
	uint32                m_u4BufferSize;
	char*                 m_pData;
#ifdef WIN32
	ACE_Shared_Memory_MM* m_pShareMemory;
#else
	ACE_Shared_Memory_SV* m_pShareMemory;
#endif
	mapSMHeader           m_mapSMHeader;
};
#endif

