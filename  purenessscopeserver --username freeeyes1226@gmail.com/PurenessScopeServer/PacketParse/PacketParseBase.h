#ifndef _PACKETPARSEBASE_H
#define _PACKETPARSEBASE_H

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
class DLL_EXPORT CPacketParseBase
#else
class CPacketParseBase
#endif 
{
public:
	CPacketParseBase(void);

	virtual ~CPacketParseBase(void);

	void Init();                                 //��ʼ��CPacketParseBase���������ݶ���
	void Clear();                                //����ǰ���ݻ��� 
	void Close();                                //ɾ�����е�ǰ���ݶ���

	const char* GetPacketVersion();              //�õ��������߼��汾 
	uint8 GetPacketMode();                       //�õ���ǰ����ģʽ��1�Ǵ���ͷ�ģ�0�ǲ�����ͷ�ģ���Ҫ�ж�ͷβ��־�ģ���Ĭ����1
	uint32 GetPacketHeadLen();                   //�õ����ݰ�ͷ���ȣ����ܺ�
	uint32 GetPacketBodyLen();                   //�õ����ݰ��峤�ȣ����ܺ�

	uint16 GetPacketCommandID();
	bool GetIsHead();                            //�õ���ͷ���λ�������ǰ�ǰ�ͷ���򷵻�True�����򷵻�False
	uint32 GetPacketHeadSrcLen();                //�õ����ݰ�ͷ���ȣ�����ǰ��
	uint32 GetPacketBodySrcLen();                //�õ����ݰ��峤�ȣ�����ǰ��

	ACE_Message_Block* GetMessageHead();         //�õ����ݰ�ͷ���ݣ����ܺ� 
	ACE_Message_Block* GetMessageBody();         //�õ����ݰ������ݣ����ܺ�

	virtual bool SetPacketHead(uint32 u4ConnectID, ACE_Message_Block* pmbHead, IMessageBlockManager* pMessageBlockManager)         = 0;  //���õõ������ݰ�ͷ�������������
	virtual bool SetPacketBody(uint32 u4ConnectID, ACE_Message_Block* pmbBody, IMessageBlockManager* pMessageBlockManager)         = 0;  //���õõ������ݰ��壬����������� 
	virtual uint8 GetPacketStream(uint32 u4ConnectID, ACE_Message_Block* pCurrMessage, IMessageBlockManager* pMessageBlockManager) = 0;  //ר�Ŷ�Ӧ ModeΪ0�Ĳ�����ͷ�����ݰ�,����Ǵ���ͷ��ģʽ������ʲô����������
	virtual bool MakePacket(uint32 u4ConnectID, const char* pData, uint32 u4Len, ACE_Message_Block* pMbData) = 0;                        //�������ذ���ṹ
	virtual uint32 MakePacketLength(uint32 u4ConnectID, uint32 u4DataLen)                                    = 0;                        //Ԥ�ȵõ����ذ���ĳ���

	virtual bool Connect(uint32 u4ConnectID)                                                                                       = 0;  //�������ӽ����Ĵ���
	virtual void DisConnect(uint32 u4ConnectID)                                                                                    = 0;  //�������ӶϿ��Ĵ���

protected:
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

#endif
