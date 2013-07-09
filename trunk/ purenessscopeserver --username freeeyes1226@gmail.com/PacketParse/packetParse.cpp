#include "PacketParse.h"


CPacketParse::CPacketParse(void)
{
	//在这里设置包头的长度，因为大部分的包头长度是固定的。包体长度是可变的。
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;
	m_u4HeadSrcSize     = 0;
	m_u4BodySrcSize     = 0;

    //这里修改属于你的包解析版本号
    sprintf_safe(m_szPacketVersion, MAX_BUFF_20, "0.90");

	//这里设置你的包模式
	m_u1PacketMode      = PACKET_WITHHEAD;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

CPacketParse::~CPacketParse(void)
{
	m_objCurrBody.Close();
}

void CPacketParse::Init()
{
	m_u4PacketHead      = PACKET_HEAD;
	m_u2PacketCommandID = 0;
	m_u4PacketData      = 0;
	m_u4HeadSrcSize     = 0;
	m_u4BodySrcSize     = 0;

	m_blIsHead          = false;

	m_pmbHead           = NULL;
	m_pmbBody           = NULL;
}

uint32 CPacketParse::GetPacketHeadLen()
{
	return m_u4PacketHead;
}

uint32 CPacketParse::GetPacketDataLen()
{
	return m_u4PacketData;
}

uint16 CPacketParse::GetPacketCommandID()
{
	return m_u2PacketCommandID;
}

bool CPacketParse::GetIsHead()
{
	return m_blIsHead;
}

bool CPacketParse::SetPacketHead(char* pData, uint32 u4Len)
{
	//这里添加自己对包头的分析，主要分析出包长度。
	m_u4HeadSrcSize = u4Len;
	if(u4Len == sizeof(uint32))
	{
		ACE_OS::memcpy(&m_u4PacketData, pData, sizeof(uint32));
		m_blIsHead = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool CPacketParse::SetPacketData(char* pData, uint32 u4Len)
{
	//这里分析出包体内的一些数据，如果包头包含了CommandID，那么包体就不必做解析。
	m_u4BodySrcSize = u4Len;
	if(u4Len >= sizeof(uint16))
	{
		ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));
		m_blIsHead = false;
		return true;
	}
	else
	{
		m_blIsHead = false;
		return false;
	}
}

uint32 CPacketParse::GetPacketHeadSrcLen()
{
	return m_u4HeadSrcSize;
}

uint32 CPacketParse::GetPacketBodySrcLen()
{
	return m_u4BodySrcSize;
}

bool CPacketParse::SetMessageHead(ACE_Message_Block* pmbHead)
{
	m_pmbHead = pmbHead;
	return true;
}

bool CPacketParse::SetMessageBody(ACE_Message_Block* pmbBody)
{
	m_pmbBody = pmbBody;
	return true;
}

ACE_Message_Block* CPacketParse::GetMessageHead()
{
	return m_pmbHead;
}

ACE_Message_Block* CPacketParse::GetMessageBody()
{
	return m_pmbBody;
}

uint32 CPacketParse::MakePacketLength(uint32 u4DataLen)
{
	return u4DataLen + sizeof(uint32);
}

bool CPacketParse::MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData)
{
	if(pMbData == NULL)
	{
		return false;
	}

	//拼装数据包
	ACE_OS::memcpy(pMbData->wr_ptr(), (const void*)&u4Len, sizeof(uint32));
	ACE_OS::memcpy(pMbData->wr_ptr() + sizeof(uint32), (const void*)pData, u4Len);
	pMbData->wr_ptr(u4Len + sizeof(uint32));

	return true;
}

void CPacketParse::Close()
{
	if(m_pmbHead != NULL)
	{
		m_pmbHead->release();
		m_pmbHead = NULL;
	}

	if(m_pmbBody != NULL)
	{
		m_pmbBody->release();
		m_pmbBody = NULL;
	}

	m_blIsHead = false;
}

const char* CPacketParse::GetPacketVersion()
{
  return m_szPacketVersion;
}

uint8 CPacketParse::GetPacketMode()
{
	return (uint8)m_u1PacketMode;
}

uint8 CPacketParse::GetPacketStream(ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager)
{
	//这里是测试代码，专门处理为数据流的数据包
	if(NULL == pCurrMessage || NULL == pMessageBlockManager)
	{
		return PACKET_GET_ERROR;
	}

	//这里添加了我的规则，比如，我的数据包第一个字节是7E开头，以7E为结尾，那么我会这么做
	//首先，判断数据里面第一个字节是不是7E，如果不是，则返回错误的数据。
	//如果是错误数据，为了保证数据安全，框架会关闭这个链接。
	char* pData   = pCurrMessage->rd_ptr();     //得到这个数据块的首字节
	uint32 u4Data = pCurrMessage->length();     //得到这个数据块的长度

	//判断数据长度是否为0
	if(u4Data <= 0)
	{
		return PACKET_GET_ERROR;
	}

	//如果收首字节不是7E，则证明数据要不不全需要粘包，要不就是错数据包。
	if(pData[0] == 0x7E)
	{
		//包头正确，开始寻找包尾
		uint32 u4Pos  = 1;
		bool   blFind = false;
		for(u4Pos = 1; u4Pos < u4Data; u4Pos++)
		{
			//找到了包末尾
			if(pData[u4Pos] == 0x7E)
			{
				blFind = true;
				break;
			}
		}

		if(blFind == false)
		{
			//将不完整的数据放入缓冲，等待完整后放入包体
			m_objCurrBody.WriteStream(pCurrMessage->rd_ptr() + 1, pCurrMessage->length() - 1);

			//没有找到包尾，需要继续接受数据
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//接收到了完整数据包
			uint32 u4PacketLen = u4Pos - 1;

			m_u4PacketHead  = sizeof(uint32);
			m_u4HeadSrcSize = sizeof(uint32);
			m_u4PacketData  = u4PacketLen;
			m_u4BodySrcSize = u4PacketLen;

			//从内存池中申请一个包头
			m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
			if(NULL == m_pmbHead)
			{
				return PACKET_GET_ERROR;
			}

			//记录包长，并转换为网络字节序，放入包头
			//uint32 u4NetPacketLen = ACE_HTONL(u4PacketLen);
			uint32 u4NetPacketLen = u4PacketLen;
			memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));
			
			//从内存池申请一个包体
			m_pmbBody = pMessageBlockManager->Create(u4PacketLen);
			if(NULL == m_pmbBody)
			{
				return PACKET_GET_ERROR;
			}

			//获得包命令ID
			ACE_OS::memcpy(&m_u2PacketCommandID, (char*)&pData[1], sizeof(uint16));

			//将包内容放入包体
			memcpy(m_pmbBody->wr_ptr(), (char*)&pData[1], u4PacketLen);

			//处理完的数据从池中移除
			pCurrMessage->rd_ptr(u4Pos);
			return PACKET_GET_ENOUGTH;
		}
	}
	else
	{
		//如果缓冲不存在，则说明缓冲中的包都解析完毕了
		if(m_objCurrBody.GetPacketLen() <= 0)
		{
			return PACKET_GET_ERROR;
		}

		uint32 u4Pos  = 1;
		bool   blFind = false;
		if(m_objCurrBody.GetPacketLen() > 0)
		{
			//如果有缓冲，则看看是否有结束标记
			for(u4Pos = 0; u4Pos < u4Data; u4Pos++)
			{
				//找到了包末尾
				if(pData[u4Pos] == 0x7E)
				{
					blFind = true;
					break;
				}
			}
		}
		else
		{
			//没有缓冲，则从首字节判断
			for(u4Pos = 1; u4Pos < u4Data; u4Pos++)
			{
				//找到了包末尾
				if(pData[u4Pos] == 0x7E)
				{
					blFind = true;
					break;
				}
			}
		}

		if(blFind == false)
		{
			//将不完整的数据放入缓冲，等待完整后放入包体
			m_objCurrBody.WriteStream(pData, u4Data);

			//没有找到包尾，需要继续接受数据
			return PACKET_GET_NO_ENOUGTH;
		}
		else
		{
			//接收到了完整数据包
			if(u4Pos > 0)
			{
				m_objCurrBody.WriteStream(pData, u4Pos);
			}
			
			m_u4PacketHead  = sizeof(uint32);
			m_u4HeadSrcSize = sizeof(uint32);
			m_u4PacketData  = m_objCurrBody.GetPacketLen();
			m_u4BodySrcSize = m_objCurrBody.GetPacketLen();

			//从内存池中申请一个包头
			m_pmbHead = pMessageBlockManager->Create(sizeof(uint32));
			if(NULL == m_pmbHead)
			{
				return PACKET_GET_ERROR;
			}

			//记录包长，并转换为网络字节序，放入包头
			//uint32 u4NetPacketLen = ACE_HTONL(m_objCurrBody.GetPacketLen());
			uint32 u4NetPacketLen = m_objCurrBody.GetPacketLen();
			memcpy(m_pmbHead->wr_ptr(), (char*)&u4NetPacketLen, sizeof(uint32));

			//从内存池申请一个包体
			m_pmbBody = pMessageBlockManager->Create(m_objCurrBody.GetPacketLen());
			if(NULL == m_pmbBody)
			{
				return PACKET_GET_ERROR;
			}

			//获得包命令ID
			ACE_OS::memcpy(&m_u2PacketCommandID, pData, sizeof(uint16));


			//将包内容放入包体
			memcpy(m_pmbBody->wr_ptr(), (char*)m_objCurrBody.GetData(), m_objCurrBody.GetPacketLen());

			//处理完的数据从池中移除
			pCurrMessage->rd_ptr(u4Pos);
			return PACKET_GET_ENOUGTH;
		}

	}
}
