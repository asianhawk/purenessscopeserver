#ifndef _QUEUEDEFINE_H
#define _QUEUEDEFINE_H

#include "define.h"

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#define PERMS_IPC 0600

//PSS�������ݺ��͵�����̵���Ϣ����ID
#define PSS_QUEUE_ID     11000
//����̷��ظ�PSS��������Ϣ����ID
#define MI_QUEUE_ID      11001

//PSS����������ݽṹ
struct _Pss_QueueData
{
	uint32 m_u4ConnectID;
	uint16 m_u2CommandID;

	_Pss_QueueData()
	{
		m_u4ConnectID = 0;
		m_u2CommandID = 0;
	}
};

//Muit�������������Ϣ
struct _Muti_QueueData
{
	uint32 m_u4ConnectID;
	uint16 m_u2CommandID;

	_Muti_QueueData()
	{
		m_u4ConnectID = 0;
		m_u2CommandID = 0;
	}
};

//��ö���ID
inline int GetLinuxMsgQID(key_t key)
{
#ifdef __LINUX__
	int nMsgQID = -1;

	if(key <= 0)
	{
		return -1;
	}

	if((nMsgQID = msgget(key, PERMS_IPC|IPC_CREAT)) < 0) 
	{
		return -2;
	}

	return nMsgQID;
#else
	//��֧��windows
	return 0;
#endif
}

//��ö�����Ϣ
inline int GetDataFromMsgQ(int nMsgQID, unsigned char *pData, int nLen)
{
#ifdef __LINUX__
	int nRetLen = 0;
	if(nMsgQID < 0)
	{
		return -1;
	}

	//IPC_NOWAITΪ������������Ϊ����
	while((nRetLen = msgrcv(nMsgQID, pData, nLen, 0, 0)) < 0)
	{
		if (errno == EIDRM)
		{
			exit(1);
		}

		if (errno != EINTR)
		{
			return -2;
		}
	}

	return nRetLen;
#else
	//��֧��windows
	return -1;
#endif
}

//д����Ϣ������Ϣ
inline int PutDataToMsgQ(int nMsgQID, const unsigned char *pData, int nLen)
{
#ifdef __LINUX__
	int ret;

	if (nMsgQID < 0)
	{
		return -1;
	}

	// �����ǲ������źŴ�ϣ����򲻻���������
	while ((ret = msgsnd(nMsgQID, pData, nLen, IPC_NOWAIT)) < 0)
	{
		if (errno == EIDRM)
		{
			exit(1);
		}

		if (errno != EINTR)
		{
			break;
		}
	}

	return ret;
#else
	//��֧��windows
	return -1;
#endif
}


#endif
