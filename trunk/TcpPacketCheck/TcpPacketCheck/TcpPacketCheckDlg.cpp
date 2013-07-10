// TcpPacketCheckDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TcpPacketCheck.h"
#include "TcpPacketCheckDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CTcpPacketCheckDlg 对话框




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


// CTcpPacketCheckDlg 消息处理程序

BOOL CTcpPacketCheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_txtServerIP.SetWindowText(_T("127.0.0.1"));
	m_txtPort.SetWindowText(_T("10002"));
	m_txtRecvLength.SetWindowText(_T("14"));
	m_txtPacketBuffer.SetWindowText(_T("0a 00 00 00 00 10 be cd aa 8f 3c 01 00 00"));

	//初始化TCP链接
	WSADATA wsaData;
	int nErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(nErr != 0)
	{
		MessageBox(_T("本机socket库加载失败，请检查本机socket库版本"), _T("错误信息"), MB_OK);
	}

	m_lstResult.InsertColumn(0, _T("测试项目"), LVCFMT_LEFT, 200);
	m_lstResult.InsertColumn(2, _T("测试结果"), LVCFMT_LEFT, 350);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTcpPacketCheckDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTcpPacketCheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTcpPacketCheckDlg::OnBnClickedButton1()
{
	_ClientInfo objClientInfo;

	//获得发送的IP和端口
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
		MessageBox(_T("必须输入要发送的IP和端口") , _T("错误信息"), MB_OK);
		return;
	}

	int nServerPort = atoi(szServerPort);

	sprintf_s(objClientInfo.m_szServerIP, 30, "%s", szServerIP);
	objClientInfo.m_nPort = nServerPort;

	//获得验证包长度
	CString strRecvLength;
	char szRecvLength[10] = {'\0'};

	m_txtRecvLength.GetWindowText(strRecvLength);

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strRecvLength, strRecvLength.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strRecvLength, nSrcLen, szRecvLength, 30, NULL,NULL);
	szRecvLength[nDecLen] = '\0';

	if(strlen(szRecvLength) == 0)
	{
		MessageBox(_T("必须输入验证接收字节长度") , _T("错误信息"), MB_OK);
		return;
	}

	objClientInfo.m_nRecvLength = atoi(szRecvLength);

	//获取发送Buffer
	CString strPacketBuffer;
	char szPacketBuffer[500] = {'\0'};

	m_txtPacketBuffer.GetWindowText(strPacketBuffer);

	nSrcLen = WideCharToMultiByte(CP_ACP, 0, strPacketBuffer, strPacketBuffer.GetLength(), NULL, 0, NULL, NULL);
	nDecLen = WideCharToMultiByte(CP_ACP, 0, strPacketBuffer, nSrcLen, szPacketBuffer, 500, NULL,NULL);
	szPacketBuffer[nDecLen] = '\0';

	if(strlen(szPacketBuffer) == 0)
	{
		MessageBox(_T("必须输入发送数据包") , _T("错误信息"), MB_OK);
		return;
	}

	CConvertBuffer objConvertBuffer;
	int nSendLength = objConvertBuffer.GetBufferSize(szPacketBuffer, nDecLen);
	char* pSendBuffer = new char[nSendLength];
	objConvertBuffer.Convertstr2charArray(szPacketBuffer, strlen(szPacketBuffer), (unsigned char*)pSendBuffer, nSendLength);

	objClientInfo.m_pSendBuffer = pSendBuffer;
	objClientInfo.m_nSendLength = nSendLength;

	//开始测试
	int nIndex = 0;
	m_lstResult.DeleteAllItems();

	//单个包检测
	CheckTcpPacket(objClientInfo, nIndex++);

	//多个包检测
	CheckMultipleTcpPacket(objClientInfo, nIndex++);

	//无效包检测
	CheckValidPacket(objClientInfo, nIndex++);

	//任意切割数据包检测
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

	//socket创建的准备工作
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//连接远程服务器
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "与[%s:%d]服务器连接失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("单个数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//发送数据
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
			sprintf_s(szResult, 1024, "与[%s:%d]服务器发送数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("单个数据包检测"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//发送完成
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

		sprintf_s(szResult, 1024, "与[%s:%d]发送字节数不匹配，缓冲字节数[%d]，实际发送字节数[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("单个数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{
			
			//如果发送成功了，则处理接收数据
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "与[%s:%d]服务器接收数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("单个数据包检测"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//接收完成
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "与[%s:%d]单数据包检测成功。", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("单个数据包检测"));
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

	//socket创建的准备工作
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//连接远程服务器
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "与[%s:%d]服务器连接失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("多个数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//固定5个数据包同时发送
	char* pData = new char[5 * objClientInfo.m_nSendLength];
	memset(pData, 0, 5 * objClientInfo.m_nSendLength);
	for(int i = 0; i < 5; i++)
	{
		memcpy_s((char* )&pData[i * objClientInfo.m_nSendLength], objClientInfo.m_nSendLength, objClientInfo.m_pSendBuffer, objClientInfo.m_nSendLength);
	}

	//发送数据
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
			sprintf_s(szResult, 1024, "与[%s:%d]服务器发送数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("多个数据包检测"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return false;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//发送完成
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

		sprintf_s(szResult, 1024, "与[%s:%d]发送字节数不匹配，缓冲字节数[%d]，实际发送字节数[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("多个数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 5 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//如果发送成功了，则处理接收数据
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "与[%s:%d]服务器接收数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("多个数据包检测"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//接收完成
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "与[%s:%d]多数据包检测成功(5个同时发送)。", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("多个数据包检测"));
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

	//socket创建的准备工作
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//连接远程服务器
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "与[%s:%d]服务器发送数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("无效数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//随便一个全0的数据包
	char* pData = new char[objClientInfo.m_nSendLength];
	memset(pData, 0, objClientInfo.m_nSendLength);

	//发送数据
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
			sprintf_s(szResult, 1024, "与[%s:%d]无效包被服务器断开，测试成功。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("无效数据包检测"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//发送完成
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

		sprintf_s(szResult, 1024, "与[%s:%d]发送字节数不匹配，缓冲字节数[%d]，实际发送字节数[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("无效数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 5 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//如果发送成功了，则处理接收数据
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "与[%s:%d]无效数据包服务器断开连接，测试成功。", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("无效数据包检测"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return true;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//接收完成
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
	sprintf_s(szResult, 1024, "与[%s:%d]无效包被服务器没有断开，测试失败。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("无效数据包检测"));
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

	//socket创建的准备工作
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(objClientInfo.m_nPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(objClientInfo.m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	DWORD TimeOut = (DWORD)2000;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//连接远程服务器
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		DWORD dwError = GetLastError();
		sprintf_s(szResult, 1024, "与[%s:%d]服务器发送数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}

	//数据包随意切分,2个数据包
	char* pData = new char[2 * objClientInfo.m_nSendLength];
	memset(pData, 0, 2 * objClientInfo.m_nSendLength);
	for(int i = 0; i < 2; i++)
	{
		memcpy_s((char* )&pData[i * objClientInfo.m_nSendLength], objClientInfo.m_nSendLength, objClientInfo.m_pSendBuffer, objClientInfo.m_nSendLength);
	}

	//发送数据（前半部分）
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
			sprintf_s(szResult, 1024, "与[%s:%d]无效包被服务器断开，测试成功。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//发送完成
				blSendFlag = true;
				break;
			}
			else
			{
				nBeginSend += nCurrSendLen;
			}
		}
	}

	//发送数据（后半部分）
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
			sprintf_s(szResult, 1024, "与[%s:%d]无效包被服务器断开，测试成功。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

			nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

			m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
			m_lstResult.SetItemText(nIndex, 1, sszResult);

			delete[] pData;
			return true;
		}
		else
		{
			nTotalSendLen -= nCurrSendLen;
			if(nTotalSendLen == 0)
			{
				//发送完成
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

		sprintf_s(szResult, 1024, "与[%s:%d]发送字节数不匹配，缓冲字节数[%d]，实际发送字节数[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, objClientInfo.m_nSendLength, nTotalSendLen);

		nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

		m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
		m_lstResult.SetItemText(nIndex, 1, sszResult);

		return false;
	}
	else
	{
		int nTotalRecvLen               = 2 * objClientInfo.m_nRecvLength;
		char szRecvBuffData[1024 * 100] = {'\0'};

		while(true)
		{

			//如果发送成功了，则处理接收数据
			nCurrRecvLen = recv(sckClient, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0);
			if(nCurrRecvLen <= 0)
			{
				closesocket(sckClient);

				DWORD dwError = GetLastError();
				sprintf_s(szResult, 1024, "与[%s:%d]服务器接收数据失败，错误号[%d]。", objClientInfo.m_szServerIP, objClientInfo.m_nPort, dwError);

				nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

				m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
				m_lstResult.SetItemText(nIndex, 1, sszResult);

				delete[] pData;
				return false;
			}
			else
			{
				nTotalRecvLen -= nCurrRecvLen;
				if(nTotalRecvLen == 0)
				{
					//接收完成
					break;
				}
				else
				{
					nBeginRecv += nCurrRecvLen;
				}
			}
		}
	}

	sprintf_s(szResult, 1024, "与[%s:%d]数据包任意切割发送检测成功(2个同时发送)。", objClientInfo.m_szServerIP, objClientInfo.m_nPort);

	nDecLen = MultiByteToWideChar(CP_ACP, 0, szResult, -1, sszResult, 1024);

	m_lstResult.InsertItem(nIndex, _T("半包数据包检测"));
	m_lstResult.SetItemText(nIndex, 1, sszResult);

	closesocket(sckClient);
	delete[] pData;
	return true;

}
