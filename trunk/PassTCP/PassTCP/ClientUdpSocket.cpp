#include "StdAfx.h"
#include "ClientUdpSocket.h"

CClientUdpSocket::CClientUdpSocket(void)
{
	m_pSocket_Info       = NULL;
	m_pSocket_State_Info = NULL;
	m_blRun              = false;
}

CClientUdpSocket::~CClientUdpSocket(void)
{
  Close();
}

void CClientUdpSocket::Close()
{
	if(NULL != m_pSocket_Info)
	{
		delete m_pSocket_Info;
		m_pSocket_Info = NULL;
	}

	if(NULL != m_pSocket_State_Info)
	{
		delete m_pSocket_State_Info;
		m_pSocket_State_Info = NULL;
	}
}

void CClientUdpSocket::Run()
{
	int nPacketCount = 1;
	m_blRun = true;
	SOCKET sckClient;
	SOCKET sckServer;

	if(m_pSocket_Info == NULL || m_pSocket_State_Info == NULL)
	{
		m_blRun = false;
		return;
	}

	//看看是否是长连接，如果是长连接，则只处理一次。
	bool blIsConnect = false;

	//socket创建的准备工作
	struct sockaddr_in svrsockaddr;

	memset(&svrsockaddr, 0, sizeof(SOCKADDR_IN));
	svrsockaddr.sin_family = AF_INET;
	svrsockaddr.sin_port   = htons(m_pSocket_Info->m_nPort);
	svrsockaddr.sin_addr.S_un.S_addr = inet_addr(m_pSocket_Info->m_szSerevrIP);

	sckClient = socket(AF_INET, SOCK_DGRAM, 0);

	DWORD TimeOut = (DWORD)m_pSocket_Info->m_nRecvTimeout;
	::setsockopt(sckClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	//设置接收监听端口
	sckServer = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in clientsockaddr;
	memset(&clientsockaddr, 0, sizeof(SOCKADDR_IN));
	clientsockaddr.sin_family = AF_INET;
	clientsockaddr.sin_port   = htons(m_pSocket_Info->m_nUdpClientPort);
	clientsockaddr.sin_addr.S_un.S_addr = inet_addr(m_pSocket_Info->m_szSerevrIP);

	bind(sckServer, (SOCKADDR *) &clientsockaddr, sizeof(clientsockaddr));

	while(m_blRun)
	{
		if(blIsConnect == false)
		{
			DWORD dwSleepTime = (DWORD)m_pSocket_Info->m_nDelaySecond;
			if(m_pSocket_Info->m_blIsRadomaDelay == true)
			{
				//如果是随机的，则从1-1000之间随机一个时间
				dwSleepTime = (DWORD)RandomValue(1, 1000);
			}

			//挂起指定的时间
			Sleep(dwSleepTime);

			m_pSocket_State_Info->m_nSuccessConnect++;
			m_pSocket_State_Info->m_nCurrectSocket = 1;
			blIsConnect = true;
		}

		if(blIsConnect == true)
		{
			//发送数据
			char szSendBuffData[MAX_BUFF_1024 * 100] = {'\0'};
			char szRecvBuffData[MAX_BUFF_1024 * 100] = {'\0'};

			char* pSendData   = NULL;
			int nSendLen      = 0;
			int nTotalRecvLen = 0;
			//如果数据为随机数据包
			if(m_pSocket_Info->m_blIsSendCount == true)
			{
				int nSendCount = RandomValue(1, 10);
				for(int i = 0; i < nSendCount; i++)
				{
					memcpy(&szSendBuffData[i * m_pSocket_Info->m_nSendLength], m_pSocket_Info->m_pSendBuff, m_pSocket_Info->m_nSendLength);
				}
				nPacketCount = nSendCount;

				//发送数据
				pSendData     = (char* )szSendBuffData;
				nSendLen      = m_pSocket_Info->m_nSendLength * nSendCount;
				nTotalRecvLen = m_pSocket_Info->m_nRecvLength * nSendCount;
			}
			else
			{
				//发送数据
				pSendData     = (char* )m_pSocket_Info->m_pSendBuff;
				nSendLen      = m_pSocket_Info->m_nSendLength;
				nTotalRecvLen = m_pSocket_Info->m_nRecvLength;
			}

			//记录应收字节总数
			int nRecvAllSize = nTotalRecvLen;

			//如果需要记录日志，则将数据计入日志
			if(m_pSocket_Info->m_blIsWriteFile == true)
			{
				WriteFile_SendBuff(pSendData, nSendLen);
			}

			int nTotalSendLen = nSendLen;
			int nBeginSend    = 0;
			int nCurrSendLen  = 0;
			bool blSendFlag   = false;
			int nBeginRecv    = 0;
			int nCurrRecvLen  = 0;
			bool blRecvFlag   = false;
			while(true)
			{
				//UDP发送
				int nssssSize = sizeof(SOCKADDR);
				nCurrSendLen = sendto(sckClient, pSendData + nBeginSend, nTotalSendLen, 0, (sockaddr* )&svrsockaddr, sizeof(SOCKADDR));
				if(nCurrSendLen <= 0)
				{
					DWORD dwError = GetLastError();
					WriteFile_Error("sendto error", (int)dwError);
					m_pSocket_State_Info->m_nFailSend += nPacketCount;
					m_pSocket_State_Info->m_nCurrectSocket = 0;
					blIsConnect = false;
					break;
				}
				else
				{
					nTotalSendLen -= nCurrSendLen;
					if(nTotalSendLen == 0)
					{
						//发送完成
						m_pSocket_State_Info->m_nSuccessSend += nPacketCount;
						blSendFlag = true;
						break;
					}
					else
					{
						nBeginSend += nCurrSendLen;
					}
				}
			}

			//接收数据
			if(blSendFlag == true && m_pSocket_Info->m_blIsRecv == true)
			{
				while(true)
				{
					//如果发送成功了，则处理接收数据
					int nClientAddrSize = sizeof(SOCKADDR_IN);

					//socket创建的准备工作
					struct sockaddr_in clientsockaddr1;
					/*
					struct sockaddr_in clientsockaddr;

					memset(&clientsockaddr, 0, sizeof(SOCKADDR_IN));
					clientsockaddr.sin_family = AF_INET;
					clientsockaddr.sin_port   = htons(20004);
					clientsockaddr.sin_addr.S_un.S_addr = inet_addr(m_pSocket_Info->m_szSerevrIP);
					*/

					nCurrRecvLen = recvfrom(sckServer, (char* )szRecvBuffData + nBeginRecv, nTotalRecvLen, 0, (sockaddr* )&clientsockaddr1, &nClientAddrSize);
					if(nCurrRecvLen <= 0)
					{
						DWORD dwError = GetLastError();
						WriteFile_Error("sendto error", (int)dwError);
						m_pSocket_State_Info->m_nFailRecv += nPacketCount;
						//closesocket(sckClient);
						m_pSocket_State_Info->m_nCurrectSocket = 0;
						blIsConnect = false;
						break;
					}
					else
					{
						nTotalRecvLen -= nCurrRecvLen;
						if(nTotalRecvLen == 0)
						{
							//接收完成
							m_pSocket_State_Info->m_nSuccessRecv += nPacketCount;
							blRecvFlag = true;

							//如果需要记录日志，则将数据计入日志
							if(m_pSocket_Info->m_blIsWriteFile == true)
							{
								WriteFile_RecvBuff(szRecvBuffData, nRecvAllSize);
							}

							break;
						}
						else
						{
							nBeginRecv += nCurrRecvLen;
						}
					}
				}
			}

			//如果是长连接，则不关闭连接
			if(m_pSocket_Info->m_blIsAlwayConnect == false)
			{
				//closesocket(sckClient);
				m_pSocket_State_Info->m_nCurrectSocket = 0;
				blIsConnect = false;
			}
		}

		//如果只发送一次，在这里退出
		if(m_pSocket_Info->m_blIsSendOne == true)
		{
			m_blRun = false;
		}
	}

	//如果连接没断，则断开
	closesocket(sckClient);
	closesocket(sckServer);
	m_pSocket_State_Info->m_nCurrectSocket = 0;
	blIsConnect = false;
}

void CClientUdpSocket::Stop()
{
	m_blRun = false;
}

void CClientUdpSocket::SetSocketThread( _Socket_Info* pSocket_Info, _Socket_State_Info* pSocket_State_Info )
{
	Close();

	m_pSocket_Info       = pSocket_Info;
	m_pSocket_State_Info = pSocket_State_Info;
}

_Socket_State_Info* CClientUdpSocket::GetStateInfo()
{
	return m_pSocket_State_Info;
}

bool CClientUdpSocket::WriteFile_SendBuff( const char* pData, int nLen )
{
	FILE* pFile = NULL;
	char szFileName[20];
	sprintf_s(szFileName, "Thread%d.log", m_pSocket_Info->m_nThreadID);
	fopen_s(&pFile, szFileName, "a+");
	if(pFile == NULL)
	{
		return false;
	}

	string strLog;
	strLog = "[SendBuff]";

	for(int i = 0; i < nLen; i++)
	{
		char szChar[20];
		sprintf_s(szChar, 20, " 0x%02X", (unsigned char )pData[i]);
		strLog += szChar;
	}

	strLog += "\n";

	fwrite(strLog.c_str(), strLog.length(), sizeof(char), pFile);

	fclose(pFile);
	return true;
}

bool CClientUdpSocket::WriteFile_RecvBuff( const char* pData, int nLen )
{
	FILE* pFile = NULL;
	char szFileName[20];
	sprintf_s(szFileName, "Thread%d.log", m_pSocket_Info->m_nThreadID);
	fopen_s(&pFile, szFileName, "a+");
	if(pFile == NULL)
	{
		return false;
	}

	string strLog;
	strLog = "[RecvBuff]";

	for(int i = 0; i < nLen; i++)
	{
		char szChar[20];
		sprintf_s(szChar, 20, " 0x%02X", (unsigned char )pData[i]);
		strLog += szChar;
	}

	strLog += "\n";

	fwrite(strLog.c_str(), strLog.length(), sizeof(char), pFile);

	fclose(pFile);
	return true;
}

bool CClientUdpSocket::WriteFile_Error( const char* pError, int nErrorNumber )
{
	time_t ttNow    = time(NULL);
	struct tm tmNow;
	localtime_s(&tmNow, &ttNow);

	char szTimeNow[30] = {'\0'};
	sprintf_s(szTimeNow, 30, "[%04d-%02d-%02d %02d:%02d:%02d]", tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

	//拼接出错日志输出
	char szError[1024] = {'\0'};
	sprintf_s(szError, 1024, "%s %s, errno=%d.\n", szTimeNow, pError, nErrorNumber);

	FILE* pFile = NULL;
	char szFileName[30];
	sprintf_s(szFileName, "StressTest_Error.log");
	fopen_s(&pFile, szFileName, "a+");
	if(pFile == NULL)
	{
		return false;
	}

	fwrite(szError, strlen(szError), sizeof(char), pFile);

	fclose(pFile);
	return true;
}