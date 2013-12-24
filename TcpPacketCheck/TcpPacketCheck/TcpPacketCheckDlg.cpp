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
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, OnCustomDraw)
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
	m_txtPacketBuffer.SetWindowText(_T("be cd aa 8f 3c 01 00 00"));

	//��ʼ��TCP����
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("����socket�����ʧ�ܣ����鱾��socket��汾"), _T("������Ϣ"), MB_OK);
	}

	m_lstResult.InsertColumn(0, _T("������Ŀ"), LVCFMT_LEFT, 220);
	m_lstResult.InsertColumn(2, _T("���Խ��"), LVCFMT_LEFT, 430);

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

	//�������ݰ�������ͷ
	CheckIsHead(objClientInfo, nIndex++);

	//���ֱ�ӷ������ݰ�(͸��ָ��)
	CheckIsNoHead(objClientInfo, nIndex++);

	//������ͷ�Ļ���
	CheckIsHeadBuffer(objClientInfo, nIndex++);

	//��ⲻ����ͷ����
	CheckIsNoHeadBuffer(objClientInfo, nIndex++);

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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_HEAD;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//��������
	int nTotalSendLen = nSendLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, szSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("�������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = nSendLen;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{
			
			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

	sprintf_s(szResult, 1024, "[s]��[%s:%d]�����ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_HEAD;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//�̶�5�����ݰ�ͬʱ����
	char* pData = new char[5 * nSendLen];
	memset(pData, 0, 5 * nSendLen);
	for(int i = 0; i < 5; i++)
	{
		memcpy_s((char* )&pData[i * nSendLen], nSendLen, szSendBuffer, nSendLen);
	}

	//��������
	int nTotalSendLen = 5 * nSendLen;
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
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 5 * nSendLen;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

	sprintf_s(szResult, 1024, "[s]��[%s:%d]�����ݰ����ɹ�(5��ͬʱ����)��", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��Ч�����������Ͽ�������ʧ�ܡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

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
				sprintf_s(szResult, 1024, "[s]��[%s:%d]��Ч���ݰ��������Ͽ����ӣ����Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

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
	sprintf_s(szResult, 1024, "[e]��[%s:%d]��Ч����������û�жϿ�������ʧ�ܡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_HEAD;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//���ݰ������з�,2�����ݰ�
	char* pData = new char[2 * nSendLen];
	memset(pData, 0, 2 * nSendLen);
	for(int i = 0; i < 2; i++)
	{
		memcpy_s((char* )&pData[i * nSendLen], nSendLen, szSendBuffer, nSendLen);
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
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��Ч�����������Ͽ�������ʧ�ܡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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
	nTotalSendLen = 2 * nSendLen - 2;
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
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��Ч�����������Ͽ������Գɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 2 * nSendLen;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

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

	sprintf_s(szResult, 1024, "[s]��[%s:%d]���ݰ������и�ͼ��ɹ�(2��ͬʱ����)��", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("������ݰ����"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);
	delete[] pData;
	return true;

}

bool CTcpPacketCheckDlg::CheckIsHead( _ClientInfo& objClientInfo, int nIndex )
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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_HEAD;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//��������
	int nTotalSendLen = nSendLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, szSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
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
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
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

		//���������ݵ���ȷ��
		int nRecvPacketSize = 0;
		int nReturnCommand  = 0;
		char szTempData[8]  = {'\0'};

		memcpy_s((char* )&nRecvPacketSize, sizeof(int), (char* )&szRecvBuffData[0], sizeof(int));
		memcpy_s((char* )&nReturnCommand, sizeof(short), (char* )&szRecvBuffData[4], sizeof(short));
		memcpy_s((char* )szTempData, 8 * sizeof(char), (char* )&szRecvBuffData[6], 8 * sizeof(char));

		//��ⷵ�ذ�ͷ��С
		if(nRecvPacketSize != 10)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���շ��ذ���С���ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ���������
		if(nReturnCommand != (int)COMMAND_AUTOTEST_RETUEN_HEAD)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����������ֲ��ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ�������
		int nRet = memcmp(szTempData, objClientInfo.m_pSendBuffer, 8);
		if(nRet != 0)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����ݺͷ������ݲ�����", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}
	}

	sprintf_s(szResult, 1024, "[s]��[%s:%d]���ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("���ݰ�����ͷ(PacketParse)���"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);

	return true;
}

bool CTcpPacketCheckDlg::CheckIsNoHead( _ClientInfo& objClientInfo, int nIndex )
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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_NOHEAD;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//��������
	int nTotalSendLen = nSendLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, szSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = nSendLen;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
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

		//���������ݵ���ȷ��
		int nRecvPacketSize = 0;
		int nReturnCommand  = 0;
		char szTempData[8]  = {'\0'};

		memcpy_s((char* )&nRecvPacketSize, sizeof(int), (char* )&szRecvBuffData[0], sizeof(int));
		memcpy_s((char* )&nReturnCommand, sizeof(short), (char* )&szRecvBuffData[4], sizeof(short));
		memcpy_s((char* )szTempData, 8 * sizeof(char), (char* )&szRecvBuffData[6], 8 * sizeof(char));

		//��ⷵ�ذ�ͷ��С
		if(nRecvPacketSize != 10)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���շ��ذ���С���ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ���������
		if(nReturnCommand != (int)COMMAND_AUTOTEST_NOHEAD)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����������ֲ��ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ�������
		int nRet = memcmp(szTempData, objClientInfo.m_pSendBuffer, 8);
		if(nRet != 0)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����ݺͷ������ݲ�����", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}
	}

	sprintf_s(szResult, 1024, "[s]��[%s:%d]���ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("���ݰ�͸��(PacketParse)���"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);

	return true;
}


bool CTcpPacketCheckDlg::CheckIsHeadBuffer( _ClientInfo& objClientInfo, int nIndex )
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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_HEADBUFF;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//��������
	int nTotalSendLen = nSendLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, szSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = nSendLen * 2;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
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

		//���������ݵ���ȷ��
		int nRecvPacketSize = 0;
		int nReturnCommand  = 0;
		char szTempData[8]  = {'\0'};

		memcpy_s((char* )&nRecvPacketSize, sizeof(int), (char* )&szRecvBuffData[0], sizeof(int));
		memcpy_s((char* )&nReturnCommand, sizeof(short), (char* )&szRecvBuffData[4], sizeof(short));
		memcpy_s((char* )szTempData, 8 * sizeof(char), (char* )&szRecvBuffData[6], 8 * sizeof(char));

		int nRecvPacketSize1 = 0;
		int nReturnCommand1  = 0;
		char szTempData1[8]  = {'\0'};

		memcpy_s((char* )&nRecvPacketSize1, sizeof(int), (char* )&szRecvBuffData[14], sizeof(int));
		memcpy_s((char* )&nReturnCommand1, sizeof(short), (char* )&szRecvBuffData[18], sizeof(short));
		memcpy_s((char* )szTempData1, 8 * sizeof(char), (char* )&szRecvBuffData[20], 8 * sizeof(char));

		//��ⷵ�ذ�ͷ��С
		if(nRecvPacketSize != 10 || nRecvPacketSize1 != 10)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���շ��ذ���С���ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ���������
		if(nReturnCommand != (int)COMMAND_AUTOTEST_RETUEN_HEADBUFF || nReturnCommand1 != COMMAND_AUTOTEST_RETUEN_HEADBUFF)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����������ֲ��ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ�������
		int nRet  = memcmp(szTempData, objClientInfo.m_pSendBuffer, 8);
		int nRet1 = memcmp(szTempData1, objClientInfo.m_pSendBuffer, 8);
		if(nRet != 0 && nRet1 != 0)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����ݺͷ������ݲ�����", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}
	}

	sprintf_s(szResult, 1024, "[s]��[%s:%d]���ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("������ͷ�Ļ���(PacketParse)���"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);

	return true;
}

bool CTcpPacketCheckDlg::CheckIsNoHeadBuffer( _ClientInfo& objClientInfo, int nIndex )
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
		sprintf_s(szResult, 1024, "[e]��[%s:%d]����������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//ƴװ���Ͱ���
	char szSendBuffer[MAX_BUFF_200] ={'\0'};

	short sCommand = (short)COMMAND_AUTOTEST_NOHEADBUFF;
	int nPacketLen = objClientInfo.m_nSendLength + 2;

	memcpy_s(szSendBuffer, sizeof(int), (char* )&nPacketLen, sizeof(int));
	memcpy_s((char* )&szSendBuffer[4], sizeof(short), (char* )&sCommand, sizeof(short));
	memcpy_s((char* )&szSendBuffer[6], sizeof(char) * objClientInfo.m_nSendLength, (char* )objClientInfo.m_pSendBuffer, sizeof(char) * objClientInfo.m_nSendLength);
	int nSendLen = nPacketLen + 4;

	//��������
	int nTotalSendLen = nSendLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	bool blSendFlag   = false;
	int nBeginRecv    = 0;
	int nCurrRecvLen  = 0;
	bool blRecvFlag   = false;
	while(true)
	{
		nCurrSendLen = send(sckClient, szSendBuffer + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);

			DWORD dwError = GetLastError();
			sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
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

		sprintf_s(szResult, 1024, "[e]��[%s:%d]�����ֽ�����ƥ�䣬�����ֽ���[%d]��ʵ�ʷ����ֽ���[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = nSendLen;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//������ͳɹ��ˣ������������
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "[e]��[%s:%d]��������������ʧ�ܣ������[%d]��", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
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

		//���������ݵ���ȷ��
		int nRecvPacketSize = 0;
		int nReturnCommand  = 0;
		int nData1          = 0;
		int nData2          = 0; 

		memcpy_s((char* )&nRecvPacketSize, sizeof(int), (char* )&szRecvBuffData[0], sizeof(int));
		memcpy_s((char* )&nReturnCommand, sizeof(short), (char* )&szRecvBuffData[4], sizeof(short));
		memcpy_s((char* )&nData1, sizeof(int), (char* )&szRecvBuffData[6], sizeof(int));
		memcpy_s((char* )&nData2, sizeof(int), (char* )&szRecvBuffData[10], sizeof(int));

		//��ⷵ�ذ�ͷ��С
		if(nRecvPacketSize != 10)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���շ��ذ���С���ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ���������
		if(nReturnCommand != (int)COMMAND_AUTOTEST_RETUEN_NOHEADBUFF)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����������ֲ��ԡ�", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}

		//�жϷ�������
		if(nData1 != 32 || nData2 != 64)
		{
			closesocket(sckClient);

			sprintf_s(szResult, 1024, "[e]��[%s:%d]���ܰ����ݺͷ������ݲ�����", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}
	}

	sprintf_s(szResult, 1024, "[s]��[%s:%d]���ݰ����ɹ���", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("��������ͷ�Ļ���(PacketParse)���"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);

	return true;
}

//��������ɫ
void CTcpPacketCheckDlg::OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR; 
	*pResult  =  CDRF_DODEFAULT;

	switch (lplvcd->nmcd.dwDrawStage) 
	{ 
	case CDDS_PREPAINT : 
		{ 
			*pResult = CDRF_NOTIFYITEMDRAW; 
			return; 
		} 
	case CDDS_ITEMPREPAINT: 
		{ 
			*pResult = CDRF_NOTIFYSUBITEMDRAW; 
			return; 
		} 
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{ 

			int nItem = static_cast<int>(lplvcd->nmcd.dwItemSpec);
			COLORREF clrNewTextColor, clrNewBkColor;

			//�ж������еĹؼ���
			CString strText;
			strText = m_lstResult.GetItemText(nItem, 1);

			//Ѱ�ҹؼ���
			int nColor = 0;

			if(strText.Find(_T("[s]")) >= 0)
			{
				nColor = 1;
			}
			else if(strText.Find(_T("[e]")) >= 0)
			{
				nColor = 2;
			}

			clrNewTextColor=RGB(0,0,0);
			if(nColor == 1)
			{
				clrNewBkColor = RGB(0, 255, 0); //�ɹ�����ɫ 
			}
			else if(nColor == 2)
			{
				clrNewBkColor = RGB(255, 0, 0); //ʧ�ܣ���ɫ
			}
			else
			{
				clrNewBkColor = RGB(255, 251, 240);   //δ֪����ɫ
			}

			lplvcd->clrText = clrNewTextColor;
			lplvcd->clrTextBk = clrNewBkColor;

			*pResult = CDRF_DODEFAULT;  
			return; 
		} 
	} 
}
