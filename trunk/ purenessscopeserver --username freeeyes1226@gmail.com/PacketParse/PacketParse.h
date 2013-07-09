#pragma once 
#include "BuffPacket.h"
#include "IMessageBlockManager.h"

#ifdef WIN32
#if defined PACKETPARSE_BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif 

#ifdef WIN32
class DLL_EXPORT CPacketParse
#else
class CPacketParse
#endif 
{
public:
	CPacketParse(void);
	~CPacketParse(void);

	void Init();

	uint32 GetPacketHeadLen();
	uint32 GetPacketDataLen();
	uint16 GetPacketCommandID();

	bool GetIsHead();

	ACE_Message_Block* GetMessageHead();
	ACE_Message_Block* GetMessageBody();
	bool SetMessageHead(ACE_Message_Block* pmbHead);
	bool SetMessageBody(ACE_Message_Block* pmbBody);

	bool SetPacketHead(char* pData, uint32 u4Len);
	bool SetPacketData(char* pData, uint32 u4Len);

	const char* GetPacketVersion();
	uint8 GetPacketMode();                                     //得到当前包的模式，1是带包头的，0是不带包头的（需要判定头尾标志的），默认是1

	//专门对应 Mode为0的不带包头的数据包,如果是带包头的模式，这里什么都不用做。
	//因为用到了内存池，所以pHead和pBody由框架提供，并且由框架回收，所以在这里，不可以用new出来的pHead和pBody，否则会造成内存泄露。
	//这里要注意一下啊。当然，如果你觉得前面的接口繁琐，你也可以用这个接口实现你的规则，前提是你的m_u1PacketMode必须是PACKET_WITHSTREAM
	uint8 GetPacketStream(ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	uint32 GetPacketHeadSrcLen();                     //得到数据包原始包头长度
	uint32 GetPacketBodySrcLen();                     //得到数据包原始包体长度

	//拼接数据返回包
	bool MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	uint32 MakePacketLength(uint32 u4DataLen);

	void Close();

private:
	uint32 m_u4PacketHead;               //包头的长度
	uint32 m_u4PacketData;               //包体的长度
	uint32 m_u4HeadSrcSize;              //包头的原始长度 
	uint32 m_u4BodySrcSize;              //包体的原始长度
	uint16 m_u2PacketCommandID;          //包命令
	bool   m_blIsHead;
    char   m_szPacketVersion[MAX_BUFF_20];   //包解析器版本
	uint8  m_u1PacketMode;                   //包解析模式    
 
	ACE_Message_Block* m_pmbHead;   //包头部分
	ACE_Message_Block* m_pmbBody;   //包体部分

	CBuffPacket m_objCurrBody;      //记录尚未完整的包体

};
