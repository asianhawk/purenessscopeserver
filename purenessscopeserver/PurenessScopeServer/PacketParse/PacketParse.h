#ifndef _PACKETPARSE_H
#define _PACKETPARSE_H

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
class CPacketParse : public CPacketParseBase
#endif 
{
public:
	CPacketParse(void);
	virtual ~CPacketParse(void);

	bool SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager);
	bool SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager);

	//ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	//��Ϊ�õ����ڴ�أ�����pHead��pBody�ɿ���ṩ�������ɿ�ܻ��գ������������������new������pHead��pBody�����������ڴ�й¶��
	//����Ҫע��һ�°�����Ȼ����������ǰ��Ľӿڷ�������Ҳ����������ӿ�ʵ����Ĺ���ǰ�������m_u1PacketMode������PACKET_WITHSTREAM
	uint8 GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	//ƴ�����ݷ��ذ�
	bool MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData, uint16 u2CommandID = 0);
	uint32 MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen, uint16 u2CommandID = 0);

	//�����ӵ�һ�ν�����ʱ�򣬷��صĽӿ��������Լ��Ĵ���
	bool Connect(uint32 u4ConnectID, _ClientIPInfo& objClientIPInfo);
	//�����ӶϿ���ʱ�򣬷������Լ��Ĵ���
	void DisConnect(uint32 u4ConnectID);

};
#endif

