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
	uint8 GetPacketMode();                                     //�õ���ǰ����ģʽ��1�Ǵ���ͷ�ģ�0�ǲ�����ͷ�ģ���Ҫ�ж�ͷβ��־�ģ���Ĭ����1

	//ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	//��Ϊ�õ����ڴ�أ�����pHead��pBody�ɿ���ṩ�������ɿ�ܻ��գ������������������new������pHead��pBody�����������ڴ�й¶��
	//����Ҫע��һ�°�����Ȼ����������ǰ��Ľӿڷ�������Ҳ����������ӿ�ʵ����Ĺ���ǰ�������m_u1PacketMode������PACKET_WITHSTREAM
	uint8 GetPacketStream(ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager);   

	uint32 GetPacketHeadSrcLen();                     //�õ����ݰ�ԭʼ��ͷ����
	uint32 GetPacketBodySrcLen();                     //�õ����ݰ�ԭʼ���峤��

	//ƴ�����ݷ��ذ�
	bool MakePacket(const char* pData, uint32 u4Len, ACE_Message_Block* pMbData);
	uint32 MakePacketLength(uint32 u4DataLen);

	void Close();

private:
	uint32 m_u4PacketHead;               //��ͷ�ĳ���
	uint32 m_u4PacketData;               //����ĳ���
	uint32 m_u4HeadSrcSize;              //��ͷ��ԭʼ���� 
	uint32 m_u4BodySrcSize;              //�����ԭʼ����
	uint16 m_u2PacketCommandID;          //������
	bool   m_blIsHead;
    char   m_szPacketVersion[MAX_BUFF_20];   //���������汾
	uint8  m_u1PacketMode;                   //������ģʽ    
 
	ACE_Message_Block* m_pmbHead;   //��ͷ����
	ACE_Message_Block* m_pmbBody;   //���岿��

	CBuffPacket m_objCurrBody;      //��¼��δ�����İ���

};
