// MessageService.h
// 处理消息，将消息分派给具体的逻辑处理类去执行
// 今天到了国家图书馆，感觉新馆真的很气派，在这里写代码很有意思。
// add by freeeyes
// 2009-01-29

#include "MessageManager.h"

CMessageManager::CMessageManager(void)
{
}

CMessageManager::~CMessageManager(void)
{
	//Close();
}

bool CMessageManager::DoMessage(IMessage* pMessage, uint32& u4Len, uint16& u2CommandID)
{
	ACE_Time_Value tvDisposeCost;
	if(NULL == pMessage)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage] pMessage is NULL.\n"));
		return false;
	}

	//放给需要继承的ClientCommand类去处理
	bool bDeleteFlag = true;         //数据包是否用完后删除
	//OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage]u2Len = %d u2CommandID = %d pMessage = 0x%08x.\n",u4Len, u2CommandID, (int)pMessage));

	CClientCommandList* pClientCommandList = GetClientCommandList(u2CommandID);
	if(pClientCommandList != NULL)
	{
		int nCount = pClientCommandList->GetCount();
		for(int i = 0; i < nCount; i++)
		{
			_ClientCommandInfo* pClientCommandInfo = pClientCommandList->GetClientCommandIndex(i);
			if(NULL != pClientCommandInfo)
			{
				ACE_Time_Value tvDisposeBegin(ACE_OS::gettimeofday());
				pClientCommandInfo->m_pClientCommand->DoMessage(pMessage, bDeleteFlag);
				ACE_Time_Value tvDisposeEnd(ACE_OS::gettimeofday());
				tvDisposeCost = tvDisposeEnd - tvDisposeBegin;
				pClientCommandInfo->m_u4Count++;
				pClientCommandInfo->m_u4TimeCost += tvDisposeCost.msec();
			}
		}		
	}

	if(true == bDeleteFlag)
	{
		App_MessagePool::instance()->Delete((CMessage* )pMessage);
	}

	return true;
}

CClientCommandList* CMessageManager::GetClientCommandList(uint16 u2CommandID)
{
	mapClientCommand::iterator f = m_mapClientCommand.find(u2CommandID);
	if(f != m_mapClientCommand.end())
	{
		CClientCommandList* pClientCommandList = (CClientCommandList* )f->second;
		return pClientCommandList;
	}
	else
	{
		//OUR_DEBUG((LM_ERROR, "[CMessageManager::GetClientCommand] u2CommandID = %d 0x%08x Add OK.\n", u2CommandID, &m_mapClientCommand));
		return NULL;
	}
}

bool CMessageManager::AddClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand, const char* pModuleName)
{
	if(NULL == pClientCommand)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageManager::AddClientCommand] u2CommandID = %d pClientCommand is NULL.\n", u2CommandID));
		return false;
	}

	mapClientCommand::iterator f = m_mapClientCommand.find(u2CommandID);
	if(f != m_mapClientCommand.end())
	{
		CClientCommandList* pClientCommandList = (CClientCommandList* )f->second;
		if(NULL != pClientCommandList)
		{
			pClientCommandList->AddClientCommand(pClientCommand, pModuleName);
		}
		else
		{
			return false;
		}

	}

	CClientCommandList* pClientCommandList = new CClientCommandList();
	if(NULL == pClientCommandList)
	{
		return false;
	}
	else
	{
		pClientCommandList->AddClientCommand(pClientCommand, pModuleName);
		m_mapClientCommand.insert(mapClientCommand::value_type(u2CommandID, pClientCommandList));
		OUR_DEBUG((LM_ERROR, "[CMessageManager::AddClientCommand] u2CommandID = %d Add OK***.\n", u2CommandID));
		return true;
	}
}

bool CMessageManager::DelClientCommand(uint16 u2CommandID, CClientCommand* pClientCommand)
{
	mapClientCommand::iterator f = m_mapClientCommand.find(u2CommandID);
	if(f != m_mapClientCommand.end())
	{
		CClientCommandList* pClientCommandList = (CClientCommandList* )f->second;
		if(NULL != pClientCommandList)
		{
			if(true == pClientCommandList->DelClientCommand(pClientCommand))
			{
				m_mapClientCommand.erase(f);
			}
		}

		OUR_DEBUG((LM_ERROR, "[CMessageManager::DelClientCommand] u2CommandID = %d Del OK.\n", u2CommandID));
		return true;
	}
	else
	{
		OUR_DEBUG((LM_ERROR, "[CMessageManager::DelClientCommand] u2CommandID = %d is not exist.\n", u2CommandID));
		return false;
	}
}

int CMessageManager::GetCommandCount()
{
	return (int)m_mapClientCommand.size();
}

void CMessageManager::Close()
{
	mapClientCommand::iterator b = m_mapClientCommand.begin();
	mapClientCommand::iterator e = m_mapClientCommand.end();

	for(b; b != e; b++)
	{
		 CClientCommandList* pClientCommandList = (CClientCommandList* )b->second;
		 SAFE_DELETE(pClientCommandList);
	}

	m_mapClientCommand.clear();
}

