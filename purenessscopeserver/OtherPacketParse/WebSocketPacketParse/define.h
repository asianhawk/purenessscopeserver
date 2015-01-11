#ifndef _DEFINE_H
#define _DEFINE_H

// Define.h
// ���ﶨ�����з������õ���ͨ�õĶ��������������һЩace��ͷ��ͨ�õ�һЩ���ݷ���
// add by freeeyes
// 2008-12-22

#include "ace/SString.h"
#include "ace/Malloc.h"
#include "ace/Malloc_T.h"
#include "ace/Task_T.h"
#include "ace/Local_Memory_Pool.h"
#include "ace/Time_Value.h"
#include "ace/OS_main.h"
#include "ace/OS_NS_sys_stat.h"
#include "ace/OS_NS_sys_socket.h"
#include "ace/OS_NS_unistd.h"
#include "ace/High_Res_Timer.h"
#include "ace/INET_Addr.h" 
#include <math.h>

#include <vector>

using namespace std;

#ifndef NULL
#define NULL 0
#endif

#define MAINCONFIG            "main.xml"
#define ALERTCONFIG           "alert.xml"
#define FORBIDDENIP_FILE      "forbiddenIP.xml"

#define MAX_BUFF_9    9
#define MAX_BUFF_20   20
#define MAX_BUFF_50   50
#define MAX_BUFF_100  100
#define MAX_BUFF_200  200
#define MAX_BUFF_300  300
#define MAX_BUFF_500  500
#define MAX_BUFF_1000 1000
#define MAX_BUFF_1024 1024

/*
//���㵱ǰ�汾���Ƿ����ƶ��汾��һ��
static bool Convert_Version(int nTagVserion)
{
	int nCurrVserion = 0;
	nCurrVserion += (int)ACE::major_version() * 1000;
	nCurrVserion += (int)ACE::minor_version() * 100;
	nCurrVserion += (int)ACE::beta_version();

	if(nTagVserion >= nCurrVserion)
	{
		return true;
	}
	else
	{
		return false;
	}
}

#define CONVERT_ACE_VERSION Convert_Version(6200)
*/

//���ݲ�ͬ�Ĳ���ϵͳ�����岻ͬ��recv���ղ�������
#ifdef WIN32
#define MSG_NOSIGNAL          0            //�ź���������WINDOWS��
#endif

#define SERVER_ACTOR_REACTOR  0
#define SERVER_ACTOR_PROACTOR 1

#define SERVER_ACTOR_TYPE     SERVER_ACTOR_PROACTOR         //��ǰ���������õļܹ����ͣ�������Proactor��Ҳ������Reactor

#define MAX_DEV_POLL_COUNT    5000         //Devpoll�������ӵ�������

#define CONSOLE_ENABLE        1            //�򿪺�̨�˿�
#define CONSOLE_DISABLE       0            //�رպ�̨�˿�

#define CONNECT_CLOSE_TIME    2            //���ӻ���ر�ʱ���ӳ�

#define MAX_MSG_THREADCOUNT   1            //Ĭ�ϵ��߼��̴߳�����
#define MAX_MSG_THREADQUEUE   60000        //�����̵߳���Ϣ������
#define MAX_MSG_TIMEDELAYTIME 60           //�߼��߳��Լ�ʱ����
#define MAX_MSG_STARTTIME     1            //�߼��̴߳���ʼʱ��
#define MAX_MSG_MASK          64000        //�߼�Mark���߳���
#define MAX_MSG_PUTTIMEOUT    100          //�����߼����ӳ�
#define MAX_MSG_SENDPACKET    10           //��໺�巢�Ͱ��ĸ���,����������ֱ�����������һ���������ݰ�
#define MAX_MSG_SNEDTHRESHOLD 10           //���ͷ�ֵ(��Ϣ���ĸ���)
#define MAX_MSG_SENDCHECKTIME 100          //ÿ�����ٺ������һ�η��͵ķ�ֵ
#define MAX_MSG_THREADTIMEOUT 30           //�߳��ж���ʱ
#define MAX_MSG_PACKETTIMEOUT 5            //�������ݰ���ʱʱ��
#define MAX_MSG_SOCKETBUFF    50*1024      //�������ݻ����С
#define MAX_MSG_SENDTIMEOUT   5            //���ͳ�ʱʱ��
#define MAX_MSG_HANDLETIME    120          //ͳ�Ƶ�ǰ�������ļ��
#define MAX_MSG_SENDALIVETIME 60           //���ʹ�����ʱ����
#define MAX_MSG_SENDALCOUNT   5            //�ж������Ƿ���ļ���
#define MAX_MSG_PACKETLENGTH  20*1024      //���һ���ݰ��������� 
#define MAX_MP_POOL           5000         //_MakePacket����������Ĵ�С
#define MAX_HANDLE_POOL       1000         //Ĭ��Handler����صĴ�С
#define MAX_ASYNCH_BACKLOG    100          //Ĭ�����õ�BACKLOG����
#define MAX_LOADMODEL_CLOSE   5            //Ĭ�ϵȴ�ģ��ر�ʱ��
#define MAX_CONNECT_COUNT     10000        //Ĭ�ϵ�λʱ�����������������
#define MAX_BLOCK_SIZE        2048         //Ĭ�������������ݿ�Ĵ�С
#define MAX_BLOCK_COUNT       3            //Ĭ������Block����
#define MAX_BLOCK_TIME        1            //Ĭ�ϵȴ��ط�ʱ�䣨��λ���룩
#define MAX_QUEUE_TIMEOUT     20           //Ĭ�϶��г�ʱ����ʱ��
#define MAX_RECV_UDP_TIMEOUT  3            //�����ճ�ʱʱ��(UDP) 

#define PACKET_PARSE          1            //��Ϣ�������־
#define PACKET_CONNECT        2            //���ӽ����¼���Ϣ��־
#define PACKET_CDISCONNECT    3            //�ͻ��˶Ͽ��¼���Ϣ��־
#define PACKET_SDISCONNECT    4            //�������Ͽ��¼���Ϣ��־
#define PACKET_SEND_TIMEOUT   6            //����������ʱ�䳬����ֵ�ı�־

#define MAX_PACKET_PARSE      5000         //PACKETPARSE����ظ���
#define MAX_MESSAGE_POOL      5000         //Message����ظ���

#define PACKET_HEAD           4            //��ͷ����
#define BUFFPACKET_MAX_COUNT  5000         //��ʼ��BuffPacket������صĸ���
#define SENDQUEUECOUNT        1            //Ĭ�Ϸ����̶߳��е�����

#define MAX_POSTMESSAGE_SIZE  65536        //����PostMessageѭ��

#define MAX_TIMERMANAGER_THREADCOUNT      1   //Timer������������߳���

#define PARM_CONNECTHANDLE_CHECK          2   //��ʱ�����Ͱ����
#define PARM_HANDLE_CLOSE                 1   //��ʱ������ر�

#define HANDLE_ISCLOSE_NO                 0      //�����Ѿ��ر�
#define HANDLE_ISCLOSE_YES                1      //����Ŀǰ����

#define TYPE_IPV4                         1      //IPv4��׼
#define TYPE_IPV6                         2      //IPv6��׼

#define MAX_UDP_PACKET_LEN                1024   //UDP���ݰ�������С
#define UDP_HANDER_ID                     0      //Ĭ��UDP��ConnectID

#define PACKET_TCP                        0      //���ݰ���Դ����TCP
#define PACKET_UDP                        1      //���ݰ���Դ����UDP

#define CONNECT_TCP                       0      //��������ΪTCP
#define CONNECT_UDP                       1      //��������ΪUDP

#define SENDMESSAGE_NOMAL                 0      //Ĭ�Ϸ������ݰ�ģʽ(��PacketPrase�˿�)
#define SENDMESSAGE_JAMPNOMAL             1      //�������ݰ�ģʽ(����PacketPrase�˿�)

#define COMMAND_TYPE_IN                   0      //��������������״̬������CommandData��ͳ��������Ϣ�ࣩ
#define COMMAND_TYPE_OUT                  1      //���������������״̬������CommandData��ͳ��������Ϣ�ࣩ

#define PACKET_WITHSTREAM                 0      //������ͷ��������ģʽ
#define PACKET_WITHHEAD                   1      //����ͷ�����ݰ�ģʽ

#define PACKET_GET_ENOUGTH                0      //�õ����������ݰ�����Ҫ��������
#define PACKET_GET_NO_ENOUGTH             1      //�õ������ݰ�������
#define PACKET_GET_ERROR                  2      //���ݰ���ʽ����

#define MAX_PACKET_SIZE     1024*1024            //�������İ��ߴ�

#define PACKET_IS_FRAMEWORK_RECYC         true   //��ܻ��գ����ݰ����ջ��ƣ�
#define PACKET_IS_SELF_RECYC              false  //���߼��Լ����գ����ݰ����ջ��ƣ�

#define PACKET_SEND_IMMEDIATLY            true   //���̷��ͣ����ݰ����ͻ��ƣ�
#define PACKET_SEND_CACHE                 false  //���淢�ͣ����ݰ����ͻ��ƣ�

#define CONNECT_LIMIT_RETRY 30                   //��ʼ���м���������Ӻ��ڼ�飬��λ����

#define TCP_NODELAY_ON      0                    //TCP Nagle�㷨���ؿ��ش�
#define TCP_NODELAY_OFF     1                    //TCP Nagle�㷨���ؿ��عر�

//��Ӧ��ǰ���֧�ֵ�����ģʽ
enum
{
	NETWORKMODE_PRO_IOCP    = 1,    //IOCPģʽ
	NETWORKMODE_RE_SELECT   = 10,   //Selectģʽ
	NETWORKMODE_RE_TPSELECT = 11,   //TPSelectģʽ 
	NETWORKMODE_RE_EPOLL    = 12,   //epollltģʽ(ˮƽ����)
	NETWORKMODE_RE_EPOLL_ET = 13,   //epolletģʽ(���ش���)
};

//��Ӧ���ӵ�״̬��������������ʱ���״̬
enum
{
	CONNECT_INIT         = 0,
	CONNECT_OPEN         = 1,
	CONNECT_RECVGEGIN    = 2,
	CONNECT_RECVGEND     = 3,
	CONNECT_SENDBEGIN    = 4,
	CONNECT_SENDEND      = 5,
	CONNECT_CLOSEBEGIN   = 6,
	CONNECT_CLOSEEND     = 7,
	CONNECT_RECVERROR    = 8,
	CONNECT_SENDERROR    = 9,
	CONNECT_SENDBUFF     = 10,
	CONNECT_SENDNON      = 11,
	CONNECT_SERVER_CLOSE = 12,
};

//��������ͨѶ���Ƿ���Ҫ�ص���ö��
enum EM_s2s
{
	S2S_NEED_CALLBACK = 0,    //��Ҫ�ص�
	S2S_INNEED_CALLBACK,      //����Ҫ�ص�
};

//��Ӧ�����̵߳�״̬
enum
{
	THREAD_INIT      = 0,   //�̳߳�ʼ��
	THREAD_RUNBEGIN  = 1,   //��ʼ�����߳�
	THREAD_RUNEND    = 2,   //�������ݽ���
	THREAD_BLOCK     = 3,   //�߳�����
};

//��Ӧ��������ͨѶ�Ĵ���״̬
enum EM_Server_Connect_State
{
	SERVER_CONNECT_READY = 0,
	SERVER_CONNECT_FIRST,
	SERVER_CONNECT_OK,
	SERVER_CONNECT_FAIL,
	SERVER_CONNECT_RECONNECT,
};

//��־�������
#define LOG_SYSTEM                      1000
#define LOG_SYSTEM_ERROR                1001
#define LOG_SYSTEM_CONNECT              1002
#define LOG_SYSTEM_WORKTHREAD           1003
#define LOG_SYSTEM_POSTTHREAD           1004
#define LOG_SYSTEM_UDPTHREAD            1005
#define LOG_SYSTEM_POSTCONNECT          1006
#define LOG_SYSTEM_PACKETTIME           1007
#define LOG_SYSTEM_PACKETTHREAD         1008
#define LOG_SYSTEM_CONNECTABNORMAL      1009
#define LOG_SYSTEM_RECVQUEUEERROR       1010
#define LOG_SYSTEM_SENDQUEUEERROR       1011
#define LOG_SYSTEM_COMMANDDATA          1012
#define LOG_SYSTEM_CONSOLEDATA          1013
#define LOG_SYSTEM_DEBUG_CLIENTRECV     1014
#define LOG_SYSTEM_DEBUG_CLIENTSEND     1015
#define LOG_SYSTEM_DEBUG_SERVERRECV     1016
#define LOG_SYSTEM_DEBUG_SERVERSEND     1017
#define LOG_SYSTEM_MONITOR              1018 

#define DEBUG_ON  1
#define DEBUG_OFF 0

#define OUR_DEBUG(X)  ACE_DEBUG((LM_INFO, "[%D|%t]")); ACE_DEBUG(X)

enum
{
	REACTOR_CLIENTDEFINE  = 0,
	REACTOR_POSTDEFINE    = 1,
	REACTOR_UDPDEFINE     = 2,
};

//�м��������ID
enum
{
	POSTSERVER_TEST    = 1,
};

//*****************************************************************
//�����������������ͷ��һ�������ӽ�����һ���������˳�
#define CLIENT_LINK_CONNECT     0x0001      //�û�����
#define CLIENT_LINK_CDISCONNET  0x0002      //�ͻ����˳�
#define CLIENT_LINK_SDISCONNET  0x0003      //�������˳�
#define CLINET_LINK_SENDTIMEOUT 0x0004      //���������Ϳͻ���ʱ�䳬����ֵ
//*****************************************************************

//*****************************************************************

//����һ���ڴ���������
typedef  ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> MUTEX_MALLOC;
typedef ACE_Allocator_Adapter<MUTEX_MALLOC> Mutex_Allocator;

#ifndef uint8
typedef ACE_UINT8 uint8;
#endif

#ifndef uint16
typedef ACE_UINT16 uint16;
#endif

#ifndef uint32
typedef ACE_UINT32 uint32;
#endif

#ifndef uint64
typedef ACE_UINT64 uint64;
#endif

#ifndef int8
typedef char int8;
#endif

#ifndef int16
typedef short int16;
#endif

#ifndef int32
typedef int int32;
#endif

#ifndef float32
typedef float float32;
#endif

#ifndef float64
typedef double float64;
#endif

#ifdef UNICODE
typedef wofstream _tofstream;
typedef wifstream _tifstream;
typedef std::wstring _tstring;
#else
typedef ofstream _tofstream;
typedef ifstream _tifstream;
typedef std::string _tstring;
#endif // UNICODE

//���VCHARS_TYPE��ģʽ
enum VCHARS_TYPE
{
	VCHARS_TYPE_TEXT = 0,      //�ı�ģʽ
	VCHARS_TYPE_BINARY,        //������ģʽ
};

#ifndef VCHARS_STR
typedef  struct _VCHARS_STR 
{
	char*       text;            //����ָ��
	uint8       u1Len;           //���ݳ���
	bool        blCopy;          //�Ƿ񿽱����ݿ飬True�ǿ�����Ĭ���ǿ���
	bool        blNew;           //�Ƿ���new����������
	VCHARS_TYPE type;            //���ͣ����Ͷ����VCHARS_TYPE

	_VCHARS_STR(bool blIsCopy = true, VCHARS_TYPE ntype = VCHARS_TYPE_TEXT)
	{
		text   = NULL;
		u1Len  = 0;
		blCopy = blIsCopy;
		type   = ntype;
		blNew  = false;
	}

	~_VCHARS_STR()
	{
		if(blNew == true)
		{
			delete text;
		}
	}

	void SetData(const char* pData, uint8& u1Length)
	{
		if(blCopy == true)
		{
			//�������Ҫ�������ڴ棬������������
			if(blNew == true)
			{
				delete text;
			}

			if(type == VCHARS_TYPE_TEXT)
			{
				//�ı�ģʽ
				text = new char[u1Length + 1];
				ACE_OS::memcpy(text, pData, u1Length);
				text[u1Length] = '\0';
				u1Len = u1Length + 1;
			}
			else
			{
				//������ģʽ
				text = new char[u1Length];
				ACE_OS::memcpy(text, pData, u1Length);
				u1Len = u1Length;
			}
			blNew = true;
		}
		else
		{
			text  = (char* )pData;
			u1Len = u1Length;
		}
	}

}VCHARS_STR;
#endif

#ifndef VCHARM_STR
typedef  struct _VCHARM_STR 
{
	char*       text;            //����ָ�� 
	uint16      u2Len;           //���ݳ���
	bool        blCopy;          //�Ƿ񿽱����ݿ飬True�ǿ�����Ĭ���ǿ���
	bool        blNew;           //�Ƿ���new����������
	VCHARS_TYPE type;            //���ͣ����Ͷ����VCHARS_TYPE

	_VCHARM_STR(bool blIsCopy = true, VCHARS_TYPE ntype = VCHARS_TYPE_TEXT)
	{
		text   = NULL;
		u2Len  = 0;
		blCopy = blIsCopy;
		type   = ntype;
		blNew  = false;
	}

	~_VCHARM_STR()
	{
		if(blNew == true)
		{
			delete text;
		}
	}

	void SetData(const char* pData, uint16& u2Length)
	{
		if(blCopy == true)
		{
			//�������Ҫ�������ڴ棬������������
			if(blNew == true)
			{
				delete text;
			}

			if(type == VCHARS_TYPE_TEXT)
			{
				//�ı�ģʽ
				text = new char[u2Length + 1];
				ACE_OS::memcpy(text, pData, u2Length);
				text[u2Length] = '\0';
				u2Len = u2Length + 1;
			}
			else
			{
				//������ģʽ
				text = new char[u2Length];
				ACE_OS::memcpy(text, pData, u2Length);
				u2Len = u2Length;
			}
			blNew = true;
		}
		else
		{
			text  = (char* )pData;
			u2Len = u2Length;
		}
	}

}VCHARM_STR;
#endif

#ifndef VCHARB_STR
typedef  struct _VCHARB_STR 
{
	char*       text;            //����ָ�� 
	uint32      u4Len;           //���ݳ���
	bool        blCopy;          //�Ƿ񿽱����ݿ飬True�ǿ�����Ĭ���ǿ���
	bool        blNew;           //�Ƿ���new����������
	VCHARS_TYPE type;            //���ͣ����Ͷ����VCHARS_TYPE

	_VCHARB_STR(bool blIsCopy = true, VCHARS_TYPE ntype = VCHARS_TYPE_TEXT)
	{
		text   = NULL;
		u4Len  = 0;
		blCopy = blIsCopy;
		type   = ntype;
		blNew  = false;
	}

	~_VCHARB_STR()
	{
		if(blNew == true)
		{
			delete text;
		}
	}

	void SetData(const char* pData, uint32& u4Length)
	{
		if(blCopy == true)
		{
			//�������Ҫ�������ڴ棬������������
			if(blNew == true)
			{
				delete text;
			}

			if(type == VCHARS_TYPE_TEXT)
			{
				//�ı�ģʽ
				text = new char[u4Length + 1];
				ACE_OS::memcpy(text, pData, u4Length);
				text[u4Length] = '\0';
				u4Len = u4Length + 1;
			}
			else
			{
				//������ģʽ
				text = new char[u4Length];
				ACE_OS::memcpy(text, pData, u4Length);
				u4Len = u4Length;
			}
			blNew = true;
		}
		else
		{
			text  = (char* )pData;
			u4Len = u4Length;
		}
	}

}VCHARB_STR;
#endif

//���ڼ�¼���ݰ�ͷ��Ϣ
#define SESSION_LEN           32     //Session�Ĵ�С
struct _PacketHeadInfo
{
	uint16 m_u2Version;              //Э��汾��
	uint16 m_u2CmdID;                //Э��������   
	uint32 m_u4BodyLen;              //Э�����ݰ��峤��  
	char   m_szSession[SESSION_LEN]; //���ݰ�Session 

	void Clear()
	{
		m_u2Version    = 0;
		m_u2CmdID      = 0;
		m_u4BodyLen    = 0;
		m_szSession[0] = '\0';
	}
};

//��ʱ������ݰ���������������Ϣ������������Ч�Ե��߼��ж�
struct _TimeConnectInfo
{
	uint8  m_u1Minutes;               //��ǰ�ķ�����
	uint32 m_u4RecvPacketCount;       //��ǰ���հ�����
	uint32 m_u4RecvSize;              //��ǰ����������
	uint32 m_u4SendPacketCount;       //��ǰ���Ͱ�����
	uint32 m_u4SendSize;              //��ǰ����������

	uint32 m_u4ValidRecvPacketCount;  //��λʱ�������������ݰ�������
	uint32 m_u4ValidRecvSize;         //��λʱ�����������ݽ�����
	uint32 m_u4ValidSendPacketCount;  //��λʱ��������������ݰ�������
	uint32 m_u4ValidSendSize;         //��λʱ�����������ݷ�����

	_TimeConnectInfo()
	{ 
		m_u1Minutes              = 0;
		m_u4RecvPacketCount      = 0;
		m_u4RecvSize             = 0;
		m_u4SendPacketCount      = 0;
		m_u4SendSize             = 0;

		m_u4ValidRecvPacketCount = 0;
		m_u4ValidRecvSize        = 0;
		m_u4ValidSendPacketCount = 0;
		m_u4ValidSendSize        = 0;
	}

	void Init(uint32 u4RecvPacketCount, uint32 u4RecvSize, uint32 u4SendPacketCount, uint32 u4ValidSendSize)
	{
		m_u1Minutes              = 0;
		m_u4RecvPacketCount      = 0;
		m_u4RecvSize             = 0;
		m_u4SendPacketCount      = 0;
		m_u4SendSize             = 0;

		m_u4ValidRecvPacketCount = u4RecvPacketCount;
		m_u4ValidRecvSize        = u4RecvSize;
		m_u4ValidSendPacketCount = u4SendPacketCount;
		m_u4ValidSendSize        = u4ValidSendSize;
	}

	bool RecvCheck(uint8 u1Minutes, uint16 u2PacketCount, uint32 u4RecvSize)
	{
		if(m_u1Minutes != u1Minutes)
		{
			m_u1Minutes         = u1Minutes;
			m_u4RecvPacketCount = u2PacketCount;
			m_u4RecvSize        = u4RecvSize;
		}
		else
		{
			m_u4RecvPacketCount += u2PacketCount;
			m_u4RecvSize        += u4RecvSize;
		}

		if(m_u4ValidRecvPacketCount > 0)
		{
			//��Ҫ�Ƚ�
			if(m_u4RecvPacketCount > m_u4ValidRecvPacketCount)
			{
				return false;
			}
		}

		if(m_u4ValidRecvSize > 0)
		{
			//��Ҫ�Ƚ�
			if(u4RecvSize > m_u4ValidRecvSize)
			{
				return false;
			}
		}

		return true;
	}

	bool SendCheck(uint8 u1Minutes, uint16 u2PacketCount, uint32 u4SendSize)
	{
		if(m_u1Minutes != u1Minutes)
		{
			m_u1Minutes         = u1Minutes;
			m_u4SendPacketCount = u2PacketCount;
			m_u4RecvSize        = u4SendSize;
		}
		else
		{
			m_u4SendPacketCount += u2PacketCount;
			m_u4RecvSize        += u4SendSize;
		}

		if(m_u4ValidSendPacketCount > 0)
		{
			//��Ҫ�Ƚ�
			if(m_u4SendPacketCount > m_u4ValidSendPacketCount)
			{
				return false;
			}
		}

		if(m_u4ValidSendSize > 0)
		{
			//��Ҫ�Ƚ�
			if(u4SendSize > m_u4ValidSendSize)
			{
				return false;
			}
		}

		return true;
	}
};

//��ʱ�����������ýṹ
struct _TimerCheckID 
{
	uint16 m_u2TimerCheckID;

	_TimerCheckID()
	{
		m_u2TimerCheckID = 0;
	}
};

//Message�������ݿ�ṹ��
struct _PacketInfo
{
	char*   m_pData;            //�����������ͷָ��
	int     m_nDataLen;         //����������ݳ���

	_PacketInfo()
	{
		m_pData       = NULL;
		m_nDataLen    = 0;
	}
};

//�ͻ���������Ϣ���ݽṹ
struct _ClientConnectInfo
{
	bool          m_blValid;              //��ǰ�����Ƿ���Ч
	uint32        m_u4ConnectID;          //����ID
	ACE_INET_Addr m_addrRemote;           //Զ�����ӵ�ַ
	uint32        m_u4RecvCount;          //���հ�����
	uint32        m_u4SendCount;          //���Ͱ�����
	uint32        m_u4AllRecvSize;        //�����ֽ���
	uint32        m_u4AllSendSize;        //�����ֽ���
	uint32        m_u4BeginTime;          //���ӽ���ʱ��
	uint32        m_u4AliveTime;          //���ʱ������
	uint32        m_u4RecvQueueCount;     //�����߼�������ĸ���
	uint64        m_u8RecvQueueTimeCost;  //�����߼��������ʱ������
	uint64        m_u8SendQueueTimeCost;  //����������ʱ������

	_ClientConnectInfo()
	{
		m_blValid             = false;
		m_u4ConnectID         = 0;
		m_u4RecvCount         = 0;
		m_u4SendCount         = 0;
		m_u4AllRecvSize       = 0;
		m_u4AllSendSize       = 0;
		m_u4BeginTime         = 0;
		m_u4AliveTime         = 0;
		m_u4RecvQueueCount    = 0;
		m_u8RecvQueueTimeCost = 0;
		m_u8SendQueueTimeCost = 0;
	}

	_ClientConnectInfo& operator = (const _ClientConnectInfo& ar)
	{
		this->m_blValid             = ar.m_blValid;
		this->m_u4ConnectID         = ar.m_u4ConnectID;
		this->m_addrRemote          = ar.m_addrRemote;
		this->m_u4RecvCount         = ar.m_u4RecvCount;
		this->m_u4SendCount         = ar.m_u4SendCount;
		this->m_u4AllRecvSize       = ar.m_u4AllRecvSize;
		this->m_u4AllSendSize       = ar.m_u4AllSendSize;
		this->m_u4BeginTime         = ar.m_u4BeginTime;
		this->m_u4AliveTime         = ar.m_u4AliveTime;
		this->m_u4RecvQueueCount    = ar.m_u4RecvQueueCount;
		this->m_u8RecvQueueTimeCost = ar.m_u8RecvQueueTimeCost;
		this->m_u8SendQueueTimeCost = ar.m_u8SendQueueTimeCost;
		return *this;
	}
};
typedef vector<_ClientConnectInfo> vecClientConnectInfo;

//Ҫ���ӵķ�������Ϣ
struct _ServerConnectInfo
{
	uint32      m_u4ServerID;     //��������ID
	ACE_TString m_strServerName;  //������������
	ACE_TString m_strServerIP;    //��������IP
	uint32      m_u4ServerPort;   //�������Ķ˿�
	uint32      m_u4MaxConn;      //������������߳�������
	uint32      m_u4TimeOut;      //�����������ӳ�ʱʱ��

	_ServerConnectInfo()
	{
		m_u4ServerID   = 0;
		m_u4ServerPort = 0;
		m_u4MaxConn    = 0;
		m_u4TimeOut    = 0;
	}
};

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if( (x) != NULL ) { delete (x); (x) = NULL; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if( (x) != NULL ) { delete[] (x); (x) = NULL; }
#endif

//����һ������������֧���ڴ�Խ����
inline void sprintf_safe(char* szText, int nLen, const char* fmt ...)
{
	if(szText == NULL)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	ACE_OS::vsnprintf(szText, nLen, fmt, ap);
	szText[nLen - 1] = '\0';

	va_end(ap);
};

//Ϊ�߼����ṩһ��Try catch�ı����꣬���ڵ��ԣ�����ʹ�÷�����ο�TestTcp����
//Ŀǰ���֧��һ��2K����־
//************************************************************************
#define ASSERT_LOG_PATH  "./Log/assert.log"   //���·�����Լ�Ҫ�����޸����

inline void __show__( const char* szTemp)
{
#ifdef WIN32
	printf_s("[__show__]%s.\n", szTemp);
#else
	printf("[__show__]%s.\n", szTemp);
#endif

	FILE* f = ACE_OS::fopen(ASSERT_LOG_PATH, "a") ;
	ACE_OS::fwrite( szTemp, strlen(szTemp), sizeof(char), f) ;
	ACE_OS::fwrite( "\r\n", 1, 2*sizeof(char), f);
	fclose(f);
};

inline void __assertspecial__(const char* file, int line, const char* func, const char* expr, const char* msg)
{
	char szTemp[2*MAX_BUFF_1024] = {0};

	sprintf_safe( szTemp, 2*MAX_BUFF_1024, "Alert[%s][%d][%s][%s][%s]", file, line, func, expr ,msg) ;
	__show__(szTemp) ;
};

#if defined(WIN32)
#define AssertSpecial(expr,msg) ((void)((expr)?0:(__assertspecial__(__FILE__, __LINE__, __FUNCTION__, #expr, msg),0)))
#else
#define AssertSpecial(expr,msg) {if(!(expr)){__assertspecial__(__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr, msg);}}
#endif

#if defined(WIN32)
#define __ENTER_FUNCTION() {try{
#define __THROW_FUNCTION(msg) throw(msg)
#define __LEAVE_FUNCTION() }catch(char* msg){AssertSpecial(false,msg); }}
#define __LEAVE_FUNCTION_WITHRETURN(ret) }catch(char* msg){AssertSpecial(false,msg); return ret; }}
#else	//linux
#define __ENTER_FUNCTION() {try{
#define __THROW_FUNCTION(msg) throw(msg)
#define __LEAVE_FUNCTION() }catch(char* msg){AssertSpecial(false,msg);}}
#define __LEAVE_FUNCTION_WITHRETURN(ret) }catch(char* msg){AssertSpecial(false,msg); return ret; }}
#endif 

//************************************************************************

//����һ��ͳ�ƺ�������ʱ��ĺ꣬��������ֱ��ͳ�Ƶ�ǰ����ִ��ʱ��
//ʹ������ __TIMECOST(100); 100Ϊ���룬����100����ͻ������־
//************************************************************************
#define ASSERT_TIME_PATH  "./Log/FuncTimeout.log"   //���·�����Լ�Ҫ�����޸����
class CTimeCost
{
public:
	CTimeCost(int nMillionSecond, const char* pFunctionName, const char* pFileName, int nLine) 
	{
		m_nMillionSecond = nMillionSecond;
		sprintf_safe(m_szFunctionName, MAX_BUFF_100, "%s", pFunctionName);
		sprintf_safe(m_szFileName, MAX_BUFF_300, "%s", pFileName);
		m_nFileLine = nLine;
		TimeBegin();
	};

	~CTimeCost()
	{
		TimeEnd();
	};

	void TimeBegin()
	{
		m_lBegin = GetSystemTickCount();
	};

	void TimeEnd()
	{
		m_lEnd = GetSystemTickCount();
		long lTimeInterval = m_lEnd - m_lBegin;  //ת���ɺ���
		if(lTimeInterval >= (long)m_nMillionSecond)
		{
			char szLog[MAX_BUFF_1024];
			//��¼��־
			FILE* pFile = ACE_OS::fopen(ASSERT_TIME_PATH, "a+");
			if(pFile != NULL)
			{
				char szTimeNow[MAX_BUFF_50];
				time_t tNow = time(NULL);
				struct tm* tmNow = ACE_OS::localtime(&tNow);
				sprintf_safe(szTimeNow, MAX_BUFF_50, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);
				sprintf_safe(szLog, MAX_BUFF_1024, "[%s]dbTimeInterval more than (%d) < (%d), File(%s):FunName(%s):Line(%d).\n", szTimeNow, m_nMillionSecond, lTimeInterval, m_szFileName, m_szFunctionName, m_nFileLine);
				ACE_OS::fwrite(szLog, strlen(szLog), sizeof(char), pFile);
				ACE_OS::fclose(pFile);
			}
		}
	};

private:
	unsigned long GetSystemTickCount()
	{
#ifdef WIN32
		return GetTickCount();
#else
		struct timespec ts;

		clock_gettime(CLOCK_MONOTONIC, &ts);

		return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
	}

private:
	long         m_lBegin;
	long         m_lEnd;
	unsigned int m_nMillionSecond;
	char         m_szFunctionName[MAX_BUFF_100];
	char         m_szFileName[MAX_BUFF_300];
	int          m_nFileLine;
};

#define __TIMECOST(cost) CTimeCost timecost(cost, __FUNCTION__, __FILE__, __LINE__);

//************************************************************************
//add by freeeyes
//��ö��Լ���С�˵������ж�
//�����mainConfig�У�����Ϊû��Ҫ��define.h���ж�
//�ڿ�����Լ��ж��Ƿ�ת����С��
enum
{
	O32_LITTLE_ENDIAN = 0x03020100ul,
	O32_BIG_ENDIAN = 0x00010203ul,
	O32_PDP_ENDIAN = 0x01000302ul
};

static const union { unsigned char bytes[4]; uint32 value; } o32_host_order =
{ { 0, 1, 2, 3 } };

#define O32_HOST_ORDER (o32_host_order.value)

//����һ����64λ�����ε������ֽ����ת��
inline uint64 hl64ton(uint64 u8Data)   
{   
	if(O32_HOST_ORDER == O32_LITTLE_ENDIAN)
	{
		union { uint32 lv[2]; uint64 llv; } u;  
		u.lv[0] = htonl(u8Data >> 32);  
		u.lv[1] = htonl(u8Data & 0xFFFFFFFFULL);  
		return u.llv;   
	}
	else
	{
		//��������Ǵ�˾Ͳ���
		return u8Data;
	}
  
}

//����һ����64λ�����ε������ֽ����ת��
inline uint64 ntohl64(uint64 u8Data)   
{   
	if(O32_HOST_ORDER == O32_LITTLE_ENDIAN)
	{
		union { uint32 lv[2]; uint64 llv; } u;  
		u.llv = u8Data;  
		return ((uint64)ntohl(u.lv[0]) << 32) | (uint64)ntohl(u.lv[1]);
	}
	else
	{
		//��������Ǵ�˾Ͳ���
		return u8Data;
	}
}

//����һ����������֧���ַ����滻��Ŀǰ�Ȳ�����֧������
inline bool Replace_String(char* pText, uint32 u4Len, const char* pOld, const char* pNew)
{
	char* pTempSrc = new char(u4Len);

	ACE_OS::memcpy(pTempSrc, pText, u4Len);
	pTempSrc[u4Len - 1] = '\0';

	uint16 u2NewLen = (uint16)ACE_OS::strlen(pNew);

	char* pPos = ACE_OS::strstr(pTempSrc, pOld); 

	while(pPos)
	{
		//�������Ҫ���ǵ��ַ�������
		uint32 u4PosLen = (uint32)(pPos - pTempSrc);

		//�����ǰ���
		ACE_OS::memcpy(pText, pTempSrc, u4PosLen);
		pText[u4PosLen] = '\0';

		if(u4PosLen + u2NewLen >= (uint32)u4Len)
		{
			//�����м����
			delete[] pTempSrc;
			return false;		
		}
		else
		{
			//������ַ�
			ACE_OS::memcpy(&pText[u4PosLen], pNew, u2NewLen);
			pText[u4PosLen + u2NewLen] = '\0';

			//ָ������ƶ�	
			pTempSrc = 	pTempSrc + u4PosLen;

			//Ѱ����һ��͸�����ַ���
			pPos = ACE_OS::strstr(pTempSrc, pOld); 
		}

	}

	//�����м����
	delete[] pTempSrc;
	return true;
}

//�ͻ���IP��Ϣ
struct _ClientIPInfo
{
	char m_szClientIP[MAX_BUFF_50];   //�ͻ��˵�IP��ַ
	int  m_nPort;                     //�ͻ��˵Ķ˿�

	_ClientIPInfo()
	{
		m_szClientIP[0] = '\0';
		m_nPort         = 0;
	}

	_ClientIPInfo& operator = (const _ClientIPInfo& ar)
	{
		sprintf_safe(this->m_szClientIP, MAX_BUFF_50, "%s", ar.m_szClientIP);
		this->m_nPort = ar.m_nPort;
		return *this;
	}
};

//���ӱ���ӳ����Ϣ(����PSS_ClientManager����)
struct _ClientNameInfo
{
	char m_szName[MAX_BUFF_100];      //���ӱ��� 
	char m_szClientIP[MAX_BUFF_50];   //�ͻ��˵�IP��ַ
	int  m_nPort;                     //�ͻ��˵Ķ˿�
	int  m_nConnectID;                //����ID  
	int  m_nLog;                      //�Ƿ��¼��־

	_ClientNameInfo()
	{
		m_szName[0]     = '\0';
		m_szClientIP[0] = '\0';
		m_nPort         = 0;
		m_nConnectID    = 0;
		m_nLog          = 0;
	}

	_ClientNameInfo& operator = (const _ClientNameInfo& ar)
	{
		sprintf_safe(this->m_szName, MAX_BUFF_100, "%s", ar.m_szName);
		sprintf_safe(this->m_szClientIP, MAX_BUFF_50, "%s", ar.m_szClientIP);
		this->m_nPort      = ar.m_nPort;
		this->m_nConnectID = ar.m_nConnectID;
		this->m_nLog       = ar.m_nLog;
		return *this;
	}
};
typedef vector<_ClientNameInfo> vecClientNameInfo;

#endif
