// PSSProxyClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PSSProxyClient.h"
#include "PSSProxyClientDlg.h"

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


// CPSSProxyClientDlg �Ի���




CPSSProxyClientDlg::CPSSProxyClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPSSProxyClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPSSProxyClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_txtServerIP);
	DDX_Control(pDX, IDC_EDIT3, m_txtServerPort);
	DDX_Control(pDX, IDC_EDIT1, m_txtSendData);
}

BEGIN_MESSAGE_MAP(CPSSProxyClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CPSSProxyClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CPSSProxyClientDlg ��Ϣ�������

BOOL CPSSProxyClientDlg::OnInitDialog()
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
	InitView();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPSSProxyClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPSSProxyClientDlg::OnPaint()
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
HCURSOR CPSSProxyClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPSSProxyClientDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strData;
	m_txtServerIP.GetWindowText(strData);

	char szServerIP[MAX_BUFF_20] = {'\0'};
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, szServerIP, MAX_BUFF_20, NULL,NULL);
	szServerIP[nDecLen] = '\0';

	m_txtServerPort.GetWindowText(strData);
	int nServerPort = _ttoi((LPCTSTR)strData);

	m_txtSendData.GetWindowText(strData);

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nBufferSize = WideCharToMultiByte(CP_ACP, 0, strData, -1, NULL, 0, NULL, NULL);  
	char* pSendData = new char[nBufferSize];
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, pSendData, nBufferSize, NULL,NULL);
	pSendData[nDecLen] = '\0';

	CConvertBuffer objConvertBuffer;
	//���Ҫת�������ݿ��С
	int nBuffSize = objConvertBuffer.GetBufferSize(pSendData, nDecLen);
	char* pSendBuff = new char[nBuffSize];
	//�����ݴ�ת���ɶ����ƴ�
	objConvertBuffer.Convertstr2charArray(pSendData, strlen(pSendData), (unsigned char*)pSendBuff, nBuffSize);
	delete[] pSendData;

	//��ʼ��������
	if(m_sckClient != INVALID_SOCKET)
	{
		Close();
	}

	if(Connect(szServerIP, nServerPort) == false)
	{
		return;
	}

	Send_Data(pSendBuff, nBuffSize);

	Close();

	delete[] pSendBuff;
}

void CPSSProxyClientDlg::InitView()
{
	m_txtServerIP.SetWindowText(_T("127.0.0.1"));
	m_txtServerPort.SetWindowText(_T("10002"));

	m_txtSendData.SetWindowText(_T("01 00 30 10 08 00 00 00 46 45 45 45 45 59 45 53 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 be cd aa 8f 3c 01 00 00"));

	//��ʼ��TCP����
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("����socket�����ʧ�ܣ����鱾��socket��汾"), _T("������Ϣ"), MB_OK);
	}

	m_sckClient = INVALID_SOCKET;
}

bool CPSSProxyClientDlg::Connect(const char* pIP, int nPort)
{
	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(pIP);

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

void CPSSProxyClientDlg::Close()
{
	closesocket(m_sckClient);
	m_sckClient = INVALID_SOCKET;
}

void CPSSProxyClientDlg::Send_Data( const char* pSendBuff, int nLen )
{
	int nTotalSendLen = nLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(m_sckClient, pSendBuff + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			DWORD dwError = GetLastError();
			MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
			return;
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
	char szRecvLength[30] = {'\0'};
	nCurrRecvLen = recv(m_sckClient, (char* )szRecvLength, 30, 0);
	if(nCurrRecvLen != 30)
	{
		DWORD dwError = GetLastError();
		MessageBox(_T("Զ�̷�������������ʧ��"), _T("������Ϣ"), MB_OK);
		return;
	}

	int nRecvLength = 0;
	memcpy_s(&nRecvLength, sizeof(int), &szRecvLength[4], sizeof(int));
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
			return;
		}
		else
		{
			nRecvBegin += nCurrRecvLen;
		}
	}

	MessageBox(_T("�������ݳɹ�"), _T("��ʾ��Ϣ"), MB_OK);
	return;
}
