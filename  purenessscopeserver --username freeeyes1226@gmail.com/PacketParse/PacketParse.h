#pragma once 

//这个类是完全交给开发者使用的
//在这里开发者只要去实现下面的5个接口，就可以完成一个数据包处理过程
//这里给出了一个例子，如果只用流模式的话，需要再构造函数里面指定m_u1PacketMode为PACKET_WITHSTREAM
//然后实现GetPacketStream()函数，里面有一个我写的小例子。
//其实，流模式并非只针对无包头的协议，有包头的协议，你不想用SetPacketHead和SetPacketBody
//你也可以自己去在GetPacketStream这个函数里实现
//看你自己的喜好，不过我还是推荐前者，因为这样做可能效率比较高。
//add by freeeyes

#include "PacketParseBase.h"

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
class DLL_EXPORT CPacketParse : public CPacketParseBase
#else
class CPacketParse
#endif 
{
public:
	CPacketParse(void);
	virtual ~CPacketParse(void);

	bool SetPacketHead(ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager);
	bool SetPacketBody(ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager);

	//专门对应 Mode为0的不带包头的数据包,如果是带包头的模式，这里什么都不用做。
	//因为用到了内存池，所以pHead和pBody由框架提供，并且由框架回收，所以在这里，不可以用new出来的pHead和pBody，否则会造成内存泄露。
	//这里要注意一下啊。当然，如果你觉得前面的接口繁琐，你也可以用这个接口实现你的规则，前提是你的m_u1PacketMode必须是PACKET_WITHSTREAM
	uint8 GetPacketStream(ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	//拼接数据返回包
	bool MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	uint32 MakePacketLength(uint32 u4DataLen);

};
