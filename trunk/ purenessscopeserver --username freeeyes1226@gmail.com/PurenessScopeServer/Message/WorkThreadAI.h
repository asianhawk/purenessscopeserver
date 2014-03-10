#ifndef _WORKTHREADAI_H
#define _WORKTHREADAI_H

//ʵ�ֶԹ����߳�Ч�ʵļ��
//���뷨��Դ��Bobo�Ľ��飬�����һ�������̳߳������������������ݻ�����ô�죿
//ɱ�������̺߳����ӹ����̶߳�������õİ취
//��õİ취�������ҵ�������׵��Ǹ����ݰ�
//��������£�Ӧ����ĳ�����ݴ������������⣬�����ܲ���ȫ�������߳�
//�������������ݰ���һ��ʱ���������˹����߳�̫��ʱ�䣬��ô�͹��ϲ�ȡ��ʩ
//ֹͣ�������һ��ʱ�䣬�����ظ��ͻ��˸�֪
//ֱ���߶̻�·��Ӧ�����쳣����֤�������ݰ����������ݴ���
//����¼��־������������ά������
//����������������Ϊĳһ��ָ����������µ������������ӦЧ�ʵ��µ�����
//�˹��ܣ����Ը�����������Ƿ�������main.xml�������ÿ���
//����������ݰ������ڻ������ڴ�AI�Ĺ���Χ֮��
//PSS���ɻ����������ұ������ƣ����������߳�
//������Ȼȫ�����ݰ�����ʱ�ˣ�Ӧ��Ҳ���������û�������
//�˹���ֻ������޶ȵı����û����飬�û������Ƿ���������ʳ��ĸ
//add by freeeyes

#include "Ring.h"
#include "define.h"

#define COMMAND_RETURN_BUSY 0xffff

//�����ƻ�ש�࣬��������ƵĴ���ת��
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

//��ز������
struct _WorkThreadAIInfo
{
	uint32     m_u4ThreadID;                       //�����߳�ID 
	uint8      m_u1WTAI;                           //�����߳�AI���أ�0Ϊ�رգ�1Ϊ��
	uint32     m_u4DisposeTime;                    //ҵ�������ʱʱ��
	uint32     m_u4WTCheckTime;                    //�����̳߳�ʱ����ʱ�䷶Χ����λ����
	uint32     m_u4WTTimeoutCount;                 //�����̳߳�ʱ���ĵ�λʱ���ڵĳ�ʱ��������
	uint32     m_u4WTStopTime;                     //ֹͣ����������ʱ��

	_WorkThreadAIInfo()
	{
		m_u4ThreadID       = 0;
		m_u1WTAI           = 0;
		m_u4DisposeTime    = 0;
		m_u4WTCheckTime    = 0;
		m_u4WTTimeoutCount = 0;
		m_u4WTStopTime     = 0;
	}
};

//��ʱ���Ԫ
struct _CommandTimeout
{
	uint32 m_u4ThreadID;        //�����߳�ID
	uint16 m_u2CommandID;       //��ʱ������
	uint32 m_u4Second;          //��ʱ��ǰʱ�䣬��1970��������ʼ���������
	uint32 m_u4Timeout;         //����ִ��ʱ�䣬��λ�Ǻ���  

	_CommandTimeout()
	{
		m_u4ThreadID  = 0;
		m_u2CommandID = 0;
		m_u4Second    = 0;
		m_u4Timeout   = 0;
	}
};

//�����û��ش���ز�ѯ��Ϣ
typedef vector<_CommandTimeout> vecCommandTimeout;

class CWorkThreadAI
{
public:
	CWorkThreadAI();
	~CWorkThreadAI();

	void Close();

	void Init(uint8 u1AI, uint32 u4DisposeTime, uint32 u4WTCheckTime, uint32 u4WTTimeoutCount, uint32 u4WTStopTime, uint8 u1WTReturnDataType, const char* pReturnData);

	bool SaveTimeout(uint16 u2CommandID, uint32 u4TimeCost);

	char* GetReturnData();
	uint16 GetReturnDataLength();

	void GetAIInfo(_WorkThreadAIInfo& objWorkThreadAIInfo);

	void ReSet(uint8 u1AI, uint32 u4DisposeTime, uint32 u4WTCheckTime, uint32 u4WTStopTime);

	bool CheckCurrTimeout(uint16 u2CommandID, uint32 u4Now);

	void GetAllTimeout(uint32 u4ThreadID, vecCommandTimeout& objTimeout);
	void GetAllForbiden(uint32 u4ThreadID, vecCommandTimeout& objForbiden);

private:
	uint8      m_u1WTAI;                           //�����߳�AI���أ�0Ϊ�رգ�1Ϊ��
	uint16     m_u4DisposeTime;                    //ҵ�������ʱʱ��
	uint32     m_u4WTCheckTime;                    //�����̳߳�ʱ����ʱ�䷶Χ����λ����
	uint32     m_u4WTTimeoutCount;                 //�����̳߳�ʱ���ĵ�λʱ���ڵĳ�ʱ��������
	uint32     m_u4WTStopTime;                     //ֹͣ����������ʱ��
	uint8      m_u1WTReturnDataType;               //���ش������ݵ����ͣ�1Ϊ�����ƣ�2Ϊ�ı�
	char       m_szWTReturnData[MAX_BUFF_1024];    //���ص������壬���1K
	uint16     m_u2ReturnDataLen;                  //���������峤��

private:
	//��ʱ�������
	struct _CommandTime
	{
		uint16 m_u2CommandID;
		CRingLink<_CommandTimeout> m_objTime;

		_CommandTime()
		{
			m_u2CommandID = 0;
		}
	};

	vector<_CommandTime*>   m_vecCommandTime;
	vector<_CommandTimeout> m_vecCommandTimeout;
};
#endif
