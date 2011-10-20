#pragma once 
#include "define.h"

#ifdef WIN32
#if defined DLLCLASS_EXPORTS
#define DLLCLASS_EXPORTS __declspec(dllexport)
#else
#define DLLCLASS_EXPORTS __declspec(dllimport)
#endif
#else
#define DLLCLASS_EXPORTS
#endif 


class DLLCLASS_EXPORTS CPacketParse
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

	//拼接数据返回包
	bool MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	uint32 MakePacketLength(uint32 u4DataLen);

	void Close();

private:
	uint32 m_u4PacketHead;
	uint32 m_u4PacketData;
	uint16 m_u2PacketCommandID;
	bool   m_blIsHead;

	ACE_Message_Block* m_pmbHead;   //包头部分
	ACE_Message_Block* m_pmbBody;   //包体部分

};
