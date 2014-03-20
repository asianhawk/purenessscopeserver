#ifndef _DEFINE_H
#define _DEFINE_H

#include <WinSock2.h>
#include <time.h>
#include <string>

using namespace std;

#define MAX_BUFF_20   20
#define MAX_BUFF_100  100
#define MAX_BUFF_1024 1024

//����һ���������
inline void InitRandom()
{
	srand((int)time(NULL));
};

//��һ��ֵ���л�ȡһ�����ֵ
inline int RandomValue(int nMin, int nMax)
{
	return  nMin + (int) ((nMax - nMin) * (rand() / (RAND_MAX + 1.0)));
};

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

enum EM_DATA_RETURN_STATE
{
	DATA_RETURN_STATE_SUCCESS = 0,
	DATA_RETURN_STATE_ERROR,
	DATA_RETURN_STATE_CONTINUE,
};

//����һ�����࣬�������ʽ�����ݷ��ͺͽ��յĲ����߼�
class CBaseDataLogic
{
public:
	virtual bool InitSendSize(int nSendLen)                                 = 0;
	virtual char* GetSendData()                                             = 0;
	virtual char* GetSendData(int nThreadID, int nCurrIndex)                = 0;
	virtual int GetSendLength()                                             = 0;                   
	virtual int GetRecvLength()                                             = 0;
	virtual void SetRecvLength(int nRecvLen)                                = 0;
	virtual void SetMaxSendLength(int nMaxLength)                           = 0;
	virtual EM_DATA_RETURN_STATE GetRecvData(int nThreadID, int nCurrIndex, char* pData, int nLen) = 0;
};

//��ͨ��TCP�շ������Ϣ�����߼�
//Ϊ�˼���һЩ����Э�飬����websocket�Լ�http
//�շ����ݲ����ǵ�����һ��ָ�룬����һ���̳�CBaseDataLogic����
class CNomalLogic : public CBaseDataLogic
{
public:
	CNomalLogic() 
	{ 
		m_pSendData    = NULL;
		m_nSendLen     = 0;
		m_nRecvLen     = 0;
		m_nCurrRecvLen = 0;
	};

	~CNomalLogic() { Close(); };

	void Close()
	{
		if(NULL != m_pSendData)
		{
			delete[] m_pSendData;
			m_pSendData = NULL;
		}
	}

	bool InitSendSize(int nSendLen)
	{
		Close();

		m_pSendData = new char[nSendLen];
		m_nSendLen  = nSendLen;

		return true;
	}

	void SetRecvLength(int nRecvLen)
	{
		m_nRecvLen = nRecvLen;
	}

	void SetMaxSendLength(int nMaxLength)
	{
		m_nSendLen = nMaxLength;
	}

	//������ط���Buff
	void SetSendBuff(const char* pData, int nLen)
	{
		memcpy_s(m_pSendData, nLen, pData, nLen);
		m_nSendLen = nLen;
	}

	char* GetSendData()
	{
		return m_pSendData;
	}

	char* GetSendData(int nThreadID, int nCurrIndex)
	{
		return m_pSendData;
	}

	int GetSendLength()
	{
		return m_nSendLen;
	}

	int GetRecvLength()
	{
		return m_nRecvLen;
	}

	EM_DATA_RETURN_STATE GetRecvData(int nThreadID, int nCurrIndex, char* pData, int nLen)
	{
		m_nCurrRecvLen += nLen;
		if(m_nCurrRecvLen == m_nRecvLen)
		{
			m_nCurrRecvLen = 0;
			//ȫ��������ϣ�������ȷ
			return DATA_RETURN_STATE_SUCCESS;
		}
		else if(nLen < m_nRecvLen)
		{
			//û�н�����ȫ����������
			return DATA_RETURN_STATE_CONTINUE;
		}

		m_nCurrRecvLen = 0;
		return DATA_RETURN_STATE_ERROR;
	}

private:
	char* m_pSendData;
	int   m_nSendLen;
	int   m_nRecvLen;
	int   m_nCurrRecvLen;
};

//�߳�������Ϣ
class _Socket_Info
{
public:
	char  m_szSerevrIP[MAX_BUFF_20];      //Զ�̷�������IP
	int   m_nPort;                        //Զ�̷������Ķ˿�
	int   m_nThreadID;                    //�߳�ID
	int   m_nRecvTimeout;                 //�������ݳ�ʱʱ�䣨��λ�Ǻ��룩
	int   m_nDelaySecond;                 //�����Ӽ���ʱ����λ�Ǻ��룩
	int   m_nPacketTimewait;              //���ݰ����ͼ��(��λ�Ǻ���)
	//int   m_nSendLength;                //�����ַ�������
	//int   m_nRecvLength;                //�����ַ��������޶�
	//char* m_pSendBuff;                  //�������ݳ���
	bool  m_blIsAlwayConnect;             //�Ƿ�����
	bool  m_blIsRadomaDelay;              //�Ƿ������ʱ
	bool  m_blIsRecv;                     //�Ƿ���ջ�Ӧ��
	bool  m_blIsBroken;                   //�Ƿ��������
	bool  m_blIsSendCount;                //�Ƿ�������ݰ���
	bool  m_blIsWriteFile;                //�Ƿ�д���ļ�
	bool  m_blIsSendOne;                  //�Ƿ�ֻ��һ��
	bool  m_blLuaAdvance;                 //�Ƿ�����Lua�߼�ģʽ 
	int   m_nConnectType;                 //�������ͣ�0��TCP��1��UDP
	int   m_nUdpClientPort;               //UDP�ͻ��˽������ݶ˿�
	int   m_nSendCount;                   //���������ݰ���
	char  m_szLuaFileName[MAX_BUFF_1024]; //�߼�ģʽ��Lua�ļ���
	CBaseDataLogic* m_pLogic;             //���ݶ���  

	_Socket_Info()
	{
		m_szSerevrIP[0]    = '\0';
		m_nPort            = 0;
		m_nThreadID        = 0;
		m_nRecvTimeout     = 0;
		m_nPacketTimewait  = 0;
		m_nDelaySecond     = 0;
		//m_nSendLength      = 0;
		//m_nRecvLength      = 0;
		m_nSendCount       = 0;
		//m_pSendBuff        = NULL;
		m_blIsAlwayConnect = false;
		m_blIsRadomaDelay  = false;
		m_blIsRecv         = true;
		m_blIsBroken       = true;
		m_blIsSendCount    = false;
		m_blIsWriteFile    = false;
		m_blIsSendOne      = false;
		m_blLuaAdvance     = false;
		m_nConnectType     = 0;
		m_nUdpClientPort   = 0;
		m_szLuaFileName[0] = '\0';
		m_pLogic           = NULL;
	}

	~_Socket_Info()
	{
		if(m_pLogic != NULL)
		{
			delete m_pLogic; 
		}
	}

};

//�߳�����״̬��Ϣ
struct _Socket_State_Info
{
	int m_nSuccessConnect;            //���ӳɹ���
	int m_nSuccessSend;               //���ͳɹ���
	int m_nSuccessRecv;               //���ճɹ���
	int m_nCurrectSocket;             //��ǰ������
	int m_nFailConnect;               //����ʧ����
	int m_nFailSend;                  //����ʧ����
	int m_nFailRecv;                  //����ʧ����
	int m_nSendByteCount;             //�����ֽ���
	int m_nRecvByteCount;             //�����ֽ���
	int m_nMinRecvTime;               //��С����ʱ��
	int m_nMaxRecvTime;               //������ʱ�� 

	_Socket_State_Info()
	{
		m_nSuccessConnect = 0;
		m_nSuccessSend    = 0;
		m_nSuccessRecv    = 0;
		m_nCurrectSocket  = 0;
		m_nFailConnect    = 0;
		m_nFailSend       = 0;
		m_nFailRecv       = 0;
		m_nSendByteCount  = 0;
		m_nRecvByteCount  = 0;
		m_nMinRecvTime    = 0;
		m_nMaxRecvTime    = 0;
	}
};


#endif
