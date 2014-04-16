#pragma once

#include "IBuffPacket.h"
#include "ClientCommand.h"
#include "IObject.h"
#include "QueueDefine.h"

//����ͻ�������(TCP)
#define COMMAND_LINUXQUEUE         0x2200     //������Ϣ����
#define COMMAND_RETURN_LINUXQUEUE  0xe200     //���ض��д�����Ϣ

using namespace std;

class CBaseCommand : public CClientCommand
{
public:
  CBaseCommand(void);
  ~CBaseCommand(void);

  int DoMessage(IMessage* pMessage, bool& bDeleteFlag);
  void SetServerObject(CServerObject* pServerObject);

  bool InitQueue();          //��ʼ���̶߳���
  int  GetMutiQueueID();     //�õ�MutiQueueID

  void SendData(_Muti_QueueData& objMutiQueueData);

private:
	void Do_Linux_Queue(IMessage* pMessage);        //�����½

private:
  CServerObject*    m_pServerObject;
  int               m_nPssQueueID;
  int               m_nMutiQueueID;
};


