#ifndef _PACKETPARSE_H
#define _PACKETPARSE_H

//�������ʵ��һ����Http�����ݽ����Լ�ӳ��
//add by freeeyes

#include "PacketParseBase.h"
#include "HttpInfo.h"

#define HTTP_HEAD_END    "\r\n\r\n"
#define HTTP_BODY_LENGTH "Content-Length: "

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

	//��ʼ��PacketParsed
	void Init();

	//�õ��������������ݰ�ͷ���ݿ飬u4ConnectID������ID��pmbHead�����ݿ飬pMessageBlockManager�����ݿ�أ�������ý����������������Ч
	bool SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager);
	//�õ��������������ݰ������ݿ飬u4ConnectID������ID��pmbHead�����ݿ飬pMessageBlockManager�����ݿ�أ�������ý����������������Ч
	bool SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager);

	//ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	//��Ϊ�õ����ڴ�أ�����pHead��pBody�ɿ���ṩ�������ɿ�ܻ��գ������������������new������pHead��pBody�����������ڴ�й¶��
	//����Ҫע��һ�°�����Ȼ����������ǰ��Ľӿڷ�������Ҳ����������ӿ�ʵ����Ĺ���ǰ�������m_u1PacketMode������PACKET_WITHSTREAM
	uint8 GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	//ƴ�����ݷ��ذ������еķ������ݰ�����������
	bool MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData, uint16 u2CommandID = 0);
	//�õ��������ݰ��ĳ���
	uint32 MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen, uint16 u2CommandID = 0);
	//�����ӵ�һ�ν�����ʱ�򣬷��صĽӿ��������Լ��Ĵ���
	bool Connect(uint32 u4ConnectID, _ClientIPInfo& objClientIPInfo);
	//�����ӶϿ���ʱ�򣬷������Լ��Ĵ���
	void DisConnect(uint32 u4ConnectID);

private:
	//���º���Http����
	//����Http��ͷ
	uint8 HttpDispose(_HttpInfo* pHttpInfo, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);

	//������Http���峤��
	uint8 GetHttpBodyLen(char* pData, uint32 u4Len, uint32 u4HeadLen, uint32& u4BodyLen);

};

#endif
