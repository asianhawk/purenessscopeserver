#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "ClientDefine.h"
#include "TcpClientConnect.h"

// CDlgTrackIP �Ի���

class CDlgTrackIP : public CDialog
{
	DECLARE_DYNAMIC(CDlgTrackIP)

public:
	CDlgTrackIP(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgTrackIP();

// �Ի�������
	enum { IDD = IDD_DIALOG_TRACKIP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_txtTrackIP;
	CEdit m_txtConnectID;
	CEdit m_txtIPInfo;
	CListCtrl m_lcTrackIPHistory;

public:
	CString GetPageTitle();
	void SetTcpClientConnect(CTcpClientConnect* pTcpClientConnect);

private:
	CTcpClientConnect* m_pTcpClientConnect;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton6();
};
