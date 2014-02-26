// DlgClientWorkThread.cpp : implementation file
//

#include "stdafx.h"
#include "PSS_ClientManager.h"
#include "DlgClientWorkThread.h"


// CDlgClientWorkThread dialog

IMPLEMENT_DYNAMIC(CDlgClientWorkThread, CDialog)

CDlgClientWorkThread::CDlgClientWorkThread(CWnd* pParent /*=NULL*/)
: CDialog(CDlgClientWorkThread::IDD, pParent)
{
}

CDlgClientWorkThread::~CDlgClientWorkThread()
{
	if(m_pWorkThreadInfo != NULL)
	{
		delete[] m_pWorkThreadInfo;
		m_pWorkThreadInfo   = NULL;
		m_nCheckThreadCount = 0;
	}
}

void CDlgClientWorkThread::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lcWorkThreadInfo);
	DDX_Control(pDX, IDC_EDIT1, m_txtThreadCount);
	DDX_Control(pDX, IDC_EDIT2, m_txtCheckTime);
	DDX_Control(pDX, IDC_LIST2, m_lbCheckLog);
}


BEGIN_MESSAGE_MAP(CDlgClientWorkThread, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgClientWorkThread::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgClientWorkThread::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON8, &CDlgClientWorkThread::OnBnClickedButton8)
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CString CDlgClientWorkThread::GetPageTitle()
{
	return _T("�����߳�״̬");
}


// CDlgClientWorkThread message handlers

void CDlgClientWorkThread::OnBnClickedButton1()
{
	//�鿴�����߳���Ϣ
	m_lcWorkThreadInfo.DeleteAllItems();

	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s WorkThreadState -s", m_pTcpClientConnect->GetKey());
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[100 * 1024] = {'\0'};
	int nRecvLen = 100 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
		return;
	}
	else
	{
		vecWorkThreadInfo objvecWorkThreadInfo;

		int nStrLen       = 0;
		int nPos          = 4;
		int nThreadCount  = 0;
		memcpy_s(&nThreadCount, sizeof(char), &szRecvBuff[nPos], sizeof(char));
		nPos += sizeof(char);

		for(int i = 0; i < nThreadCount; i++)
		{
			//��ʼ��ԭ���ݽṹ
			_WorkThreadInfo WorkThreadInfo;
			sprintf_s(WorkThreadInfo.m_szThreadName, 50, "�����߳�");

			memcpy_s(&WorkThreadInfo.m_nThreadID, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			char szUpdateTime[30] = {'\0'};
			memcpy_s(&WorkThreadInfo.m_nUpdateTime, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);
			struct tm tmDate;
			time_t newRawTime = WorkThreadInfo.m_nUpdateTime;
			localtime_s(&tmDate, &newRawTime);
			sprintf_s(szUpdateTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate.tm_year + 1900, 
				tmDate.tm_mon + 1,
				tmDate.tm_mday,
				tmDate.tm_hour,
				tmDate.tm_min,
				tmDate.tm_sec);

			char szCreateTime[30] = {'\0'};
			memcpy_s(&WorkThreadInfo.m_nCreateTime, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);
			struct tm tmDate1;
			time_t newRawTime1 = WorkThreadInfo.m_nCreateTime;
			localtime_s(&tmDate1, &newRawTime1);
			sprintf_s(szCreateTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate.tm_year + 1900, 
				tmDate1.tm_mon + 1,
				tmDate1.tm_mday,
				tmDate1.tm_hour,
				tmDate1.tm_min,
				tmDate1.tm_sec);

			memcpy_s(&WorkThreadInfo.m_nState, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			memcpy_s(&WorkThreadInfo.m_nRecvPacketCount, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			memcpy_s(&WorkThreadInfo.m_nCommandID, sizeof(short), &szRecvBuff[nPos], sizeof(short));
			nPos += sizeof(short);

			memcpy_s(&WorkThreadInfo.m_nPacketTime, sizeof(short), &szRecvBuff[nPos], sizeof(short));
			nPos += sizeof(short);

			memcpy_s(&WorkThreadInfo.m_nCurrPacketCount, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			//��ʾ�ڽ�����
			wchar_t szzTheadName[50]  = {'\0'};
			wchar_t szzUpdateTime[30] = {'\0'};
			wchar_t szzCreateTime[30] = {'\0'};
			CString strThreadID;
			CString strSendPacketCount;
			CString strRecvPacketCount;
			CString strCurrCommandID;
			CString strPacketTime;
			CString strAvgDisposeCount;

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, WorkThreadInfo.m_szThreadName, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, WorkThreadInfo.m_szThreadName, -1, szzTheadName, 50);

			nSrcLen = MultiByteToWideChar(CP_ACP, 0, szUpdateTime, -1, NULL, 0);
			nDecLen = MultiByteToWideChar(CP_ACP, 0, szUpdateTime, -1, szzUpdateTime, 50);

			nSrcLen = MultiByteToWideChar(CP_ACP, 0, szCreateTime, -1, NULL, 0);
			nDecLen = MultiByteToWideChar(CP_ACP, 0, szCreateTime, -1, szzCreateTime, 50);

			strThreadID.Format(_T("%d"), WorkThreadInfo.m_nThreadID);
			strRecvPacketCount.Format(_T("%d"), WorkThreadInfo.m_nRecvPacketCount);
			strSendPacketCount.Format(_T("%d"), WorkThreadInfo.m_nSendPacketCount);
			strCurrCommandID.Format(_T("%d"), WorkThreadInfo.m_nCommandID);
			strPacketTime.Format(_T("%d"), WorkThreadInfo.m_nPacketTime);
			strAvgDisposeCount.Format(_T("%d"), WorkThreadInfo.m_nCurrPacketCount);

			m_lcWorkThreadInfo.InsertItem(i, _T("�����߳�"));
			m_lcWorkThreadInfo.SetItemText(i, 1, strThreadID);
			m_lcWorkThreadInfo.SetItemText(i, 2, szzCreateTime);
			m_lcWorkThreadInfo.SetItemText(i, 3, szzUpdateTime);
			m_lcWorkThreadInfo.SetItemText(i, 4, strRecvPacketCount);
			m_lcWorkThreadInfo.SetItemText(i, 5, strCurrCommandID);
			m_lcWorkThreadInfo.SetItemText(i, 6, strPacketTime);
			m_lcWorkThreadInfo.SetItemText(i, 7, strAvgDisposeCount);

			objvecWorkThreadInfo.push_back(WorkThreadInfo);
		}
	}
}

void CDlgClientWorkThread::SetTcpClientConnect( CTcpClientConnect* pTcpClientConnect )
{
	m_pTcpClientConnect = pTcpClientConnect;
}

BOOL CDlgClientWorkThread::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_lcWorkThreadInfo.InsertColumn(0, _T("�߳�����"), LVCFMT_CENTER, 100);
	m_lcWorkThreadInfo.InsertColumn(1, _T("�߳�ID"), LVCFMT_CENTER, 100);
	m_lcWorkThreadInfo.InsertColumn(2, _T("�̴߳���ʱ��"), LVCFMT_CENTER, 100);
	m_lcWorkThreadInfo.InsertColumn(3, _T("���ִ��ʱ��"), LVCFMT_CENTER, 100);
	m_lcWorkThreadInfo.InsertColumn(4, _T("���������"), LVCFMT_CENTER, 80);
	m_lcWorkThreadInfo.InsertColumn(5, _T("��ǰ����ID"), LVCFMT_CENTER, 80);
	m_lcWorkThreadInfo.InsertColumn(6, _T("ƽ����ʱ"), LVCFMT_CENTER, 80);
	m_lcWorkThreadInfo.InsertColumn(7, _T("��λ������"), LVCFMT_CENTER, 80);

	m_txtThreadCount.SetWindowText(_T("3"));
	m_txtCheckTime.SetWindowText(_T("60"));

	m_pWorkThreadInfo   = NULL;
	m_nCheckThreadCount = 0;
	m_blTimeRun         = false;

	return TRUE;
}

void CDlgClientWorkThread::OnBnClickedButton2()
{
	//��ʼ���
	if(m_pWorkThreadInfo != NULL)
	{
		delete[] m_pWorkThreadInfo;
		m_pWorkThreadInfo   = NULL;
		m_nCheckThreadCount = 0;
	}

	CString strData;
	m_txtThreadCount.GetWindowText(strData);
	m_nCheckThreadCount = _ttoi(strData);
	m_txtCheckTime.GetWindowText(strData);
	int nTimeInterval = _ttoi(strData);

	if(m_blTimeRun == true)
	{
		//������ڼ�أ���ֹͣ��һ�εļ��
		KillTimer(1);
	}

	m_pWorkThreadInfo = new _WorkThreadInfo[m_nCheckThreadCount];

	m_lbCheckLog.ResetContent();

	SetTimer(1, nTimeInterval * 1000, NULL);
	m_blTimeRun = true;

	CString strTImeNow;

	CTime tm = CTime::GetCurrentTime();
	strTImeNow = tm.Format(_T("[%Y-%m-%d %X]"));
	strData = strTImeNow + _T("��ʼ���...");

	m_lbCheckLog.AddString(strData);
}

void CDlgClientWorkThread::OnBnClickedButton8()
{
	//ֹͣ���
	KillTimer(1);
	m_blTimeRun = false;

	if(m_pWorkThreadInfo != NULL)
	{
		delete[] m_pWorkThreadInfo;
		m_pWorkThreadInfo = NULL;
		m_nCheckThreadCount = 0;
	}

	CString strData;
	CString strTImeNow;

	CTime tm = CTime::GetCurrentTime();
	strTImeNow = tm.Format(_T("[%Y-%m-%d %X]"));
	strData = strTImeNow + _T("ֹͣ���...");

	m_lbCheckLog.AddString(strData);
}

void CDlgClientWorkThread::OnTimer(UINT_PTR nIDEvent)
{
	//��ʱ��
	switch(nIDEvent) 
	{
	case 1:
		CheckWorkThread();
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CDlgClientWorkThread::CheckWorkThread()
{
	char szSendMessage[200] = {'\0'};
	char szCommand[100]     = {'\0'};
	sprintf_s(szCommand, 100, "%s WorkThreadState -s", m_pTcpClientConnect->GetKey());
	int nSendLen = (int)strlen(szCommand); 

	memcpy_s(szSendMessage, 200, &nSendLen, sizeof(int));
	memcpy_s(&szSendMessage[4], 200, &szCommand, nSendLen);

	char szRecvBuff[100 * 1024] = {'\0'};
	int nRecvLen = 100 * 1024;
	bool blState = m_pTcpClientConnect->SendConsoleMessage(szSendMessage, nSendLen + sizeof(int), (char*)szRecvBuff, nRecvLen);
	if(blState == false)
	{
		MessageBox(_T(MESSAGE_SENDERROR) , _T(MESSAGE_TITLE_ERROR), MB_OK);
		return;
	}
	else
	{
		vecWorkThreadInfo objvecWorkThreadInfo;

		int nStrLen       = 0;
		int nPos          = 4;
		int nThreadCount  = 0;
		memcpy_s(&nThreadCount, sizeof(char), &szRecvBuff[nPos], sizeof(char));
		nPos += sizeof(char);

		if(nThreadCount != m_nCheckThreadCount)
		{
			char szError[1024]     = {'\0'};
			wchar_t szzError[1024] = {'\0'};

			sprintf_s(szError, "[����]��Ҫ��ص��߳��������ڵ�ǰ�������߳�������ǰ�������߳���[%d]����ǰ����߳���[%d].�޷���ɼ��", nThreadCount, m_nCheckThreadCount);

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szError, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, szError, -1, szzError, 50);

			m_lbCheckLog.AddString(szzError);

			return;
		}

		//���յ������й����߳���Ϣ
		for(int i = 0; i < nThreadCount; i++)
		{
			//��ʼ��ԭ���ݽṹ
			_WorkThreadInfo WorkThreadInfo;
			sprintf_s(WorkThreadInfo.m_szThreadName, 50, "�����߳�");

			memcpy_s(&WorkThreadInfo.m_nThreadID, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			char szUpdateTime[30] = {'\0'};
			memcpy_s(&WorkThreadInfo.m_nUpdateTime, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);
			struct tm tmDate;
			time_t newRawTime = WorkThreadInfo.m_nUpdateTime;
			localtime_s(&tmDate, &newRawTime);
			sprintf_s(szUpdateTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate.tm_year + 1900, 
				tmDate.tm_mon + 1,
				tmDate.tm_mday,
				tmDate.tm_hour,
				tmDate.tm_min,
				tmDate.tm_sec);

			char szCreateTime[30] = {'\0'};
			memcpy_s(&WorkThreadInfo.m_nCreateTime, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);
			struct tm tmDate1;
			time_t newRawTime1 = WorkThreadInfo.m_nCreateTime;
			localtime_s(&tmDate1, &newRawTime1);
			sprintf_s(szCreateTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate.tm_year + 1900, 
				tmDate1.tm_mon + 1,
				tmDate1.tm_mday,
				tmDate1.tm_hour,
				tmDate1.tm_min,
				tmDate1.tm_sec);

			memcpy_s(&WorkThreadInfo.m_nState, sizeof(char), &szRecvBuff[nPos], sizeof(char));
			nPos += sizeof(char);

			memcpy_s(&WorkThreadInfo.m_nRecvPacketCount, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			memcpy_s(&WorkThreadInfo.m_nCommandID, sizeof(short), &szRecvBuff[nPos], sizeof(short));
			nPos += sizeof(short);

			memcpy_s(&WorkThreadInfo.m_nPacketTime, sizeof(short), &szRecvBuff[nPos], sizeof(short));
			nPos += sizeof(short);

			memcpy_s(&WorkThreadInfo.m_nCurrPacketCount, sizeof(int), &szRecvBuff[nPos], sizeof(int));
			nPos += sizeof(int);

			objvecWorkThreadInfo.push_back(WorkThreadInfo);
		}

		for(int i = 0; i < m_nCheckThreadCount; i++)
		{
			if(m_pWorkThreadInfo[i].m_nCreateTime == 0)
			{
				//��һ�ε��ã�ֻ����
				m_pWorkThreadInfo[i] = objvecWorkThreadInfo[i];
			}
			else
			{
				//������������ݣ����бȽ��ж�
				if(m_pWorkThreadInfo[i].m_nCreateTime != objvecWorkThreadInfo[i].m_nCreateTime)
				{
					char szError[1024]     = {'\0'};
					char szThreadTime[100] = {'\0'};
					char szCurrTime[100]   = {'\0'};
					wchar_t szzError[1024] = {'\0'};

					struct tm tmDate;
					time_t newRawTime = m_pWorkThreadInfo[i].m_nCreateTime;
					localtime_s(&tmDate, &newRawTime);
					sprintf_s(szThreadTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate.tm_year + 1900, 
						tmDate.tm_mon + 1,
						tmDate.tm_mday,
						tmDate.tm_hour,
						tmDate.tm_min,
						tmDate.tm_sec);

					struct tm tmDate1;
					time_t newRawTime1 = objvecWorkThreadInfo[i].m_nCreateTime;
					localtime_s(&tmDate1, &newRawTime1);
					sprintf_s(szCurrTime, 30, "%04d-%02d-%02d %02d:%02d:%02d", tmDate1.tm_year + 1900, 
						tmDate1.tm_mon + 1,
						tmDate1.tm_mday,
						tmDate1.tm_hour,
						tmDate1.tm_min,
						tmDate1.tm_sec);

					sprintf_s(szError, "[����]ԭ�߳�ʱ����[%s], ��ǰ�߳�����ʱ��[%s]", szThreadTime, szCurrTime);

					int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szError, -1, NULL, 0);
					int nDecLen = MultiByteToWideChar(CP_ACP, 0, szError, -1, szzError, 50);

					m_lbCheckLog.AddString(szzError);
				}
			}
		}
	}
}
void CDlgClientWorkThread::OnClose()
{

	CDialog::OnClose();
}
