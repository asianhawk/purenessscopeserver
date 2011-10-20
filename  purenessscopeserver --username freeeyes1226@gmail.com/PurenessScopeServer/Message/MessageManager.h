#ifndef _MESSAGEMANAGER_H
#define _MESSAGEMANAGER_H

#include "IMessageManager.h"
#include "Message.h"
#include <map>
#include <vector>

//这里修改一下，如果一个命令对应一个模块是有限制的。
//这里改为一个信令可以对应任意数量的处理模块，这样就比较好了。
//add by freeeyes

using namespace std;

//命令订阅者者的格式
struct _ClientCommandInfo
{
	CClientCommand* m_pClientCommand;             //当前命令指针
	uint32          m_u4Count;                     //当前命令被调用的次数
	uint32          m_u4TimeCost;                  //当前命令总时间消耗
	char            m_szModuleName[MAX_BUFF_100]; //所属模块名称

	_ClientCommandInfo()
	{
		m_pClientCommand  = NULL;
		m_u4Count         = 0;
		m_u4TimeCost      = 0;
		m_szModuleName[0] = '\0';
	}
};

//一个消息可以对应一个CClientCommand*的数组，当消息到达的时候分发给这些订阅者
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

	//如果返回为true，证明这个消息已经没有对应项，需要外围map中除去
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

	//得到个数
	int GetCount()
	{
		return (int)m_vecClientCommandList.size();
	}

	//得到指定位置的指针
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

	bool DoMessage(IMessage* pMessage, uint32& u4Len, uint16& u2CommandID);   //执行命令
	void Close();

	bool AddClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand, const char* pModuleName);   //注册命令
	bool DelClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand);   //卸载命令

	int  GetCommandCount();                                            //得到当前注册命令的个数
	CClientCommandList* GetClientCommandList(uint16 u2CommandID);      //得到当前命令的执行列表

private:
	typedef map<uint16, CClientCommandList*> mapClientCommand;
	mapClientCommand m_mapClientCommand;
};

typedef ACE_Singleton<CMessageManager, ACE_Null_Mutex> App_MessageManager; 
#endif
