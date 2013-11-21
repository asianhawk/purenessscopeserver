// PlugClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#define MAX_BUFF_50      50
#define MAX_BUFF_200     200
#define MAX_BUFF_500     500

#define COMMAND_PLUGA    0x2201

struct _ServerInfo
{
	char m_szServerIP[MAX_BUFF_50];
	int  m_nServerPort;

	_ServerInfo()
	{
		m_szServerIP[0] = '\0';
		m_nServerPort   = 0;
	}
};

struct _PassTest
{
	int m_nSendCount;   //ѹ�ⷢ�����ݰ���
	int m_nRecvCount;   //ѹ��������ݰ���

	_PassTest()
	{
		Init();
	}

	void Init()
	{
		m_nSendCount = 0;
		m_nRecvCount = 0;
	}
};

// CPlugClientDlg �Ի���
class CPlugClientDlg : public CDialog
{
// ����
public:
	CPlugClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PLUGCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
	void Init();
	bool Connect();
	void Close();
	bool Send_Plug(const char* pText, bool blIsDisPlay = true);
	void Show_Send_List();

public:
	bool Send_Multiple_Plug();

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CEdit m_txtServerIP;
	CEdit m_txtServerPort;
	CEdit m_txtText;
	CEdit m_txtReturnText;
	CEdit m_txtSendCount;
	CEdit m_txtRecvCount;

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnClose();

private:
	_ServerInfo  m_objServerInfo;
	_PassTest    m_objPassTest;
	SOCKET       m_sckClient;
	bool         m_blIsRun;         //�Ƿ��ڽ���ѹ������
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
