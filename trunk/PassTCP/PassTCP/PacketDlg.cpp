// CPacketDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PassTCP.h"
#include "PacketDlg.h"
#include "tinyxml.h"

// CPacketDlg �Ի���

IMPLEMENT_DYNAMIC(CPacketDlg, CDialog)

CPacketDlg::CPacketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPacketDlg::IDD, pParent)
{

}

CPacketDlg::~CPacketDlg()
{
}

void CPacketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_txtPacketData);
	DDX_Control(pDX, IDC_LIST1, m_lbRondomPacketList);
	DDX_Control(pDX, IDC_LIST2, m_lbPreviewList);
	DDX_Control(pDX, IDC_EDIT2, m_txtRecvLength);
	DDX_Control(pDX, IDC_RADIO1, m_RadioBuffer);
}


BEGIN_MESSAGE_MAP(CPacketDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CPacketDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CPacketDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CPacketDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CPacketDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON2, &CPacketDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON6, &CPacketDlg::OnBnClickedButton6)
END_MESSAGE_MAP()

void CPacketDlg::SetBaseDataLogic(CBaseDataLogic* pBaseDataLogic)
{
	m_pBaseDataLogic = pBaseDataLogic;
	m_pBaseDataLogic->ClearRandomPacket();
	m_pBaseDataLogic->InitSendSize(MAX_RANDOM_PACKET);
}

// CPacketDlg ��Ϣ�������

void CPacketDlg::OnBnClickedButton1()
{
	//������ݰ�
	CString strData;
	char szPacketData[MAX_RANDOM_PACKET] = {'\0'};

	if(m_pBaseDataLogic == NULL)
	{
		MessageBox(_T("����ָ��m_pBaseDataLogicָ�룬ָ�벻��Ϊ�ա�"));
		return;
	}

	m_txtPacketData.GetWindowText(strData);
	int nSrcLen = WideCharToMultiByte(CP_ACP, 0, strData, strData.GetLength(), NULL, 0, NULL, NULL);
	int nDecLen = WideCharToMultiByte(CP_ACP, 0, strData, nSrcLen, szPacketData, MAX_RANDOM_PACKET, NULL,NULL);
	szPacketData[nDecLen] = '\0';

	//��ý����ֽ���
	m_txtRecvLength.GetWindowText(strData);
	int nRecvLength = _ttoi(strData);

	if(nRecvLength <= 0)
	{
		MessageBox(_T("����ָ���������ݰ��ĳ��ȣ������޷����͡�"));
		return;
	}

	if(((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck() == 1)
	{
		//������ģʽ2
		m_pBaseDataLogic->InsertRandomPacket(szPacketData, nDecLen, nRecvLength, 2);
	}
	else
	{
		//�ı�ģʽ1
		string strFormatData = (string)szPacketData;

		//�滻\r\n
		string_replace(strFormatData, "\\r", "\r");
		string_replace(strFormatData, "\\n", "\n");

		m_pBaseDataLogic->InsertRandomPacket(strFormatData.c_str(), strFormatData.length(), nRecvLength, 1);
	}

	ShowPacketList();
}

void CPacketDlg::ShowPacketList()
{
	//��ʾ���е����ݰ�
	m_lbRondomPacketList.ResetContent();

	if(m_pBaseDataLogic == NULL)
	{
		return;
	}

	for(int i = 0; i < m_pBaseDataLogic->GetRandomPacketCount(); i++)
	{
		_RandomPacketInfo* pRandomPacketInfo = m_pBaseDataLogic->GettRandomPacket(i);
		if(NULL != pRandomPacketInfo)
		{
			wchar_t szTemp[MAX_RANDOM_PACKET] = {'\0'};

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, pRandomPacketInfo->szPacket, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, pRandomPacketInfo->szPacket, -1, szTemp, MAX_RANDOM_PACKET);
			szTemp[nDecLen] = '\0';

			m_lbRondomPacketList.AddString(szTemp);
		}
	}
}

void CPacketDlg::OnBnClickedButton3()
{
	//Ԥ�����ݰ�
	m_lbPreviewList.ResetContent();
	int nRecvLength = 0;

	char szTemp[MAX_RANDOM_PACKET] = {'\0'};
	for(int i = 0; i < m_pBaseDataLogic->GetRandomPacketCount(); i++)
	{
		int nLen = MAX_RANDOM_PACKET;
		m_pBaseDataLogic->GetReview(i, szTemp, nLen, nRecvLength);
		if(0 < nLen)
		{
			wchar_t sszTemp[MAX_RANDOM_PACKET] = {'\0'};

			int nSrcLen = MultiByteToWideChar(CP_ACP, 0, szTemp, -1, NULL, 0);
			int nDecLen = MultiByteToWideChar(CP_ACP, 0, szTemp, -1, sszTemp, MAX_RANDOM_PACKET);
			sszTemp[nDecLen] = '\0';

			m_lbPreviewList.AddString(sszTemp);
		}
	}
}

void CPacketDlg::OnBnClickedButton4()
{
	//ɾ��ѡ�е���Ŀ
	int nIndex = m_lbRondomPacketList.GetCurSel();
	if(nIndex >= 0)
	{
		m_pBaseDataLogic->DeleteRandomPacket(nIndex);
	}
}

BOOL CPacketDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_RadioBuffer.SetCheck(BST_CHECKED);
	m_txtRecvLength.SetWindowText(_T("0"));

	return TRUE;
}
void CPacketDlg::OnBnClickedButton5()
{
	//�رյ�ǰ������
	OnOK();
}

void CPacketDlg::OnBnClickedButton2()
{
	//�����XML�ļ�
	FILE* pFile = NULL;

	char szFileName[20];
	sprintf_s(szFileName, PACKETDATA_FILENAME);
	fopen_s(&pFile, szFileName, "w");

	if(pFile == NULL)
	{
		MessageBox(_T("����˳�����ݰ������ļ�������"), _T("��ʾ��Ϣ"), MB_OK);
		return;
	}

	char szLogText[MAX_RANDOM_PACKET] = {'\0'};

	sprintf_s(szLogText, MAX_RANDOM_PACKET, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fwrite(szLogText, strlen(szLogText), sizeof(char), pFile);

	sprintf_s(szLogText, MAX_RANDOM_PACKET, "<config>\n");
	fwrite(szLogText, strlen(szLogText), sizeof(char), pFile);

	for(int i = 0; i < m_pBaseDataLogic->GetRandomPacketCount(); i++)
	{
		_RandomPacketInfo* pRandomPacketInfo = m_pBaseDataLogic->GettRandomPacket(i);
		sprintf_s(szLogText, MAX_RANDOM_PACKET, "<PacketData Len=\"%d\" Type=\"%d\" RecdvLength=\"%d\"><![CDATA[%s]]></PacketData>\n"
			,pRandomPacketInfo->nLen 
			,pRandomPacketInfo->nType
			,pRandomPacketInfo->nRecdvLength
			,pRandomPacketInfo->szPacket);

		fwrite(szLogText, strlen(szLogText), sizeof(char), pFile);
	}

	sprintf_s(szLogText, MAX_RANDOM_PACKET, "</config>\n");
	fwrite(szLogText, strlen(szLogText), sizeof(char), pFile);

	fclose(pFile);

	MessageBox(_T("����˳�����ݰ��ɹ�"), _T("��ʾ��Ϣ"), MB_OK);
}

void CPacketDlg::OnBnClickedButton6()
{
	m_pBaseDataLogic->ClearRandomPacket();

	//��ȡXML�ļ�
	TiXmlDocument * pDocument = new TiXmlDocument(PACKETDATA_FILENAME);//tstl����Ϊ�ļ�·�����ļ���
	pDocument->LoadFile();

	if(NULL == pDocument)
	{
		MessageBox(_T("��ȡ˳�����ݰ�xml�ļ�ʧ��"), _T("��ʾ��Ϣ"), MB_OK);
		return;
	}

	TiXmlElement *Root = pDocument->RootElement();//��ȡ���ڵ�<Particls>
	TiXmlElement *Particl = NULL;

	int nLen        = 0;
	int nType       = 0;
	int nRecvLength = 0;

	for(Particl = Root->FirstChildElement();Particl != NULL;Particl = Particl->NextSiblingElement())
	{
		//�õ��ĵ�����
		const char *sztext = Particl->GetText();

		//�õ��ĵ�����
		TiXmlAttribute* pAddrAttr = Particl->FirstAttribute();
		if(strcmp(pAddrAttr->Name(), "Len") == 0)
		{
			nLen = atoi(pAddrAttr->Value());
		}
		else if(strcmp(pAddrAttr->Name(), "Type") == 0)
		{
			nType = atoi(pAddrAttr->Value());
		}
		else
		{
			nRecvLength = atoi(pAddrAttr->Value());
		}

		TiXmlAttribute *pAttr1 = pAddrAttr->Next();
		if(strcmp(pAttr1->Name(), "Len") == 0)
		{
			nLen = atoi(pAttr1->Value());
		}
		else if(strcmp(pAttr1->Name(), "Type") == 0)
		{
			nType = atoi(pAttr1->Value());
		}
		else
		{
			nRecvLength = atoi(pAttr1->Value());
		}

		TiXmlAttribute * pAttr2 = pAttr1->Next();
		if(strcmp(pAttr2->Name(), "Len") == 0)
		{
			nLen = atoi(pAttr2->Value());
		}
		else if(strcmp(pAttr2->Name(), "Type") == 0)
		{
			nType = atoi(pAttr2->Value());
		}
		else
		{
			nRecvLength = atoi(pAttr2->Value());
		}

		//�������
		m_pBaseDataLogic->InsertRandomPacket(sztext, nLen, nRecvLength, nType);
	}

	ShowPacketList();

	delete pDocument;
}
