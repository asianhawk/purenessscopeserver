#ifndef _PACKETCONSOLEPARSE_H
#define _PACKETCONSOLEPARSE_H

//���������ȫ����������ʹ�õ�
//�����￪����ֻҪȥʵ�������5���ӿڣ��Ϳ������һ�����ݰ��������
//���������һ�����ӣ����ֻ����ģʽ�Ļ�����Ҫ�ٹ��캯������ָ��m_u1PacketModeΪPACKET_WITHSTREAM
//Ȼ��ʵ��GetPacketStream()������������һ����д��С���ӡ�
//��ʵ����ģʽ����ֻ����ް�ͷ��Э�飬�а�ͷ��Э�飬�㲻����SetPacketHead��SetPacketBody
//��Ҳ�����Լ�ȥ��GetPacketStream���������ʵ��
//�����Լ���ϲ�ã������һ����Ƽ�ǰ�ߣ���Ϊ����������Ч�ʱȽϸߡ�
//add by freeeyes

#include "PacketParseBase.h"

class CConsolePacketParse : public CPacketParseBase 
{
public:
	CConsolePacketParse(void);
	virtual ~CConsolePacketParse(void);

	//�õ��������������ݰ�ͷ���ݿ飬u4ConnectID������ID��pmbHead�����ݿ飬pMessageBlockManager�����ݿ�أ�������ý����������������Ч
	bool SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager);
	//�õ��������������ݰ������ݿ飬u4ConnectID������ID��pmbHead�����ݿ飬pMessageBlockManager�����ݿ�أ�������ý����������������Ч
	bool SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager);

	//ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	//��Ϊ�õ����ڴ�أ�����pHead��pBody�ɿ���ṩ�������ɿ�ܻ��գ������������������new������pHead��pBody�����������ڴ�й¶��
	//����Ҫע��һ�°�����Ȼ����������ǰ��Ľӿڷ�������Ҳ����������ӿ�ʵ����Ĺ���ǰ�������m_u1PacketMode������PACKET_WITHSTREAM
	uint8 GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	//ƴ�����ݷ��ذ������еķ������ݰ�����������
	bool MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	//�õ��������ݰ��ĳ���
	uint32 MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen);
	//�����ӵ�һ�ν�����ʱ�򣬷��صĽӿ��������Լ��Ĵ���
	bool Connect(uint32 u4ConnectID);
	//�����ӶϿ���ʱ�򣬷������Լ��Ĵ���
	void DisConnect(uint32 u4ConnectID);

};

#endif
