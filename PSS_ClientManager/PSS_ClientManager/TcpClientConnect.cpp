#include "StdAfx.h"
#include "TcpClientConnect.h"

CTcpClientConnect::CTcpClientConnect(void)
{
}

CTcpClientConnect::~CTcpClientConnect(void)
{
}

void CTcpClientConnect::Init( const char* pIp, int nPort, const char* pKey )
{
	sprintf_s(m_szServerIP, 20, "%s", pIp);
	sprintf_s(m_szConsoleKey, 50, "%s", pKey);
	m_nServerPort = nPort;
}

bool CTcpClientConnect::SendConsoleMessage( const char* pMessage, int nMessageLen, char* pRevBuff, int& nRecvLen )
{
	SOCKET sckClient;
	char szBodyBuff[10*1024] = {'\0'};
	int nPacketLen = 0;

	//socket������׼������
	struct sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(m_nServerPort);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(m_szServerIP);

	sckClient = socket(AF_INET, SOCK_STREAM, 0);

	//����Զ�̷�����
	int nErr = connect(sckClient, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));
	if(0 != nErr)
	{
		closesocket(sckClient);
		return false;
	}

	int nTotalRecvLen = nRecvLen;
	int nTotalSendLen = nMessageLen;
	int nBeginSend    = 0;
	int nCurrSendLen  = 0;
	int nFinishRecv   = 0;
	bool blSendFlag   = false;

	while(true)
	{
		nCurrSendLen = send(sckClient, pMessage + nBeginSend, nTotalSendLen, 0);
		if(nCurrSendLen <= 0)
		{
			closesocket(sckClient);
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

	int nCurrRecvLen = 0;
	int nBodyLen     = 0;

	//��������
	if(blSendFlag == true)
	{
		//���Ƚ������ֽ�����ͷ
		char szHeadData[6] = {'\0'};
		int nHeadLen = sizeof(int) + sizeof(short);

		while(true)
		{
			int nLen = recv(sckClient, (char* )&szHeadData[nCurrRecvLen], nHeadLen - nCurrRecvLen, 0);
			if(nLen <= 0)
			{
				closesocket(sckClient);
				return false;
			}
			else
			{
				if(nLen + nCurrRecvLen >= nHeadLen)
				{
					break;
				}
				else
				{
					//����δ��ɣ���������
					nCurrRecvLen += nLen;
				}
			}
		}

		//������ݰ�����
		memcpy_s(&nBodyLen, sizeof(int), szHeadData, sizeof(int));
		if(nBodyLen <= 0 || nBodyLen >= 1024*1024)
		{
			closesocket(sckClient);
			return false;
		}

		//ȥ������ͷ2�ֽڵ� CommandID
		nBodyLen = nBodyLen - 2;

		//��ʼ����������
		nCurrRecvLen = 0;
		while(true)
		{
			int nLen = recv(sckClient, (char* )&szBodyBuff[nCurrRecvLen], nBodyLen - nCurrRecvLen, 0);
			if(nLen <= 0)
			{
				closesocket(sckClient);
				return false;
			}
			else
			{
				if(nLen + nCurrRecvLen >= nBodyLen)
				{
					break;
				}
				else
				{
					//����δ��ɣ���������
					nCurrRecvLen += nLen;
				}
			}
		}

	}

	memcpy_s(pRevBuff, nBodyLen, szBodyBuff, nBodyLen);
	nRecvLen = nBodyLen;

	closesocket(sckClient);
	return true;
}

char* CTcpClientConnect::GetKey()
{
	return m_szConsoleKey;
}

char* CTcpClientConnect::GetServerIP()
{
	return m_szServerIP;
}

int CTcpClientConnect::GetServerPort()
{
	return m_nServerPort;
}

char* CTcpClientConnect::GetServerKey()
{
	return m_szConsoleKey;
}
