// TcpPacketCheck.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTcpPacketCheckApp:
// �йش����ʵ�֣������ TcpPacketCheck.cpp
//

class CTcpPacketCheckApp : public CWinApp
{
public:
	CTcpPacketCheckApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTcpPacketCheckApp theApp;