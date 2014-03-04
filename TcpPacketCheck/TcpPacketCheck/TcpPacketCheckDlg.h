// TcpPacketCheckDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define MAX_BUFF_200   200

#define COMMAND_AUTOTEST_HEAD         0x5000   //��ͷ�������ݰ�
#define COMMAND_AUTOTEST_NOHEAD       0x5001   //����ͷ���ݰ�
#define COMMAND_AUTOTEST_HEADBUFF     0x5002   //��ͷ�������ݰ�
#define COMMAND_AUTOTEST_NOHEADBUFF   0x5003   //����ͷ�������ݰ�
#define COMMAND_AUTOTEST_LOGDATA      0x5004   //���Լ�¼��־
#define COMMAND_AUTOTEST_WORKTIMEOUT  0x5005   //���Թ����̳߳�ʱ�ؽ�

#define COMMAND_AUTOTEST_RETUEN_HEAD         0xf000   
#define COMMAND_AUTOTEST_RETUEN_NOHEAD       0xf001
#define COMMAND_AUTOTEST_RETUEN_HEADBUFF     0xf002
#define COMMAND_AUTOTEST_RETUEN_NOHEADBUFF   0xf003
#define COMMAND_AUTOTEST_RETUEN_LOGDATA      0xf004
#define COMMAND_AUTOTEST_RETURN_WORKTIMEOUT  0xf005

class CConvertBuffer
{
public:
	CConvertBuffer() {};
	~CConvertBuffer() {};
	int GetBufferSize(const char* pData, int nSrcLen)
	{
		char szData[3] = {'\0'};
		int nPos         = 0;
		int nCurrSize    = 0;
		int nConvertSize = 0;
		bool blState     = false;   //ת������ַ����Ƿ���Ч
		bool blSrcState  = true;    //Ԫ�ַ����Ƿ���Ч
		unsigned char cData;

		while(nPos < nSrcLen)
		{
			if(pData[nPos] == '\r' || pData[nPos] == '\n' || pData[nPos] == ' ' || nPos == nSrcLen - 1)
			{
				if(nPos == nSrcLen - 1)
				{
					szData[nCurrSize++] = pData[nPos];
				}

				szData[nCurrSize] = '\0';
				if(blSrcState == true)
				{
					blState = ConvertStr2char(szData, cData);
					if(blState == true)
					{
						nConvertSize++;
					}
				}
				nCurrSize  = 0;
				blSrcState = true;
				nPos++;
			}
			else
			{
				if(nCurrSize < 2)
				{
					szData[nCurrSize++] = pData[nPos];
				}
				else
				{
					blSrcState = false;
				}
				nPos++;
			}
		}

		return nConvertSize;
	};

	bool Convertstr2charArray(const char* pData, int nSrcLen, unsigned char* pDes, int& nMaxLen)
	{
		char szData[3] = {'\0'};
		int nPos         = 0;
		int nCurrSize    = 0;
		int nConvertSize = 0;
		bool blState     = false;   //ת������ַ����Ƿ���Ч
		bool blSrcState  = true;    //Ԫ�ַ����Ƿ���Ч

		while(nPos < nSrcLen)
		{
			if(pData[nPos] == '\r' || pData[nPos] == '\n' || pData[nPos] == ' ' || nPos == nSrcLen - 1)
			{
				if(nPos == nSrcLen - 1)
				{
					szData[nCurrSize++] = pData[nPos];
				}

				szData[nCurrSize] = '\0';
				if(nConvertSize < nMaxLen && blSrcState == true)
				{
					blState = ConvertStr2char(szData, pDes[nConvertSize]);
					if(blState == true)
					{
						nConvertSize++;
					}
				}
				nCurrSize  = 0;
				blSrcState = true;
				nPos++;
			}
			else
			{
				if(nCurrSize < 2)
				{
					szData[nCurrSize++] = pData[nPos];
				}
				else
				{
					blSrcState = false;
				}
				nPos++;
			}
		}

		nMaxLen = nConvertSize;
		return true;
	};
private:
	bool Get_binary_Char(unsigned char cTag, unsigned char& cDes)
	{
		if(cTag >='A'&&  cTag <='F')
		{
			cDes = cTag - 'A' + 10;
			return true;
		}
		else if(cTag >='a'&&  cTag <='f')
		{
			cDes = cTag - 'a' + 10;
			return true; 
		}
		else if(cTag >= '0'&& cTag<= '9')
		{
			cDes = cTag-'0';
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ConvertStr2char(const char* pData, unsigned char& cData)
	{
		if(pData == NULL || strlen(pData) != 2)
		{
			return false;
		}

		char cFirst = pData[1];
		unsigned char cTemp = 0;
		bool blStste = Get_binary_Char(cFirst, cTemp);
		if(false == blStste)
		{
			return false;
		}
		cData = cTemp;
		char cSecond = pData[0];
		blStste  = Get_binary_Char(cSecond, cTemp);
		if(false == blStste)
		{
			return false;
		}
		cTemp = cTemp << 4;
		cData = cData | cTemp;

		return true;
	}
};

struct _ClientInfo
{
	char  m_szServerIP[30];
	int   m_nPort; 
	char* m_pSendBuffer;
	int   m_nSendLength;
	int   m_nRecvLength;

	_ClientInfo()
	{
		m_szServerIP[0] = '\0';
		m_nPort         = 0;
		m_pSendBuffer   = NULL;
		m_nSendLength   = 0;
		m_nRecvLength   = 0;
	}
};

// CTcpPacketCheckDlg �Ի���
class CTcpPacketCheckDlg : public CDialog
{
// ����
public:
	CTcpPacketCheckDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TCPPACKETCHECK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

public:
	void Run();

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
	bool CheckTcpPacket(_ClientInfo& objClientInfo, int nIndex);
	bool CheckMultipleTcpPacket(_ClientInfo& objClientInfo, int nIndex);
	bool CheckValidPacket(_ClientInfo& objClientInfo, int nIndex);
	bool CheckHalfPacket(_ClientInfo& objClientInfo, int nIndex);
	bool CheckIsHead(_ClientInfo& objClientInfo, int nIndex);
	bool CheckIsNoHead(_ClientInfo& objClientInfo, int nIndex);
	bool CheckIsHeadBuffer(_ClientInfo& objClientInfo, int nIndex);
	bool CheckIsNoHeadBuffer(_ClientInfo& objClientInfo, int nIndex);
	bool CheckLogFile(_ClientInfo& objClientInfo, int nIndex);
	bool CheckWorkTimeout(_ClientInfo& objClientInfo, int nIndex);

private:
	CEdit m_txtPacketBuffer;
	CEdit m_txtServerIP;
	CEdit m_txtPort;
	CEdit m_txtRecvLength;
	CListCtrl m_lstResult;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CButton m_btnRun;
};
