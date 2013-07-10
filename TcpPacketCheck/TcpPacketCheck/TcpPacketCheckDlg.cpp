// TcpPacketCheckDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TcpPacketCheck.h"
#include "TcpPacketCheckDlg.h"

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


// CTcpPacketCheckDlg �Ի���




CTcpPacketCheckDlg::CTcpPacketCheckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTcpPacketCheckDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTcpPacketCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtPacketBuffer);
	DDX_Control(pDX, IDC_EDIT2, m_txtServerIP);
	DDX_Control(pDX, IDC_EDIT3, m_txtPort);
	DDX_Control(pDX, IDC_EDIT4, m_txtRecvLength);
	DDX_Control(pDX, IDC_LIST1, m_lstResult);
}

BEGIN_MESSAGE_MAP(CTcpPacketCheckDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CTcpPacketCheckDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CTcpPacketCheckDlg ��Ϣ�������

BOOL CTcpPacketCheckDlg::OnInitDialog()
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
	m_txtServerIP.SetWindowText(_T("127.0.0.1"));
	m_txtPort.SetWindowText(_T("10002"));
	m_txtRecvLength.SetWindowText(_T("14"));
	m_txtPacketBuffer.SetWindowText(_T("0a 00 00 00 00 10 be cd aa 8f 3c 01 00 00"));

	//��ʼ��TCP����
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("����socket�����ʧ�ܣ����鱾��socket��汾"), _T("������Ϣ"), MB_OK);
	}

	m_lstResult.InsertColumn(0, _T("������Ŀ"), LVCFMT_LEFT, 200);
	m_lstResult.InsertColumn(2, _T("���Խ��"), LVCFMT_LEFT, 350);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTcpPacketCheckDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTcpPacketCheckDlg::OnPaint()
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
HCURSOR CTcpPacketCheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTcpPacketCheckDlg::OnBnClickedButton1()
{
	_ClientInfo objClientInfo;

	//��÷��͵�IP�Ͷ˿�
	CString strServerIP;
	CString strPort;
	char szServerIP[30]   = {'\0'};
	char szServerPort[20] = {'\0'};

	m_txtServerIP.GetWindowText(strServerIP);
	m_txtPort.GetWindowText(strPort);

	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strServerIP, strServerIP.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strServerIP, nSrcLen, szServerIP, 30, NULL,NULL);
	szServerIP[nDecLen] = '\0';

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strPort, strPort.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strPort, nSrcLen, szServerPort, 30, NULL,NULL);
	szServerPort[nDecLen] = '\0';

	if(strlen(szServerPort) == 0 || strlen(szServerIP) == 0)
	{
		MessageBox(_T("��������Ҫ���͵�IP�Ͷ˿�") , _T("������Ϣ"), MB_OK);
		return;
	}

	int nServerPort = atoi(szServerPort);

	sprintf_s(objClientInfo.m_szServerIP, 30, "%s", szServerIP);
	objClientInfo.m_nPort = nServerPort;

	//�����֤������
	CString strRecvLength;
	char szRecvLength[10] = {'\0'};

	m_txtRecvLength.GetWindowText(strRecvLength);

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strRecvLength, strRecvLength.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strRecvLength, nSrcLen, szRecvLength, 30, NULL,NULL);
	szRecvLength[nDecLen] = '\0';

	if(strlen(szRecvLength) == 0)
	{
		MessageBox(_T("����������֤�����ֽڳ���") , _T("������Ϣ"), MB_OK);
		return;
	}

	objClientInfo.m_nRecvLength = atoi(szRecvLength);

	//��ȡ����Buffer
	CString strPacketBuffer;
	char szPacketBuffer[500] = {'\0'};

	m_txtPacketBuffer.GetWindowText(strPacketBuffer);

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strPacketBuffer, strPacketBuffer.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strPacketBuffer, nSrcLen, szPacketBuffer, 500, NULL,NULL);
	szPacketBuffer[nDecLen] = '\0';

	if(strlen(szPacketBuffer) == 0)
	{
		MessageBox(_T("�������뷢�����ݰ�") , _T("������Ϣ"), MB_OK);
		return;
	}

	CConvertBuffer objConvertBuffer;
	int nSendLength = objConvertBuffer.GetBufferSize(szPacketBuffer, nDecLen);
	char* pSendBuffer = new char[nSendLength];
	objConvertBuffer.Convertstr2charArray(szPacketBuffer, strlen(szPacketBuffer), (unsigned char*)pSendBuffer, nSendLength);

	objClientInfo.m_pSendBuffer = pSendBuffer;
	objClientInfo.m_nSendLength = nSendLength;

	//��ʼ����
	int nIndex = 0;
	m_lstResult.DeleteAllItems();

	//���������
	CheckTcpPacket(objClientInfo, nIndex++);

	//��������
	CheckMultipleTcpPacket(objClientInfo, nIndex++);

	//��Ч�����
	CheckValidPacket(objClientInfo, nIndex++);

	//�����и����ݰ����
	CheckHalfPacket(objClientInfo, nIndex++);

	delete[] pSendBuffer;
}

bool CTcpPacketCheckDlg::CheckTcpPacket( _ClientInfo& objClientInfo, int nIndex)
{
	SOCKET sckClient;
	char szResult[1024]     = {'\0'};
	wchar_t sszResult[1024] = {'\0'};
	int nSrcLen = 0;
	int nDecLen = 0;

	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//����Զ�̷�����
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//��������
	int nTotalSendLen = objClientInfo.m_nSendLength;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, objClientInfo.m_pSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

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

	if(blSendFlag == false)
	{
		closesocket(sckClient);

		sprintf_s(szResult, 1024, "��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{
			
			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//�������
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "��[%s:%d]�����ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);

	return true;
}

bool CTcpPacketCheckDlg::CheckMultipleTcpPacket( _ClientInfo& objClientInfo, int nIndex )
{
	SOCKET sckClient;
	char szResult[1024]     = {'\0'};
	wchar_t sszResult[1024] = {'\0'};
	int nSrcLen = 0;
	int nDecLen = 0;

	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//����Զ�̷�����
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//�̶�5�����ݰ�ͬʱ����
	char* pData = new char[5 * objClientInfo.m_nSendLength];
	memset(pData, 0, 5 * objClientInfo.m_nSendLength);
	for(int i = 0; i < 5; i++)
	{
		memcpy_s((char* )&pData[i * objClientInfo.m_nSendLength], objClientInfo.m_nSendLength, objClientInfo.m_pSendBuffer, objClientInfo.m_nSendLength);
	}

	//��������
	int nTotalSendLen = 5 * objClientInfo.m_nSendLength;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, pData + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
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

	if(blSendFlag == false)
	{
		closesocket(sckClient);

		sprintf_s(szResult, 1024, "��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 5 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//�������
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "��[%s:%d]�����ݰ����ɹ�(5��ͬʱ����)��", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);
	delete[] pData;
	return true;
}

bool CTcpPacketCheckDlg::CheckValidPacket( _ClientInfo& objClientInfo, int nIndex)
{
	SOCKET sckClient;
	char szResult[1024]     = {'\0'};
	wchar_t sszResult[1024] = {'\0'};
	int nSrcLen = 0;
	int nDecLen = 0;

	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//����Զ�̷�����
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("��Ч���ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//���һ��ȫ0�����ݰ�
	char* pData = new char[objClientInfo.m_nSendLength];
	memset(pData, 0, objClientInfo.m_nSendLength);

	//��������
	int nTotalSendLen = 5 * objClientInfo.m_nSendLength;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, pData + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "��[%s:%d]��Ч�����������Ͽ������Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("��Ч���ݰ����"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
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

	if(blSendFlag == false)
	{
		closesocket(sckClient);

		sprintf_s(szResult, 1024, "��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("��Ч���ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 5 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "��[%s:%d]��Ч���ݰ��������Ͽ����ӣ����Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("��Ч���ݰ����"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return true;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//�������
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	closesocket(sckClient);

	DWORD dwError = GetLastError();
	sprintf_s(szResult, 1024, "��[%s:%d]��Ч����������û�жϿ�������ʧ�ܡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("��Ч���ݰ����"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	delete[] pData;
	return false;
}

bool CTcpPacketCheckDlg::CheckHalfPacket( _ClientInfo& objClientInfo, int nIndex )
{
	SOCKET sckClient;
	char szResult[1024]     = {'\0'};
	wchar_t sszResult[1024] = {'\0'};
	int nSrcLen = 0;
	int nDecLen = 0;

	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//����Զ�̷�����
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//���ݰ������з�,2�����ݰ�
	char* pData = new char[2 * objClientInfo.m_nSendLength];
	memset(pData, 0, 2 * objClientInfo.m_nSendLength);
	for(int i = 0; i < 2; i++)
	{
		memcpy_s((char* )&pData[i * objClientInfo.m_nSendLength], objClientInfo.m_nSendLength, objClientInfo.m_pSendBuffer, objClientInfo.m_nSendLength);
	}

	//�������ݣ�ǰ�벿�֣�
	int nTotalSendLen = 2;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, pData + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "��[%s:%d]��Ч�����������Ͽ������Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
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

	//�������ݣ���벿�֣�
	nTotalSendLen = 2 * objClientInfo.m_nSendLength - 2;
	nBeginSend    = 0;
	nCurrSendLen  = 0;
	blSendFlag   = false;
	nBeginRecv    = 0;
	nCurrRecvLen  = 0;
	blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, pData + 2 + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "��[%s:%d]��Ч�����������Ͽ������Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
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

	if(blSendFlag == false)
	{
		closesocket(sckClient);

		sprintf_s(szResult, 1024, "��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 2 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//�������
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "��[%s:%d]���ݰ������и�ͼ��ɹ�(2��ͬʱ����)��", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);
	delete[] pData;
	return true;

}
