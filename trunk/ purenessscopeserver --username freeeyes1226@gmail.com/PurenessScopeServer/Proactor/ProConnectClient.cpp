#include "ProConnectClient.h"
#include "ClientProConnectManager.h"

CProConnectClient::CProConnectClient(void)
{
	m_nIOCount          = 1;
	m_nServerID         = 0;
	m_mbRecv            = NULL;
	m_pClientMessage    = NULL;
    m_u4MaxPacketSize   = MAX_MSG_PACKETLENGTH;

	m_u4SendSize        = 0;
	m_u4SendCount       = 0;
	m_u4RecvSize        = 0;
	m_u4RecvCount       = 0;
	m_u4CostTime        = 0;
}

CProConnectClient::~CProConnectClient(void)
{
}

void CProConnectClient::Close()
{
	m_ThreadWritrLock.acquire();
	if(m_nIOCount > 0)
	{
		m_nIOCount--;
	}
	m_ThreadWritrLock.release();

	if(m_nIOCount == 0)
	{
		m_Reader.cancel();
		m_Writer.cancel();

		if(this->handle() != ACE_INVALID_HANDLE)
		{
			ACE_OS::closesocket(this->handle());
			this->handle(ACE_INVALID_HANDLE);
		}

		App_ClientProConnectManager::instance()->CloseByClient(m_nServerID);

		OUR_DEBUG((LM_DEBUG, "[CProConnectClient::Close]delete OK.\n"));
		delete this;
	}
}

void CProConnectClient::ClientClose()
{
	if(this->handle() != ACE_INVALID_HANDLE)
	{
		ACE_OS::shutdown(this->handle(), SD_SEND);
		ACE_OS::closesocket(this->handle());
		this->handle(ACE_INVALID_HANDLE);
	}
}

void CProConnectClient::SetServerID(int nServerID)
{
	m_nServerID = nServerID;
}

void CProConnectClient::SetClientMessage(IClientMessage* pClientMessage)
{
	m_pClientMessage = pClientMessage;
}

int CProConnectClient::GetServerID()
{
	return m_nServerID;
}

void CProConnectClient::open(ACE_HANDLE h, ACE_Message_Block&)
{
  //�������ļ���ȡ����
  m_u4MaxPacketSize  = App_MainConfig::instance()->GetRecvBuffSize();

	m_nIOCount = 1;
	this->handle(h);
	if(this->m_Reader.open(*this, h, 0, App_ProactorManager::instance()->GetAce_Proactor(REACTOR_POSTDEFINE)) == -1||this->m_Writer.open(*this, h,  0, App_ProactorManager::instance()->GetAce_Proactor(REACTOR_POSTDEFINE)) == -1)
	{
		OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::open] m_reader or m_reader == 0.\n"));	
		Close();
		return;
	}

	m_u4SendSize        = 0;
	m_u4SendCount       = 0;
	m_u4RecvSize        = 0;
	m_u4RecvCount       = 0;
	m_u4CostTime        = 0;
	m_atvBegin          = ACE_OS::gettimeofday();

	App_ClientProConnectManager::instance()->SetHandler(m_nServerID, this);
	m_pClientMessage = App_ClientProConnectManager::instance()->GetClientMessage(m_nServerID);

	RecvData(MAX_BUFF_1024);
}


void CProConnectClient::handle_read_stream(const ACE_Asynch_Read_Stream::Result &result)
{
	ACE_Message_Block& mb = result.message_block();
	uint32 u4PacketLen = (uint32)result.bytes_transferred();
	
	if(!result.success() || u4PacketLen == 0)
	{
		mb.release();
		if(NULL != m_pClientMessage)
		{
			m_pClientMessage->ConnectError((int)ACE_OS::last_error());
		}
		Close();
		return;
	}
	else 
	{
		//�����������(���ﲻ�����ǲ����������������ϲ��߼��Լ�ȥ�ж�)
		m_pClientMessage->RecvData(&mb);
		mb.release();

		//������һ�����ݰ�
		RecvData(MAX_BUFF_1024);
	}
}

void CProConnectClient::handle_write_stream(const ACE_Asynch_Write_Stream::Result &result)
{
	ACE_Message_Block& mblk = result.message_block();
	if(!result.success() || result.bytes_transferred() == 0)
	{
		//�������ݷ��ͳ���
		OUR_DEBUG((LM_DEBUG, "[CProConnectClient::handle_write_stream]Write error(%d).\n", ACE_OS::last_error()));
		mblk.release();
		if(NULL != m_pClientMessage)
		{
			m_pClientMessage->ConnectError((int)ACE_OS::last_error());
		}
		Close();
	}
	else
	{
		mblk.release();
	}
}

void CProConnectClient::addresses(const ACE_INET_Addr &remote_address, const ACE_INET_Addr &local_address)
{
	m_AddrRemote = remote_address;
}

bool CProConnectClient::RecvData(uint32 u4PacketLen)
{
	ACE_NEW_NORETURN(m_mbRecv, ACE_Message_Block(u4PacketLen));
	if(this->m_Reader.read(*m_mbRecv, m_mbRecv->space()) == -1)
	{
		OUR_DEBUG((LM_DEBUG,"[CProConnectHandle::open] m_reader is error(%d).\n", (int)ACE_OS::last_error()));	
		m_mbRecv->release();
		if(NULL != m_pClientMessage)
		{
			m_pClientMessage->ConnectError((int)ACE_OS::last_error());
		}
		Close();
		return false;
	}

	return true;
}

bool CProConnectClient::SendData(ACE_Message_Block* pmblk)
{
	//OUR_DEBUG((LM_DEBUG, "[CProConnectClient::SendData]Begin.\n"));

	//�����DEBUG״̬����¼��ǰ���ܰ��Ķ���������
	if(App_MainConfig::instance()->GetDebug() == DEBUG_ON)
	{
		string strDebugData;
		char szLog[10]  = {'\0'};
		int  nDebugSize = 0; 
		bool blblMore   = false;

		if(pmblk->length() >= MAX_BUFF_200)
		{
			nDebugSize = MAX_BUFF_200;
			blblMore   = true;
		}
		else
		{
			nDebugSize = pmblk->length();
		}

		char* pData = pmblk->rd_ptr();
		for(int i = 0; i < nDebugSize; i++)
		{
			sprintf_safe(szLog, 10, "0x%02X ", (unsigned char)pData[i]);
			strDebugData += szLog;
		}

		if(blblMore == true)
		{
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_DEBUG_SERVERSEND, "[%s:%d]%s.(���ݰ�����ֻ��¼ǰ200�ֽ�)", m_AddrRemote.get_host_addr(), m_AddrRemote.get_port_number(), strDebugData.c_str());
		}
		else
		{
			AppLogManager::instance()->WriteLog(LOG_SYSTEM_DEBUG_SERVERSEND, "[%s:%d]%s.", m_AddrRemote.get_host_addr(), m_AddrRemote.get_port_number(), strDebugData.c_str());
		}
	}

	//������Ϣ
	if(NULL != pmblk)
	{
		int nLen = (int)pmblk->length();
		if (m_Writer.write(*pmblk, pmblk->length()) == -1)
		{
			OUR_DEBUG((LM_DEBUG,"[CProConnectClient::SendData] Send Error(%d).\n", ACE_OS::last_error()));	
			pmblk->release();
			if(NULL != m_pClientMessage)
			{
				m_pClientMessage->ConnectError((int)ACE_OS::last_error());
			}
			Close();
			return false;
		}

		m_u4SendSize += (uint32)nLen;
		m_u4SendCount++;		
	}

	return true;
}

_ClientConnectInfo CProConnectClient::GetClientConnectInfo()
{
	_ClientConnectInfo ClientConnectInfo;
	ClientConnectInfo.m_blValid       = true;
	//ClientConnectInfo.m_addrRemote    = m_AddrRemote;
	ClientConnectInfo.m_u4AliveTime   = (uint32)(ACE_OS::gettimeofday().sec() - m_atvBegin.sec());
	ClientConnectInfo.m_u4AllRecvSize = m_u4RecvSize;
	ClientConnectInfo.m_u4RecvCount   = m_u4RecvCount;
	ClientConnectInfo.m_u4AllSendSize = m_u4SendSize;
	ClientConnectInfo.m_u4SendCount   = m_u4SendCount;
	ClientConnectInfo.m_u4ConnectID   = m_nServerID;
	ClientConnectInfo.m_u4BeginTime   = (uint32)m_atvBegin.sec();
	return ClientConnectInfo;
}

