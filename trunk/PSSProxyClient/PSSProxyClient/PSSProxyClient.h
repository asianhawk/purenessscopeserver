// PSSProxyClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPSSProxyClientApp:
// �йش����ʵ�֣������ PSSProxyClient.cpp
//

class CPSSProxyClientApp : public CWinApp
{
public:
	CPSSProxyClientApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPSSProxyClientApp theApp;