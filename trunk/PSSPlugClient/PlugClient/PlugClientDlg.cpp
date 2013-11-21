// PlugClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PlugClient.h"
#include "PlugClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CPlugClientDlg �Ի���
//�߳�ִ��
DWORD WINAPI ThreadProc(LPVOID argv)
{
	CPlugClientDlg* pLoginClientDlg = (CPlugClientDlg *)argv;
	if(NULL != pLoginClientDlg)
	{
		pLoginClientDlg->Send_Multiple_Plug();
	}

	return 0;
}



CPlugClientDlg::CPlugClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlugClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlugClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtServerIP);
	DDX_Control(pDX, IDC_EDIT2, m_txtServerPort);
	DDX_Control(pDX, IDC_EDIT3, m_txtText);
	DDX_Control(pDX, IDC_EDIT4, m_txtReturnText);
	DDX_Control(pDX, IDC_EDIT5, m_txtSendCount);
	DDX_Control(pDX, IDC_EDIT7, m_txtRecvCount);
}

BEGIN_MESSAGE_MAP(CPlugClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CPlugClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPlugClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPlugClientDlg::OnBnClickedButton3)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPlugClientDlg ��Ϣ�������

BOOL CPlugClientDlg::OnInitDialog()
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

void CPlugClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPlugClientDlg::OnPaint()
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
HCURSOR CPlugClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlugClientDlg::Init()
{
	m_txtServerIP.SetWindowText(_T("127.0.0.1"));
	m_txtServerPort.SetWindowText(_T("10002"));
	m_txtText.SetWindowText(_T("freeeyes������ʣ�������һ˿�Ʋʡ�"));

	//��ʼ��TCP����
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("����socket�����ʧ�ܣ����鱾��socket��汾"), _T("������Ϣ"), MB_OK);
	}

	m_blIsRun = false;
}


void CPlugClientDlg::OnBnClickedButton1()
{
	CString strData;

	//�����ط�����������Ϣ
	m_txtServerIP.GetWindowText(strData);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, m_objServerInfo.m_szServerIP, MAX_BUFF_50, NULL,NULL);
	m_objServerInfo.m_szServerIP[nDecLen] = '\0';

	m_txtServerPort.GetWindowText(strData);
	m_objServerInfo.m_nServerPort = _ttoi((LPCTSTR)strData);

	//�ж�socket�Ƿ��Ѿ����ӣ������������Ͽ�
	if(m_sckClient != INVALID_SOCKET)
	{
		Close();
	}

	if(Connect() == false)
	{
		return;
	}

	char szText[MAX_BUFF_200] = {'\0'};

	m_txtText.GetWindowText(strData);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, szText, MAX_BUFF_50, NULL,NULL);
	szText[nDecLen] = '\0';

	Send_Plug(szText);


	Close();
}

bool CPlugClientDlg::Connect()
{
	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(m_objServerInfo.m_nServerPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(m_objServerInfo.m_szServerIP);

	m_sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)1000;
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

bool CPlugClientDlg::Send_Plug(const char* pText, bool blIsDisPlay)
{
	//��������
	int nCommand = COMMAND_PLUGA;
	char szSendBuff[MAX_BUFF_500] = {'\0'};
	if(m_sckClient == INVALID_SOCKET)
	{
		return false;
	}

	int nPos = 0;
	int nLen = 2 + 2 + (int)strlen(pText);

	memcpy_s(&szSendBuff[nPos], sizeof(int), (char*)&nLen, sizeof(int));
	nPos += sizeof(int);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nCommand, sizeof(short));
	nPos += sizeof(short);
	int nStrLen = (int)strlen(pText);
	memcpy_s(&szSendBuff[nPos], sizeof(short), (char*)&nStrLen, sizeof(short));
	nPos += sizeof(short);
	memcpy_s(&szSendBuff[nPos], nStrLen, (char* )pText, nStrLen);
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

	//���շ�������
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
			delete[] pRecvBuff;
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
	int nRetA          = 0;
	int nRetB          = 0;
	nPos               = 0;


	memcpy_s((char*)&nRecvCommandID,  sizeof(short), &pRecvBuff[nPos], sizeof(short));
	nPos += sizeof(short);
	memcpy_s((char*)&nRetB,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);
	memcpy_s((char*)&nRetA,  sizeof(int), &pRecvBuff[nPos], sizeof(int));
	nPos += sizeof(int);

	if(blIsDisPlay == true)
	{
		char    szReturnText[MAX_BUFF_50]  = {'\0'};
		wchar_t wszReturnText[MAX_BUFF_50] = {'\0'};
		sprintf_s(szReturnText, MAX_BUFF_50, "PlugA:%d,PlugB:%d.", nRetA, nRetB);

		CString steData;
		steData.Format(_T("���ջ�Ӧ���ֽ���:%d,PlugAӦ��:%d,PlugBӦ��:%d."), nRecvLength + sizeof(int), nRetA, nRetB);
		m_txtReturnText.SetWindowText(steData);
	}

	return true;
}

void CPlugClientDlg::Close()
{
	closesocket(m_sckClient);
	m_sckClient = INVALID_SOCKET;
}
void CPlugClientDlg::OnBnClickedButton2()
{
	//ѹ��
	SetTimer(1, 1000, NULL);

	DWORD  ThreadID = 0;
	CreateThread(NULL, NULL, ThreadProc, (LPVOID)this, NULL, &ThreadID);
}

void CPlugClientDlg::OnBnClickedButton3()
{
	m_blIsRun = false;
}

bool CPlugClientDlg::Send_Multiple_Plug()
{
	//��ʼѹ��
	CString strData;

	//�����ط�����������Ϣ
	m_txtServerIP.GetWindowText(strData);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, m_objServerInfo.m_szServerIP, MAX_BUFF_50, NULL,NULL);
	m_objServerInfo.m_szServerIP[nDecLen] = '\0';

	m_txtServerPort.GetWindowText(strData);
	m_objServerInfo.m_nServerPort = _ttoi((LPCTSTR)strData);

	//�ж�socket�Ƿ��Ѿ����ӣ������������Ͽ�
	if(m_sckClient != INVALID_SOCKET)
	{
		Close();
	}

	if(Connect() == false)
	{
		return false;
	}

	char szText[MAX_BUFF_200] = {'\0'};

	m_txtText.GetWindowText(strData);
	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, szText, MAX_BUFF_50, NULL,NULL);
	szText[nDecLen] = '\0';

	m_blIsRun = true;
	m_objPassTest.Init();

	while(m_blIsRun)
	{
		bool blState = Send_Plug(szText, false);
		if(true == blState)
		{
			m_objPassTest.m_nSendCount++;
			m_objPassTest.m_nRecvCount++;
		}
	}

	Close();
	KillTimer(1);

	Show_Send_List();

	return true;
}

void CPlugClientDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(m_blIsRun == true)
	{
		m_blIsRun = false;
		Sleep(1);
	}

	CDialog::OnClose();
}

void CPlugClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch(nIDEvent)
	{
	case 1:
		Show_Send_List();
	}


	CDialog::OnTimer(nIDEvent);
}

void CPlugClientDlg::Show_Send_List()
{
	CString strData;

	strData.Format(_T("%d"), m_objPassTest.m_nSendCount);
	m_txtSendCount.SetWindowText(strData);
	strData.Format(_T("%d"), m_objPassTest.m_nRecvCount);
	m_txtRecvCount.SetWindowText(strData);
}