// DlgTrackIP.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PSS_ClientManager.h"
#include "DlgTrackIP.h"


// CDlgTrackIP �Ի���

IMPLEMENT_DYNAMIC(CDlgTrackIP, CDialog)

CDlgTrackIP::CDlgTrackIP(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTrackIP::IDD, pParent)
{

}

CDlgTrackIP::~CDlgTrackIP()
{
}

void CDlgTrackIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtTrackIP);
	DDX_Control(pDX, IDC_LIST1, m_lcTrackIPHistory);
	DDX_Control(pDX, IDC_EDIT2, m_txtConnectID);
	DDX_Control(pDX, IDC_EDIT7, m_txtIPInfo);
}


BEGIN_MESSAGE_MAP(CDlgTrackIP, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgTrackIP::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgTrackIP::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON8, &CDlgTrackIP::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgTrackIP::OnBnClickedButton6)
END_MESSAGE_MAP()

CString CDlgTrackIP::GetPageTitle()
{
	return _T("IP׷����");
}

void CDlgTrackIP::SetTcpClientConnect( CTcpClientConnect* pTcpClientConnect )
{
	m_pTcpClientConnect = pTcpClientConnect;
}

// CDlgTrackIP ��Ϣ�������

BOOL CDlgTrackIP::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_lcTrackIPHistory.InsertColumn(0, _T("IP��ַ"), LVCFMT_CENTER, 70);
	m_lcTrackIPHistory.InsertColumn(1, _T("�˿�"), LVCFMT_CENTER, 50);
	m_lcTrackIPHistory.InsertColumn(2, _T("����"), LVCFMT_CENTER, 50);
	m_lcTrackIPHistory.InsertColumn(3, _T("����ʱ��"), LVCFMT_CENTER, 150);
	m_lcTrackIPHistory.InsertColumn(4, _T("�Ͽ�ʱ��"), LVCFMT_CENTER, 150);
	m_lcTrackIPHistory.InsertColumn(5, _T("�����ֽ�"), LVCFMT_CENTER, 70);
	m_lcTrackIPHistory.InsertColumn(6, _T("�����ֽ�"), LVCFMT_CENTER, 70);

	DWORD dwStyle = m_lcTrackIPHistory.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	m_lcTrackIPHistory.SetExtendedStyle(dwStyle); //������չ���

	return TRUE;
}

void CDlgTrackIP::OnBnClickedButton1()
{
	//����IP
	char szIP[20]      = {'\0'};
	CString strIP;

	m_txtTrackIP.GetWindowText(strIP);

	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strIP, strIP.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strIP, nSrcLen, szIP, 20, NULL,NULL);
	szIP[nDecLen] = '\0';

	if(strlen(szIP) == 0)
	{
		MessageBox(_T(MESSAGE_INSERT_NULL) , _T(MESSAGE_TITLE_ERROR), MB_OK);
		return;
	}

	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s SetTrackIP -c %s ", m_pTcpClientConnect->GetKey(), szIP);
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[10 * 1024] = {'\0'};
	int nRecvLen = 10 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
	}
	else
	{
		int nStrLen      = 0;
		int nPos         = 0;
		int nResult      = 0;
		memcpy_s(&nResult, sizeof(char), &szRecvBuff[nPos], sizeof(char));
		nPos += sizeof(char);

		if(nResult == 0)
		{
			MessageBox(_T(MESSAGE_RESULT_SUCCESS) , _T(MESSAGE_TITLE_ERROR), MB_OK);
		}
		else
		{
			MessageBox(_T(MESSAGE_RESULT_FAIL) , _T(MESSAGE_TITLE_SUCCESS), MB_OK);
		}
	}

}

void CDlgTrackIP::OnBnClickedButton2()
{
	//ȡ������IP
	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s DelTrackIP -a", m_pTcpClientConnect->GetKey());
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[10 * 1024] = {'\0'};
	int nRecvLen = 10 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
	}
	else
	{
		int nStrLen      = 0;
		int nPos         = 0;
		int nResult      = 0;
		memcpy_s(&nResult, sizeof(char), &szRecvBuff[nPos], sizeof(char));
		nPos += sizeof(char);

		if(nResult == 0)
		{
			MessageBox(_T(MESSAGE_RESULT_SUCCESS) , _T(MESSAGE_TITLE_ERROR), MB_OK);
		}
		else
		{
			MessageBox(_T(MESSAGE_RESULT_FAIL) , _T(MESSAGE_TITLE_SUCCESS), MB_OK);
		}
	}
}

void CDlgTrackIP::OnBnClickedButton8()
{
	//������ʷ��¼
	m_lcTrackIPHistory.DeleteAllItems();

	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s GetTrackIPInfo -a", m_pTcpClientConnect->GetKey());
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[10 * 1024] = {'\0'};
	int nRecvLen = 10 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
	}
	else
	{
		int nStrLen      = 0;
		int nPos         = 0;
		int nCount       = 0;
		memcpy_s(&nCount, sizeof(short), &szRecvBuff[nPos], sizeof(short));
		nPos += sizeof(short);

		for(int i = 0; i < nCount; i++)
		{
			//��ʼ��ԭ���ݽṹ
			char szIP[20]         = {'\0'};
			int  nPort            = 0;
			char szBeginTime[100] = {'\0'};
			char szEndTime[100]   = {'\0'};
			int  nRecvByteSize    = 0;
			int  nSendByteSize    = 0;
			int  nState           = 0;

			memcpy_s(&nStrLen, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			//���IP��ַ
			memcpy_s(szIP, nStrLen, &szRecvBuff[nPos], nStrLen);
			nPos += nStrLen;
			szIP[nStrLen] = '\0';

			//�õ��˿�
			memcpy_s(&nPort, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			//�õ���ʼʱ��
			memcpy_s(&nStrLen, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			memcpy_s(szBeginTime, nStrLen, &szRecvBuff[nPos], nStrLen);
			nPos += nStrLen;
			szBeginTime[nStrLen] = '\0';

			//�õ�����ʱ��
			memcpy_s(&nStrLen, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			memcpy_s(szEndTime, nStrLen, &szRecvBuff[nPos], nStrLen);
			nPos += nStrLen;
			szEndTime[nStrLen] = '\0';

			//�õ������ֽ���
			memcpy_s(&nRecvByteSize, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			//�õ������ֽ���
			memcpy_s(&nSendByteSize, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			//�õ�״̬λ
			memcpy_s(&nState, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			//��ʾ��������
			wchar_t swzIP[200]         = {'\0'};
			wchar_t swzzBeginTime[100] = {'\0'};
			wchar_t swzzEndTime[100]   = {'\0'};

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szIP, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, szIP, -1, swzIP, 200);

			nSrcLen = MultiByteToWideChar(CP_ACP, 0, szBeginTime, -1, NULL, 0);
			nDecLen = MultiByteToWideChar(CP_ACP, 0, szBeginTime, -1, swzzBeginTime, 100);

			nSrcLen = MultiByteToWideChar(CP_ACP, 0, szEndTime, -1, NULL, 0);
			nDecLen = MultiByteToWideChar(CP_ACP, 0, szEndTime, -1, swzzBeginTime, 100);

			CString strData;

			m_lcTrackIPHistory.InsertItem(i, swzIP);
			strData.Format(_T("%d"), nPort);
			m_lcTrackIPHistory.SetItemText(i, 1, strData);
			if(nState == 1)
			{
				m_lcTrackIPHistory.SetItemText(i, 2, _T("�ر�"));
			}
			else
			{

				m_lcTrackIPHistory.SetItemText(i, 2, _T("��δ�ر�"));
			}

			m_lcTrackIPHistory.SetItemText(i, 3, swzzBeginTime);
			if(nState == 1)
			{
				m_lcTrackIPHistory.SetItemText(i, 4, swzzBeginTime);
			}
			else
			{

				m_lcTrackIPHistory.SetItemText(i, 4, _T("��δ�ر�"));
			}
			strData.Format(_T("%d"), nRecvByteSize);
			m_lcTrackIPHistory.SetItemText(i, 5, strData);
			strData.Format(_T("%d"), nSendByteSize);
			m_lcTrackIPHistory.SetItemText(i, 6, strData);
		}
	}
}

void CDlgTrackIP::OnBnClickedButton6()
{
	//����ConnectID�����ص�����

	//�õ�ConnectID
	int nConnectID = 0;
	CString strData;

	m_txtConnectID.GetWindowText(strData);

	nConnectID = _ttoi(strData);

	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s GetConnectIPInfo -s %d", m_pTcpClientConnect->GetKey(), nConnectID);
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[10 * 1024] = {'\0'};
	int nRecvLen = 10 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
	}
	else
	{
		int nStrLen      = 0;
		int nPos         = 0;
		int nResult      = 0;
		memcpy_s(&nResult, sizeof(short), &szRecvBuff[nPos], sizeof(short));
		nPos += sizeof(short);

		if(nResult == 1)
		{
			m_txtIPInfo.SetWindowText(_T(""));
			MessageBox(_T("��ǰConnectID��������Ϣ��") , _T(MESSAGE_TITLE_ERROR), MB_OK);
		}
		else
		{
			char szIP[20]         = {'\0'};
			int  nPort            = 0;

			memcpy_s(&nStrLen, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			//���IP��ַ
			memcpy_s(szIP, nStrLen, &szRecvBuff[nPos], nStrLen);
			nPos += nStrLen;
			szIP[nStrLen] = '\0';

			//�õ��˿�
			memcpy_s(&nPort, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			char szData[100] ={'\0'};
			sprintf_s(szData, 100, "%s:%d", szIP, nPort);

			wchar_t swzIP[200]         = {'\0'};

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szData, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, szData, -1, swzIP, 200);

			m_txtIPInfo.SetWindowText(swzIP);
		}

	}
}
