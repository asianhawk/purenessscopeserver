#ifndef _MESSAGEMANAGER_H
#define _MESSAGEMANAGER_H

#include "IMessageManager.h"
#include "Message.h"
#include <map>
#include <vector>

//�����޸�һ�£����һ�������Ӧһ��ģ���������Ƶġ�
//�����Ϊһ��������Զ�Ӧ���������Ĵ���ģ�飬�����ͱȽϺ��ˡ�
//add by freeeyes

using namespace std;

//��������ߵĸ�ʽ
struct _ClientCommandInfo
{
	CClientCommand* m_pClientCommand;             //��ǰ����ָ��
	uint32          m_u4Count;                     //��ǰ������õĴ���
	uint32          m_u4TimeCost;                  //��ǰ������ʱ������
	char            m_szModuleName[MAX_BUFF_100]; //����ģ������

	_ClientCommandInfo()
	{
		m_pClientCommand  = NULL;
		m_u4Count         = 0;
		m_u4TimeCost      = 0;
		m_szModuleName[0] = '\0';
	}
};

//һ����Ϣ���Զ�Ӧһ��CClientCommand*�����飬����Ϣ�����ʱ��ַ�����Щ������
class CClientCommandList
{
private:
	typedef vector<_ClientCommandInfo*> vecClientCommandList;
	vecClientCommandList m_vecClientCommandList;

public:
	CClientCommandList()
	{
	};

	~CClientCommandList()
	{
	};

	void AddClientCommand(CClientCommand* pClientCommand, const char* pMuduleName)
	{
		_ClientCommandInfo* pClientCommandInfo = new _ClientCommandInfo();
		if(NULL != pClientCommandInfo)
		{
			pClientCommandInfo->m_pClientCommand = pClientCommand;
			m_vecClientCommandList.push_back(pClientCommandInfo);
			sprintf_safe(pClientCommandInfo->m_szModuleName, MAX_BUFF_100, "%s", pMuduleName);
		}

	};

	//�������Ϊtrue��֤�������Ϣ�Ѿ�û�ж�Ӧ���Ҫ��Χmap�г�ȥ
	bool DelClientCommand(CClientCommand* pClientCommand)
	{
		vecClientCommandList::iterator b = m_vecClientCommandList.begin();
		vecClientCommandList::iterator e = m_vecClientCommandList.end();

		for(b; b!= e; b++)
		{
			if(pClientCommand == (CClientCommand* )(*b)->m_pClientCommand)
			{
				SAFE_DELETE((*b));
				m_vecClientCommandList.erase(b);
				break;
			}
		}

		if((int)m_vecClientCommandList.size() == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//�õ�����
	int GetCount()
	{
		return (int)m_vecClientCommandList.size();
	}

	//�õ�ָ��λ�õ�ָ��
	_ClientCommandInfo* GetClientCommandIndex(int nIndex)
	{
		if(nIndex >= (int)m_vecClientCommandList.size())
		{
			return NULL;
		}
		else
		{
			return (_ClientCommandInfo* )m_vecClientCommandList[nIndex];
		}
	}
};

class CMessageManager : public IMessageManager
{
public:
	CMessageManager(void);
	~CMessageManager(void);

	bool DoMessage(IMessage* pMessage, uint32& u4Len, uint16& u2CommandID);   //ִ������
	void Close();

	bool AddClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand, const char* pModuleName);   //ע������
	bool DelClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand);   //ж������

	int  GetCommandCount();                                            //�õ���ǰע������ĸ���
	CClientCommandList* GetClientCommandList(uint16 u2CommandID);      //�õ���ǰ�����ִ���б�

private:
	typedef map<uint16, CClientCommandList*> mapClientCommand;
	mapClientCommand m_mapClientCommand;
};

typedef ACE_Singleton<CMessageManager, ACE_Null_Mutex> App_MessageManager; 
#endif
