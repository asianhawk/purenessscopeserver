// PssFtpClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PssFtpClient.h"
#include "PssFtpClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//�߳�ִ��
DWORD WINAPI ThreadProc(LPVOID argv)
{
	CPssFtpClientDlg* pPssFtpClientDlg = (CPssFtpClientDlg *)argv;
	if(NULL != pPssFtpClientDlg)
	{
		pPssFtpClientDlg->DownLoadListFile();
	}

	return 0;
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPssFtpClientDlg �Ի���




CPssFtpClientDlg::CPssFtpClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPssFtpClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPssFtpClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtServerIP);
	DDX_Control(pDX, IDC_EDIT2, m_txtServerPort);
	DDX_Control(pDX, IDC_EDIT3, m_txtUserName);
	DDX_Control(pDX, IDC_EDIT4, m_txtUserPass);
	DDX_Control(pDX, IDC_LIST1, m_lcPath);
	DDX_Control(pDX, IDC_BUTTON1, m_btnLogin);
	DDX_Control(pDX, IDC_BUTTON2, m_btnLogout);
	DDX_Control(pDX, IDC_EDIT5, m_txtLocalPath);
	DDX_Control(pDX, IDC_EDIT6, m_txtRemotePath);
	DDX_Control(pDX, IDC_BUTTON4, m_btnDownLoadFile);
	DDX_Control(pDX, IDC_EDIT7, m_txtUpFileName);
}

BEGIN_MESSAGE_MAP(CPssFtpClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CPssFtpClientDlg::OnBnClickedButton1)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, &CPssFtpClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPssFtpClientDlg::OnBnClickedButton3)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CPssFtpClientDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON4, &CPssFtpClientDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CPssFtpClientDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CPssFtpClientDlg ��Ϣ�������

BOOL CPssFtpClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	Init();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPssFtpClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPssFtpClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPssFtpClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPssFtpClientDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strData;

	m_txtServerIP.GetWindowText(strData);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, m_ClientFTPInfo.szServerIP, MAX_BUFF_50, NULL,NULL);
	m_ClientFTPInfo.szServerIP[nDecLen] = '\0';

	m_txtServerPort.GetWindowText(strData);
	m_ClientFTPInfo.nServerPort = _ttoi((LPCTSTR)strData);

	m_txtUserName.GetWindowText(strData);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, m_ClientFTPInfo.szUserName, MAX_BUFF_50, NULL,NULL);
	m_ClientFTPInfo.szUserName[nDecLen] = '\0';

	m_txtUserPass.GetWindowText(strData);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, m_ClientFTPInfo.szUserPass, MAX_BUFF_50, NULL,NULL);
	m_ClientFTPInfo.szUserPass[nDecLen] = '\0';

	//�ж�socket�Ƿ��Ѿ����ӣ������������Ͽ�
	if(m_sckClient != INVALID_SOCKET)
	{
		Close();
	}

	if(Connect() == false)
	{
		return;
	}

	if(true == Send_Login())
	{
		MessageBox(_T("����Զ�̷������ɹ�"), _T("��ʾ��Ϣ"), MB_OK);
		m_btnLogin.EnableWindow(FALSE);
		m_btnLogout.EnableWindow(TRUE);
	}
	else
	{
		Close();
	}
}

void CPssFtpClientDlg::OnBnClickedButton2()
{
	//���õǳ���Ϊ
	Send_Logout();
	Close();
	MessageBox(_T("�ǳ��������ɹ�"), _T("��ʾ��Ϣ"), MB_OK);
	m_btnLogin.EnableWindow(TRUE);
	m_btnLogout.EnableWindow(FALSE);
}

void CPssFtpClientDlg::Init()
{
	m_txtServerIP.SetWindowText(_T("127.0.0.1"));
	m_txtServerPort.SetWindowText(_T("10002"));
	m_txtUserName.SetWindowText(_T("freeeyes"));
	m_txtUserPass.SetWindowText(_T("1234"));

	m_lcPath.InsertColumn(0, _T("�ļ�����"), LVCFMT_CENTER, 300);
	m_lcPath.InsertColumn(1, _T("����"), LVCFMT_CENTER, 50);
	m_lcPath.InsertColumn(2, _T("��С"), LVCFMT_CENTER, 150);

	DWORD dwStyle = m_lcPath.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_lcPath.SetExtendedStyle(dwStyle); //������չ���

	//��ʼ��TCP����
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("����socket�����ʧ�ܣ����鱾��socket��汾"), _T("������Ϣ"), MB_OK);
	}

	m_txtLocalPath.SetWindowText(_T("./"));
	m_txtRemotePath.SetWindowText(_T("./"));

	m_btnLogin.EnableWindow(TRUE);
	m_btnLogout.EnableWindow(FALSE);

	m_sckClient = INVALID_SOCKET;
}

void CPssFtpClientDlg::Close()
{
	closesocket(m_sckClient);
	m_sckClient = INVALID_SOCKET;
}

bool CPssFtpClientDlg::Connect()
{
	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(m_ClientFTPInfo.nServerPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(m_ClientFTPInfo.szServerIP);

	m_sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)100;
	::setsockopt(m_sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	int nErr = connect(m_sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		//����ʧ��
		DWORD dwError = GetLastError();
		MessageBox(_T("����Զ�̷�����ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}
	
	return true;
}
void CPssFtpClientDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//�ж�socket�Ƿ��Ѿ����ӣ������������Ͽ�
	if(m_sckClient != INVALID_SOCKET)
	{
		Close();
	}

	CDialog::OnClose();
}

bool CPssFtpClientDlg::Send_Login()
{
	int nCommand = COMMAND_LOGIN;
	char szSendBuff[MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	int nPos = 0;
	int nLen = 2 + 2 + (int)strlen(m_ClientFTPInfo.szUserName) + (int)strlen(m_ClientFTPInfo.szUserPass);

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
	nPos += sizeof(short);

	int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
	memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
	nPos += sizeof(char);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
	nPos += nStrLen;
	
	nStrLen = (int)strlen(m_ClientFTPInfo.szUserPass);
	memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
	nPos += sizeof(char);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserPass, nStrLen);
	nPos += nStrLen;

	int nTotalSendLen = nPos;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(m_sckClient, szSendBuff + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//�������
				blSendFlag = true;
				break;
			}
			else
			{
				nBeginSend += nCurrSendLen;
			}
		}
	}

	//�Ƚ������ֽڵ����ݰ�����
	char szRecvLength[4] = {'\0'};
	nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 4, 0);
	if(nCurrRecvLen != 4)
	{
		DWORD dwError = GetLastError();
		MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}

	int nRecvLength = 0;
	memcpy_s(&nRecvLength, sizeof(int), szRecvLength, sizeof(int));
	char* pRecvBuff = new char[nRecvLength];
	int nRecvBegin  = 0;

	while(true)
	{
		if(nRecvLength - nRecvBegin == 0)
		{
			break;
		}

		//������ͳɹ��ˣ������������
		nCurrRecvLen = recv(m_sckClient, (char* )pRecvBuff + nRecvBegin, nRecvLength - nRecvBegin, 0);
		if(nCurrRecvLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nRecvBegin += nCurrRecvLen;
		}
	}

	int nRecvCommandID = 0;
	int nRet           = 0;
	nPos               = 0;

	memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
	nPos += sizeof(short);
	memcpy_s((char*)&nRet,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);

	if(nRet == LOGIN_FAIL_NOEXIST)
	{
		MessageBox(_T("�û���������"), _T("������Ϣ"), MB_OK);
		return false;
	}
	else if(nRet == LOGIN_FAIL_ONLINE)
	{
		MessageBox(_T("���û������ߣ����ܵ�¼"), _T("������Ϣ"), MB_OK);
		return false;
	}
	else if(nRet == LOGIN_FAIL_PASSWORD)
	{
		MessageBox(_T("�û����벻��ȷ"), _T("������Ϣ"), MB_OK);
		return false;
	}

	return true;
}

bool CPssFtpClientDlg::Send_Logout()
{
	int nCommand = COMMAND_LOGOUT;
	char szSendBuff[MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	int nPos = 0;
	int nLen = 2 + 1 + (int)strlen(m_ClientFTPInfo.szUserName);

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
	nPos += sizeof(short);

	int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
	memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
	nPos += sizeof(char);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
	nPos += nStrLen;

	int nTotalSendLen = nPos;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(m_sckClient, szSendBuff + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//�������
				blSendFlag = true;
				break;
			}
			else
			{
				nBeginSend += nCurrSendLen;
			}
		}
	}

	//�Ƚ������ֽڵ����ݰ�����
	char szRecvLength[4] = {'\0'};
	nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 4, 0);
	if(nCurrRecvLen != 4)
	{
		DWORD dwError = GetLastError();
		MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}

	int nRecvLength = 0;
	memcpy_s(&nRecvLength, sizeof(int), szRecvLength, sizeof(int));
	char* pRecvBuff = new char[nRecvLength];
	int nRecvBegin  = 0;

	while(true)
	{
		if(nRecvLength - nRecvBegin == 0)
		{
			break;
		}

		//������ͳɹ��ˣ������������
		nCurrRecvLen = recv(m_sckClient, (char* )pRecvBuff + nRecvBegin, nRecvLength - nRecvBegin, 0);
		if(nCurrRecvLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nRecvBegin += nCurrRecvLen;
		}
	}

	int nRecvCommandID = 0;
	int nRet           = 0;
	nPos               = 0;

	memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
	nPos += sizeof(short);
	memcpy_s((char*)&nRet,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);

	return true;
}

bool CPssFtpClientDlg::Send_FileList(const char* pRemotePath)
{
	m_lcPath.DeleteAllItems();
	int nCommand = COMMAND_FILELIST;
	char szSendBuff[MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	int nPos = 0;
	int nLen = 2 + 3 + (int)strlen(m_ClientFTPInfo.szUserName) + (int)strlen(pRemotePath);

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
	nPos += sizeof(short);

	int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
	memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
	nPos += sizeof(char);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
	nPos += nStrLen;

	nStrLen = (int)strlen(pRemotePath);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nStrLen, sizeof(short));
	nPos += sizeof(short);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)pRemotePath, nStrLen);
	nPos += nStrLen;

	int nTotalSendLen = nPos;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(m_sckClient, szSendBuff + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//�������
				blSendFlag = true;
				break;
			}
			else
			{
				nBeginSend += nCurrSendLen;
			}
		}
	}

	//�Ƚ������ֽڵ����ݰ�����
	char szRecvLength[4] = {'\0'};
	nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 4, 0);
	if(nCurrRecvLen != 4)
	{
		DWORD dwError = GetLastError();
		MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}

	int nRecvLength = 0;
	memcpy_s(&nRecvLength, sizeof(int), szRecvLength, sizeof(int));
	char* pRecvBuff = new char[nRecvLength];
	int nRecvBegin  = 0;

	while(true)
	{
		if(nRecvLength - nRecvBegin == 0)
		{
			break;
		}

		//������ͳɹ��ˣ������������
		nCurrRecvLen = recv(m_sckClient, (char* )pRecvBuff + nRecvBegin, nRecvLength - nRecvBegin, 0);
		if(nCurrRecvLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nRecvBegin += nCurrRecvLen;
		}
	}

	int nRecvCommandID = 0;
	int nRet           = 0;
	nPos               = 0;

	memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
	nPos += sizeof(short);
	memcpy_s((char*)&nRet,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);

	if(nRet == OP_OK)
	{
		int nCount = 0;
		memcpy_s((char*)&nCount, sizeof(int), &pRecvBuff[nPos], sizeof(int));
		nPos += sizeof(int);

		for(int i = 0; i < nCount; i++)
		{
			int nFileNameLen = 0;
			char szFileName[MAX_BUFF_500] = {'\0'};
			int nFileType = IS_FILE;
			int nFileSize = 0;
			memcpy_s((char*)&nFileNameLen, sizeof(char), &pRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);
			memcpy_s((char*)&szFileName, nFileNameLen, &pRecvBuff[nPos], nFileNameLen);
			nPos += nFileNameLen;
			memcpy_s((char*)&nFileType, sizeof(int), &pRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);
			memcpy_s((char*)&nFileSize, sizeof(int), &pRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			//������ʾ
			CString strData;
			wchar_t wszFileName[MAX_BUFF_500]    = {'\0'};

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szFileName, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wszFileName, MAX_BUFF_500);

			m_lcPath.InsertItem(i, wszFileName);
			if(nFileType == IS_FILE)
			{
				m_lcPath.SetItemText(i, 1, _T("�ļ�"));
			}
			else
			{
				m_lcPath.SetItemText(i, 1, _T("�ļ���"));
			}
			strData.Format(_T("%d"), nFileSize);
			m_lcPath.SetItemText(i, 2, strData);
		}
	}

	return true;
}

bool CPssFtpClientDlg::Send_Download(const char* pLocalPath, const char* pFileName, const char* pRemotePath, int nIndex, int nSize, int& nBockCount)
{
	int nCommand = COMMAND_FILE_DOWNLOAD;
	char szSendBuff[MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	int nPos = 0;
	int nLen = 2 + 3 + (int)strlen(m_ClientFTPInfo.szUserName) + (int)strlen(pRemotePath) + 8;

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
	nPos += sizeof(short);

	int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
	memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
	nPos += sizeof(char);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
	nPos += nStrLen;

	nStrLen = (int)strlen(pRemotePath);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nStrLen, sizeof(short));
	nPos += sizeof(short);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char*)pRemotePath, nStrLen);
	nPos += nStrLen;

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nSize, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nIndex, sizeof(int));
	nPos += sizeof(int);

	int nTotalSendLen = nPos;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(m_sckClient, szSendBuff + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//�������
				blSendFlag = true;
				break;
			}
			else
			{
				nBeginSend += nCurrSendLen;
			}
		}
	}

	//�Ƚ������ֽڵ����ݰ�����
	char szRecvLength[4] = {'\0'};
	nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 4, 0);
	if(nCurrRecvLen != 4)
	{
		DWORD dwError = GetLastError();
		MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}

	int nRecvLength = 0;
	memcpy_s(&nRecvLength, sizeof(int), szRecvLength, sizeof(int));
	char* pRecvBuff = new char[nRecvLength];
	int nRecvBegin  = 0;

	while(true)
	{
		if(nRecvLength - nRecvBegin == 0)
		{
			break;
		}

		//������ͳɹ��ˣ������������
		nCurrRecvLen = recv(m_sckClient, (char* )pRecvBuff + nRecvBegin, nRecvLength - nRecvBegin, 0);
		if(nCurrRecvLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return false;
		}
		else
		{
			nRecvBegin += nCurrRecvLen;
		}
	}

	int nRecvCommandID = 0;
	int nRet           = 0;
	nPos               = 0;

	memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
	nPos += sizeof(short);
	memcpy_s((char*)&nRet,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);

	if(nRet == OP_OK)
	{
		//�õ��ļ��飬��д���ļ�
		char* pBuffer = new char[nSize];
		int nFileBlockCount     = 0;
		int nFileCurrIndex      = 0;
		int nFileCurrBufferSize = 0;

		memcpy_s((char*)&nBockCount,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
		nPos += sizeof(int);
		memcpy_s((char*)&nFileCurrIndex,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
		nPos += sizeof(int);
		memcpy_s((char*)&nFileCurrBufferSize,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
		nPos += sizeof(int);
		memcpy_s((char*)pBuffer, nFileCurrBufferSize, &pRecvBuff[nPos], nFileCurrBufferSize);
		nPos += nFileCurrBufferSize;

		char szLocalFilePath[MAX_BUFF_500] = {'\0'};
		sprintf_s(szLocalFilePath, MAX_BUFF_500, "%s%s", pLocalPath, pFileName);

		//д���ļ�
		if(nFileCurrIndex == 0)
		{
			//����ǳ�ʼ������ɾ����ǰ�ļ���
			remove(szLocalFilePath);
		}

		FILE* pFile = NULL;
		fopen_s(&pFile, szLocalFilePath, "ab+");
		if(pFile == NULL)
		{
			MessageBox(_T("д�뱾���ļ�ʧ��"), _T("������Ϣ"), MB_OK);
			delete[] pBuffer;
			return false;
		}

		fwrite(pBuffer, sizeof(char), nFileCurrBufferSize, pFile);
		fclose(pFile);
		delete[] pBuffer;
	}
	else
	{
		MessageBox(_T("Զ���ļ����ʧ��"), _T("������Ϣ"), MB_OK);
		return false;
	}

	return true;
}

bool CPssFtpClientDlg::Send_Upload( const char* pLocalPath, const char* pFileName, const char* pRemotePath )
{
	int nBufferSize = MAX_BUFF_10240;
	int nCommand = COMMAND_FILE_UPLOAD;
	char szSendBuff[MAX_BUFF_10240 + MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	char szLoaclFilePath[MAX_BUFF_500] = {'\0'};
	sprintf_s(szLoaclFilePath, MAX_BUFF_500, "%s%s", pLocalPath, pFileName);

	//�õ���ǰ�ļ���С
	FILE* pFile = NULL;
	fopen_s(&pFile, szLoaclFilePath, "rb");
	if(NULL == pFile)
	{
		return false;
	}

	fseek(pFile, 0L, SEEK_END);
	int nFileLen        = (int)ftell(pFile);
	int nBlockCount     = 0;
	int nLastBufferSize = 0;

	if(nFileLen % nBufferSize != 0)
	{
		nBlockCount     = nFileLen / nBufferSize + 1;
		nLastBufferSize = nFileLen % nBufferSize;
	}
	else
	{
		nBlockCount     = nFileLen / nBufferSize;
		nLastBufferSize = nBufferSize;
	}

	fclose(pFile);
	pFile = NULL;

	for(int i = 0; i < nBlockCount; i++)
	{
		fopen_s(&pFile, szLoaclFilePath, "rb");
		if(NULL == pFile)
		{
			return false;
		}

		int nPos = 0;
		char szBuffer[MAX_BUFF_10240] = {'\0'};
		if(i != nBlockCount - 1)
		{
			//����������һ����
			fseek(pFile, (long)(i * nBufferSize), SEEK_CUR);

			int nReadSize = fread((char* )szBuffer, sizeof(char), nBufferSize, pFile);
			
			int nLen = 2 + 3 + 4 + (int)strlen(m_ClientFTPInfo.szUserName) + (int)strlen(pRemotePath) + 8 + nBufferSize;

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
			nPos += sizeof(short);

			int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
			memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
			nPos += sizeof(char);
			memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
			nPos += nStrLen;

			nStrLen = (int)strlen(pRemotePath);
			memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nStrLen, sizeof(short));
			nPos += sizeof(short);
			memcpy_s(&szSendBuff[nPos], nStrLen, (char*)pRemotePath, nStrLen);
			nPos += nStrLen;

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nBufferSize, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&i, sizeof(int));
			nPos += sizeof(int);

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nBufferSize, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], nBufferSize, (char*)szBuffer, nBufferSize);
			nPos += nBufferSize;
		}
		else
		{
			//��������һ����
			fseek(pFile, (long)(i * nBufferSize), SEEK_CUR);

			int nReadSize = fread((char* )szBuffer, sizeof(char), nLastBufferSize, pFile);

			int nLen = 2 + 3 + 4 + (int)strlen(m_ClientFTPInfo.szUserName) + (int)strlen(pRemotePath) + 8 + nLastBufferSize;

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
			nPos += sizeof(short);

			int nStrLen = (int)strlen(m_ClientFTPInfo.szUserName);
			memcpy_s(&szSendBuff[nPos], sizeof(char), (char*)&nStrLen, sizeof(char));
			nPos += sizeof(char);
			memcpy_s(&szSendBuff[nPos], nStrLen, (char*)m_ClientFTPInfo.szUserName, nStrLen);
			nPos += nStrLen;

			nStrLen = (int)strlen(pRemotePath);
			memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nStrLen, sizeof(short));
			nPos += sizeof(short);
			memcpy_s(&szSendBuff[nPos], nStrLen, (char*)pRemotePath, nStrLen);
			nPos += nStrLen;

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLastBufferSize, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&i, sizeof(int));
			nPos += sizeof(int);

			memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLastBufferSize, sizeof(int));
			nPos += sizeof(int);
			memcpy_s(&szSendBuff[nPos], nLastBufferSize, (char*)szBuffer, nLastBufferSize);
			nPos += nLastBufferSize;
		}

		int nTotalSendLen = nPos;
		int nBeginSend    = 0;
		int nCurrSendLen  = 0;
		bool blSendFlag   = false;
		int nBeginRecv    = 0;
		int nCurrRecvLen  = 0;
		bool blRecvFlag   = false;
		while(true)
		{
			nCurrSendLen = send(m_sckClient, szSendBuff + nBeginSend, nTotalSendLen, 0);
			if(nCurrSendLen <= 0)
			{
				DWORD dwError = GetLastError();
				MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
				fclose(pFile);
				return false;
			}
			else
			{
				nTotalSendLen -= nCurrSendLen;
				if(nTotalSendLen == 0)
				{
					//�������
					blSendFlag = true;
					break;
				}
				else
				{
					nBeginSend += nCurrSendLen;
				}
			}
		}

		//�Ƚ������ֽڵ����ݰ�����
		char szRecvLength[4] = {'\0'};
		nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 4, 0);
		if(nCurrRecvLen != 4)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			fclose(pFile);
			return false;
		}

		int nRecvLength = 0;
		memcpy_s(&nRecvLength, sizeof(int), szRecvLength, sizeof(int));
		char* pRecvBuff = new char[nRecvLength];
		int nRecvBegin  = 0;

		while(true)
		{
			if(nRecvLength - nRecvBegin == 0)
			{
				break;
			}

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(m_sckClient, (char* )pRecvBuff + nRecvBegin, nRecvLength - nRecvBegin, 0);
			if(nCurrRecvLen <= 0)
			{
				DWORD dwError = GetLastError();
				MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
				fclose(pFile);
				return false;
			}
			else
			{
				nRecvBegin += nCurrRecvLen;
			}
		}

		int nRecvCommandID = 0;
		int nRet           = 0;
		nPos               = 0;

		memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
		nPos += sizeof(short);
		memcpy_s((char*)&nRet,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
		nPos += sizeof(int);

		if(nRet != OP_OK)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			fclose(pFile);
			return false;
		}

		fclose(pFile);
	}

	MessageBox(_T("�ϴ��ļ��ɹ�"), _T("������Ϣ"), MB_OK);

	return true;
}

void CPssFtpClientDlg::OnBnClickedButton3()
{
	//��Զ��Ŀ¼,������б�
	CString strData;
	char szRemotePath[MAX_BUFF_500] = {'\0'};

	m_txtRemotePath.GetWindowText(strData);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, szRemotePath, MAX_BUFF_500, NULL,NULL);
	szRemotePath[nDecLen] = '\0';

	Send_FileList(szRemotePath);
}

void CPssFtpClientDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;

	//������к�
	int nRow = phdn->iItem;

	CString StrPathName = m_lcPath.GetItemText(nRow, 0);
	CString StrPathType = m_lcPath.GetItemText(nRow, 1);

	if(wcscmp((LPCTSTR)StrPathType, _T("�ļ���")) == 0)
	{
		//������ļ��У������m_txtRemotePath
		CString strData;
		m_txtRemotePath.GetWindowText(strData);
		strData += StrPathName + _T("/");
		m_txtRemotePath.SetWindowText(strData);
	}

	if(StrPathType )

	*pResult = 0;
}

void CPssFtpClientDlg::OnBnClickedButton4()
{
	//������̨�߳�����
	DWORD  ThreadID = 0;
	CreateThread(NULL, NULL, ThreadProc, (LPVOID)this, NULL, &ThreadID);
}

void CPssFtpClientDlg::DownLoadListFile()
{
	m_btnDownLoadFile.EnableWindow(FALSE);
	//�����ļ�
	for(int i = 0; i < m_lcPath.GetItemCount(); i++)
	{
		if( m_lcPath.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED || m_lcPath.GetCheck(i))
		{
			//ѡ���������ļ�
			int nBufferCount = 0;
			char szFileName[MAX_BUFF_500]   = {'\0'};
			char szRemotePath[MAX_BUFF_500] = {'\0'};
			char szLocalPath[MAX_BUFF_500]  = {'\0'};

			//�õ��ļ���
			CString strData;
			CString strRomoteFilePath;
			CString strLocalPath;
			CString StrPathName = m_lcPath.GetItemText(i, 0);
			m_txtRemotePath.GetWindowText(strData);
			strRomoteFilePath = strData + StrPathName;

			int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strRomoteFilePath, strRomoteFilePath.GetLength(), NULL, 0, NULL, NULL);
			int nDecLen = WideCharToMultiByte(CP_ACP, 0, strRomoteFilePath, nSrcLen, szRemotePath, MAX_BUFF_500, NULL, NULL);
			szRemotePath[nDecLen] = '\0';

			nSrcLen = WideCharToMultiByte(CP_ACP, 0, StrPathName, StrPathName.GetLength(), NULL, 0, NULL, NULL);
			nDecLen = WideCharToMultiByte(CP_ACP, 0, StrPathName, nSrcLen, szFileName, MAX_BUFF_500, NULL, NULL);
			szFileName[nDecLen] = '\0';

			m_txtLocalPath.GetWindowText(strLocalPath);
			nSrcLen = WideCharToMultiByte(CP_ACP, 0, strLocalPath, strLocalPath.GetLength(), NULL, 0, NULL, NULL);
			nDecLen = WideCharToMultiByte(CP_ACP, 0, strLocalPath, nSrcLen, szLocalPath, MAX_BUFF_500, NULL, NULL);
			szLocalPath[nDecLen] = '\0';

			_DownloadFileInfo objDownloadFileInfo;

			//����Ҫע�⣬���Ҫ���ø�������ݿ飬��Ҫ�޸�PSS�����ݰ�����С�������ļ�
			//Ĭ��PSS֧��������ݰ���20K�������Ҫ���Ըĵĸ���
			int nSize  = MAX_BUFF_10240; 

			sprintf_s(objDownloadFileInfo.szLocalPath, MAX_BUFF_500, "%s", szLocalPath);
			sprintf_s(objDownloadFileInfo.szFileName, MAX_BUFF_500, "%s", szFileName);
			sprintf_s(objDownloadFileInfo.szRemotePath, MAX_BUFF_500, "%s", szRemotePath);
			objDownloadFileInfo.nSize = nSize;

			int nIndex = 0;

			bool blState = Send_Download(szLocalPath, szFileName, szRemotePath, nIndex, nSize, nBufferCount);
			if(blState == false)
			{
				MessageBox(_T("Զ���ļ�����ʧ��"), _T("������Ϣ"), MB_OK);
				continue;
			}

			for(int i = 1; i < nBufferCount; i++)
			{
				blState = Send_Download(szLocalPath, szFileName, szRemotePath, i, nSize, nBufferCount);
				if(blState == false)
				{
					MessageBox(_T("Զ���ļ�����ʧ��"), _T("������Ϣ"), MB_OK);
					break;
				}
			}
		}
	}

	MessageBox(_T("�����ļ����"), _T("��ʾ��Ϣ"), MB_OK);
	m_btnDownLoadFile.EnableWindow(TRUE);
}
void CPssFtpClientDlg::OnBnClickedButton5()
{
	//�ϴ��ļ�
	char szFileName[MAX_BUFF_500]   = {'\0'};
	char szRemotePath[MAX_BUFF_500] = {'\0'};
	char szLocalPath[MAX_BUFF_500]  = {'\0'};
	char szLocalFile[MAX_BUFF_500]  = {'\0'};
	char szRemoteFile[MAX_BUFF_500] = {'\0'};

	//�õ��ļ���
	CString strData;
	CString strRomoteFilePath;
	CString strLocalPath;
	CString strLocalFile;

	m_txtRemotePath.GetWindowText(strRomoteFilePath);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strRomoteFilePath, strRomoteFilePath.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strRomoteFilePath, nSrcLen, szRemotePath, MAX_BUFF_500, NULL, NULL);
	szRemotePath[nDecLen] = '\0';

	m_txtLocalPath.GetWindowText(strLocalPath);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strLocalPath, strLocalPath.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strLocalPath, nSrcLen, szLocalPath, MAX_BUFF_500, NULL, NULL);
	szLocalPath[nDecLen] = '\0';

	m_txtUpFileName.GetWindowText(strLocalFile);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strLocalFile, strLocalFile.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strLocalFile, nSrcLen, szLocalFile, MAX_BUFF_500, NULL, NULL);
	szLocalFile[nDecLen] = '\0';

	sprintf_s(szRemoteFile, MAX_BUFF_500, "%s/%s", strRomoteFilePath, szLocalFile);

	Send_Upload(szLocalPath, szLocalFile, szRemoteFile);

}
