#pragma once 

//���������ȫ����������ʹ�õ�
//�����￪����ֻҪȥʵ�������5���ӿڣ��Ϳ������һ�����ݰ��������
//���������һ�����ӣ����ֻ����ģʽ�Ļ�����Ҫ�ٹ��캯������ָ��m_u1PacketModeΪPACKET_WITHSTREAM
//Ȼ��ʵ��GetPacketStream()������������һ����д��С���ӡ�
//��ʵ����ģʽ����ֻ����ް�ͷ��Э�飬�а�ͷ��Э�飬�㲻����SetPacketHead��SetPacketBody
//��Ҳ�����Լ�ȥ��GetPacketStream���������ʵ��
//�����Լ���ϲ�ã������һ����Ƽ�ǰ�ߣ���Ϊ����������Ч�ʱȽϸߡ�
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

	//ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	//��Ϊ�õ����ڴ�أ�����pHead��pBody�ɿ���ṩ�������ɿ�ܻ��գ������������������new������pHead��pBody�����������ڴ�й¶��
	//����Ҫע��һ�°�����Ȼ����������ǰ��Ľӿڷ�������Ҳ����������ӿ�ʵ����Ĺ���ǰ�������m_u1PacketMode������PACKET_WITHSTREAM
	uint8 GetPacketStream(ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	//ƴ�����ݷ��ذ�
	bool MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	uint32 MakePacketLength(uint32 u4DataLen);

};
