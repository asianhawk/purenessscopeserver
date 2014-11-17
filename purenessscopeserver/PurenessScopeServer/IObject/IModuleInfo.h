#ifndef _IMODULEINFO_H
#define _IMODULEINFO_H

#include "define.h"

class IModuleInfo
{
	virtual bool GetModuleExist(const char* pModuleName)           = 0;   //ָ����ģ���Ƿ����
	virtual const char* GetModuleParam(const char* pModuleName)    = 0;   //�õ�ģ����ز���
	virtual const char* GetModuleFileName(const char* pModuleName) = 0;   //�õ�ģ���ļ���
	virtual const char* GetModuleFilePath(const char* pModuleName) = 0;   //�õ�ģ�����·��
	virtual const char* GetModuleFileDesc(const char* pModuleName) = 0;   //�õ�ģ��������Ϣ
	virtual uint16 GetModuleCount()                                = 0;   //�õ�����ģ�����
	virtual const char* GetModuleName(uint16 u2Index)              = 0;   //����Index�õ�ģ�������
};

#endif
