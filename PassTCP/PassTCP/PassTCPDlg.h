
// PassTCPDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "ClientTcpSocket.h"
#include "ClientUdpSocket.h"
#include "afxwin.h"
#include <afxpriv.h>
#include <vector>

using namespace std;

typedef vector<CClientTcpSocket* > vecClientTcpSocket;
typedef vector<CClientUdpSocket* > vecClientUdpSocket;

#define COLOR_TEXT_BULE 1
#define COLOR_TEXT_RED  2

// CPassTCPDlg �Ի���
class CPassTCPDlg : public CDialog
{
// ����
public:
	CPassTCPDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PASSTCP_DIALOG };

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

private:
  void InitView();
  void ClearResult();
  void Close();

  void SetRichTextColor(int nColor);

private:
  CTime m_tmBegin;             //ѹ�⿪ʼʱ��
  bool  m_blIsRun;             //ѹ���Ƿ��ڽ���
  CTime m_tmEnd;               //ѹ��ֹͣʱ��
  int   m_nCurrTextStyle;      //��ǰ�ı����  

public:
  CEdit m_txtServerIP;
  CEdit m_txtPort;
  CEdit m_txtThreadCount;
  CEdit m_txtRecvTimeout;
  CEdit m_txtSocketInterval;
  CEdit m_txtRecvLength;
  CButton m_chkIsAlwayConnect;
  CButton m_chkRadomaDelay;
  CButton m_chkIsRecv;
  CButton m_ChkIsBroken;
  CButton m_chkRadomSendCount;
  CButton m_chkIsWriteLog;
  CButton m_chkSendOne;
  CEdit m_txtSuccessConnect;
  CEdit m_txtSuccessSend;
  CEdit m_txtSuccessRecv;
  CEdit m_txtCurrConnect;
  CEdit m_txtFailConnect;
  CEdit m_txtFailSend;
  CEdit m_txtFailRecv;
  CEdit m_txtClientUdpPort;
  CEdit m_txtPacketTimewait;
  CEdit m_txtSendByteCount;
  CEdit m_txtRecvByteCount;
  CComboBox m_cbSendBuffStyle;
  CRichEditCtrl m_reSendText;
  //CEdit m_reSendText;
  int m_nRadio;
  
  vecClientTcpSocket m_vecClientTcpSocket;
  vecClientUdpSocket m_vecClientUdpSocket;
  afx_msg void OnClose();
  afx_msg void OnBnClickedButton2();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnBnClickedButton3();
  afx_msg void OnCbnSelchangeCombo1();
  afx_msg void OnBnClickedButton1();
  
};
