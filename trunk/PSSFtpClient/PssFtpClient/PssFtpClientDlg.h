// PssFtpClientDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <WinSock2.h>

#define MAX_BUFF_50    50
#define MAX_BUFF_500   500 
#define MAX_BUFF_10240 10240

#define COMMAND_LOGIN                  0x2000     //��½
#define COMMAND_LOGOUT                 0x2001     //�˳� 
#define COMMAND_FILELIST               0x2002     //�ļ����ļ����б�
#define COMMAND_FILE_DOWNLOAD          0x2003     //�ļ�����
#define COMMAND_FILE_UPLOAD            0x2004     //�ļ��ϴ�
#define COMMAND_RETURN_LOGIN           0xe000     //��½Ӧ�� 
#define COMMAND_RETURN_LOGOUT          0xe001     //�ǳ�Ӧ��
#define COMMAND_RETURN_FILELIST        0xe002     //�ļ����ļ����б�Ӧ��
#define COMMAND_RETURN_FILE_DOWNLOAD   0xe003     //�ļ�����
#define COMMAND_RETURN_FILE_UPLOAD     0xe004     //�ļ��ϴ�

#define LOGIN_SUCCESS            0
#define LOGIN_FAIL_NOEXIST       1
#define LOGIN_FAIL_ONLINE        2
#define LOGIN_FAIL_PASSWORD      3

#define OP_OK                    0
#define OP_FAIL                  1
 
#define IS_FILE                  0
#define IS_DIRRNT                1

struct _ClientFTPInfo
{
	char szServerIP[MAX_BUFF_50];
	int  nServerPort;
	char szUserName[MAX_BUFF_50];
	char szUserPass[MAX_BUFF_50];

	_ClientFTPInfo()
	{
		szServerIP[0] = '\0';
		nServerPort   = 0;
		szUserName[0] = '\0';
		szUserPass[0] = '\0';
	}
};

struct _DownloadFileInfo
{
	char szLocalPath[MAX_BUFF_500];
	char szFileName[MAX_BUFF_500];
	char szRemotePath[MAX_BUFF_500];
	int  nSize;
};

// CPssFtpClientDlg �Ի���
class CPssFtpClientDlg : public CDialog
{
// ����
public:
	CPssFtpClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PSSFTPCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


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
	afx_msg void OnBnClickedButton1();
	CEdit m_txtServerIP;
	CEdit m_txtServerPort;
	CEdit m_txtUserName;
	CEdit m_txtUserPass;
	CListCtrl m_lcPath;
	CButton m_btnLogin;
	CButton m_btnLogout;
	CEdit m_txtLocalPath;
	CEdit m_txtRemotePath;
	CButton m_btnDownLoadFile;

private:
	void Init();
	void Close();
	bool Connect();

	bool Send_Login();
	bool Send_Logout();
	bool Send_FileList(const char* pRemotePath);
	bool Send_Download(const char* pLocalPath, const char* pFileName, const char* pRemotePath, int nIndex, int nSize, int& nBockCount);

private:
	SOCKET         m_sckClient;
	_ClientFTPInfo m_ClientFTPInfo;

public:
	void DownLoadListFile();

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton4();
};
