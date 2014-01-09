#include "ProxyClient.h"

CProxyClient::CProxyClient( void )
{
	m_nIOCount      = 0;
	m_u4RecvAllSize = 0;
	m_u4SendAllSize = 0;
	m_u4ConnectID   = 0;
	m_pServerObject = NULL;
}

CProxyClient::~CProxyClient( void )
{
}

bool CProxyClient::Close()
{
	m_ThreadLock.acquire();
	if(m_nIOCount > 0)
	{
		m_nIOCount--;
	}
	m_ThreadLock.release();

	//�ӷ�Ӧ��ע���¼�
	if(m_nIOCount == 0)
	{
		//msg_queue()->deactivate();
		shutdown();
		OUR_DEBUG((LM_ERROR, "[CProxyClient::Close]Close(%s:%d) OK.\n", m_addrRemote.get_host_addr(), m_addrRemote.get_port_number()));

		//�ر�Զ�̺Ϳͻ��˵�����
		if(NULL != m_pServerObject)
		{
			uint32 u4ConnectID = App_ProxyManager::instance()->FindConnectID(this);
			if(0 != u4ConnectID)
			{
				m_pServerObject->GetConnectManager()->CloseConnect(u4ConnectID);
			}
		}

		//ɾ��ӳ���ϵ
		App_ProxyManager::instance()->DeleteByProxyClient(this);

		//�ع��ù���ָ��
		delete this;
		return true;
	}

	return false;
}

int CProxyClient::open( void* )
{
	//��������Ϊ������ģʽ
	if (this->peer().enable(ACE_NONBLOCK) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProxyClient::open]this->peer().enable  = ACE_NONBLOCK error.\n"));
		return -1;
	}

	//���Զ�����ӵ�ַ�Ͷ˿�
	if(this->peer().get_remote_addr(m_addrRemote) == -1)
	{
		OUR_DEBUG((LM_ERROR, "[CProxyClient::open]this->peer().get_remote_addr error.\n"));
		return -1;
	}

	m_nIOCount = 1;

	//���ӳ���ϵ
	App_ProxyManager::instance()->Insert(m_u4ConnectID, this);

	//ע����¼�
	this->reactor()->register_handler(this,  ACE_Event_Handler::READ_MASK);
		
	OUR_DEBUG((LM_ERROR, "[CProxyClient::open]open OK.\n"));

	return 0;

}

int CProxyClient::handle_input( ACE_HANDLE fd /*= ACE_INVALID_HANDLE*/ )
{
	ACE_Time_Value     nowait(MAX_MSG_PACKETTIMEOUT);

	//���յ�Զ�̵������������ݣ��������ݷ��ظ��ͻ���
	char szClientBuff[MAX_RECV_BUFF] = {'\0'};

	int nDataLen = this->peer().recv(szClientBuff, MAX_RECV_BUFF, MSG_NOSIGNAL, &nowait);
	if(nDataLen <= 0)
	{
		uint32 u4Error = (uint32)errno;
		OUR_DEBUG((LM_ERROR, "[CProxyClient::handle_input] ConnectID = %d, recv data is error nDataLen = [%d] errno = [%d].\n", m_u4ConnectID, nDataLen, u4Error));
		return -1;
	}
	else
	{
		//OUR_DEBUG((LM_ERROR, "[CProxyClient::handle_input] ConnectID = %d, nDataLen=%d.\n", m_u4ConnectID, nDataLen));
		//������յ������ݷ��͸��ͻ���
		if(NULL != m_pServerObject)
		{
			m_pServerObject->GetConnectManager()->PostMessage(m_u4ConnectID, szClientBuff, nDataLen, SENDMESSAGE_JAMPNOMAL,
															  (uint16)COMMAND_RETURN_PROXY, PACKET_SEND_IMMEDIATLY, PACKET_IS_SELF_RECYC);
		}
	}

	//�����¼���յ����ֽ���
	m_u4RecvAllSize += nDataLen;

	return 0;
}

int CProxyClient::handle_close( ACE_HANDLE h, ACE_Reactor_Mask mask )
{
	if(h == ACE_INVALID_HANDLE)
	{
		OUR_DEBUG((LM_DEBUG,"[CProxyClient::handle_close] h is NULL mask=%d.\n", m_u4ConnectID, (int)mask));
	}

	Close();
	return 0;
}

bool CProxyClient::SendData(char* pData, int nLen)
{
	ACE_Time_Value     nowait(MAX_MSG_PACKETTIMEOUT);

	int nSendLen = nLen;   //�������ݵ��ܳ���
	int nIsSendSize = 0;

	//ѭ�����ͣ�ֱ�����ݷ�����ɡ�
	while(true)
	{
		if(nSendLen <= 0)
		{
			OUR_DEBUG((LM_ERROR, "[CProxyClient::SendData] ConnectID = %d, nCurrSendSize error is %d.\n", m_u4ConnectID, nSendLen));
			return false;
		}

		int nDataLen = this->peer().send(&pData[nIsSendSize], nSendLen - nIsSendSize, &nowait);
		int nErr = ACE_OS::last_error();
		if(nDataLen <= 0)
		{
			if(nErr == EWOULDBLOCK)
			{
				//������Ͷ��������10������ٷ��͡�
				ACE_Time_Value tvSleep(0, 10 * MAX_BUFF_1000);
				ACE_OS::sleep(tvSleep);
				continue;
			}

			OUR_DEBUG((LM_ERROR, "[CProxyClient::SendData] ConnectID = %d, error = %d.\n", m_u4ConnectID, errno));
			Close();
			return false;
		}
		else if(nDataLen + nIsSendSize >= nSendLen)   //�����ݰ�ȫ��������ϣ���ա�
		{
			//OUR_DEBUG((LM_ERROR, "[CConnectHandler::handle_output] ConnectID = %d, send (%d) OK.\n", GetConnectID(), msg_queue()->is_empty()));
			m_u4SendAllSize += (uint32)nSendLen;
			return true;
		}
		else
		{
			nIsSendSize      += nDataLen;
			continue;
		}
	}

	return true;
}

void CProxyClient::SetServerObject(uint32 u4ConnectID, CServerObject* pServerObject)
{
	m_u4ConnectID   = u4ConnectID;
	m_pServerObject = pServerObject;
}