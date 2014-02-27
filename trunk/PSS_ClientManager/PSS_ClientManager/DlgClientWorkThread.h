#pragma once
#include "afxcmn.h"
#include "ClientDefine.h"
#include "TcpClientConnect.h"
#include "XmlOpeation.h"

#include <vector>
#include <time.h>
#include "afxwin.h"

using namespace std;

#define CHECKSERVER_FILE "./CheckServer.xml"

//����߳���Ϣ
struct _WorkThreadInfo
{
	char                m_szThreadName[50];   //�߳�����
	int                 m_nThreadID;         //�߳�ID
	int                 m_nUpdateTime;       //�߳���������ݵ�ʱ��
	int                 m_nCreateTime;       //�߳���������ݵ�ʱ��
	int                 m_nState;            //0Ϊû���������ڴ���1Ϊ���ڴ���������ݡ�
	int                 m_nRecvPacketCount;  //���߳��½��հ��Ĵ������
	int                 m_nSendPacketCount;  //���߳��·��Ͱ��Ĵ������
	int                 m_nCommandID;        //��ǰ���������ID
	int                 m_nPacketTime;       //ƽ�����ݰ�����ʱ��
	int                 m_nCurrPacketCount;  //��λʱ���ڵİ���������

	_WorkThreadInfo()
	{
		m_nThreadID        = 0;
		m_nUpdateTime      = 0;
		m_nCreateTime      = 0;
		m_nState           = 0;
		m_nRecvPacketCount = 0;
		m_nSendPacketCount = 0;
		m_nCommandID       = 0;
		m_nPacketTime      = 0;
		m_nCurrPacketCount = 0;
	}
};

typedef vector<_WorkThreadInfo> vecWorkThreadInfo;

//��ط�������Ҫ���ص���Ϣ
struct _CheckServerInfo
{
public:
	CTcpClientConnect m_objTcpClientConnect;
	int               m_nCheckThreadCount;
	_WorkThreadInfo*  m_pWorkThreadInfo;

	_CheckServerInfo()
	{
		m_nCheckThreadCount = 0;
		m_pWorkThreadInfo   = NULL;
	}

	~_CheckServerInfo()
	{
		if(NULL != m_pWorkThreadInfo)
		{
			delete[] m_pWorkThreadInfo;
			m_pWorkThreadInfo = NULL;
		}
	}
};

typedef vector<_CheckServerInfo*> vecCheckServerInfo;

//XML�м��صķ�������������Ϣ
struct _CheckServer
{
public:
	char m_szIP[50]; 
	int  m_nPort;
	char m_szMagicCode[300];
	int  m_nThreadCount;

	_CheckServer()
	{
		m_szIP[0]        = '\0';
		m_nPort          = 0;
		m_szMagicCode[0] = '\0';
		m_nThreadCount   = 0;
	}
};

typedef vector<_CheckServer> vecCheckServer;

class CDlgClientWorkThread : public CDialog
{
	DECLARE_DYNAMIC(CDlgClientWorkThread)

public:
	CDlgClientWorkThread(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgClientWorkThread();

	// Dialog Data
	enum { IDD = IDD_DIALOG_WORKTHREAD };

public:
	CString GetPageTitle();
	void SetTcpClientConnect(CTcpClientConnect* pTcpClientConnect);
	void CheckWorkThread(_CheckServerInfo* pCheckServerInfo);
	void BeepAlarm();
	void ShowGroupServerList();
	void ClearCheckServerInfo();

private:
	CTcpClientConnect* m_pTcpClientConnect;
	bool               m_blTimeRun;

	//��Ⱥ��ز���
	vecCheckServerInfo m_vecCheckServerInfo;
	vecCheckServer     m_vecCheckServer;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_lcWorkThreadInfo;
	CEdit m_txtThreadCount;
	CEdit m_txtCheckTime;
	CListBox m_lbCheckLog;
	CButton m_btnVoice;
	CListBox m_lbServerList;

	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
};
