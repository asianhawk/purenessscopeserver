#ifndef _ICLIENTMESSAGE_H
#define _ICLIENTMESSAGE_H

//����һ���ӿ��࣬�������ݷ��ͳ�ȥ����������ݷ��أ��ṩ���Ӵ������
//��������RecvMessage()�������ݺ�ConnectError()���ʹ������ַ�����
//add by freeeyes 2010-12-31

#include "define.h"
#include "ace/Message_Block.h"

class IClientMessage
{
public:
	virtual ~IClientMessage() {};
	
	virtual void ReConnect(int nServerID)                                            = 0;    //���������ɹ��ӿ�
	virtual bool RecvData(ACE_Message_Block* mbRecv, _ClientIPInfo objServerIPInfo)  = 0;    //�������ݵĺ���
	virtual bool ConnectError(int nError, _ClientIPInfo objServerIPInfo)             = 0;    //�������ʱ�򣬵��ô˽ӿڷ��ش�����Ϣ  
};

#endif
