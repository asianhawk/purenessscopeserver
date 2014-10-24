// MessageService.h
// ������Ϣ������Ϣ���ɸ�������߼�������ȥִ��
// ���쵽�˹���ͼ��ݣ��о��¹���ĺ����ɣ�������д���������˼��
// ��ʱ���������ǵ�֧���£�PSS�Ż��ߵĸ�Զ��
// ���ϵ��кý���������ɼ���Ҷ�������ʹ���С�
// ��ʱ��ƽ������������
// add by freeeyes
// 2009-01-29

#include "MessageManager.h"
#ifdef WIN32
	#include "ProConnectHandle.h"
#else
	#include "ConnectHandler.h"
#endif

bool Delete_CommandInfo(_ClientCommandInfo* pClientCommandInfo)
{
	return pClientCommandInfo->m_u4CurrUsedCount == 0;
}

CMessageManager::CMessageManager(void)
{
}

CMessageManager::~CMessageManager(void)
{
	OUR_DEBUG((LM_INFO, "[CMessageManager::~CMessageManager].\n"));
	//Close();
}

bool CMessageManager::DoMessage(ACE_Time_Value& tvBegin, IMessage* pMessage, uint16& u2CommandID, uint32& u4TimeCost, uint16& u2Count)
{
	if(NULL == pMessage)
	{
		OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage] pMessage is NULL.\n"));
		return false;
	}

	//�Ÿ���Ҫ�̳е�ClientCommand��ȥ����
	bool bDeleteFlag = true;         //���ݰ��Ƿ������ɾ��
	//OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage]u2CommandID = %d.\n", u2CommandID));

	CClientCommandList* pClientCommandList = GetClientCommandList(u2CommandID);
	if(pClientCommandList != NULL)
	{
		int nCount = pClientCommandList->GetCount();
		for(int i = 0; i < nCount; i++)
		{
			_ClientCommandInfo* pClientCommandInfo = pClientCommandList->GetClientCommandIndex(i);
			if(NULL != pClientCommandInfo && pClientCommandInfo->m_u1State == 0)
			{
				//OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage]u2CommandID = %d Begin.\n", u2CommandID));
				//����ָ��¼���������
				pClientCommandInfo->m_pClientCommand->DoMessage(pMessage, bDeleteFlag);
				ACE_Time_Value tvCost =  ACE_OS::gettimeofday() - tvBegin;
				u4TimeCost =  (uint32)tvCost.msec();

				//��¼������ô���
				u2Count++;
				//OUR_DEBUG((LM_ERROR, "[CMessageManager::DoMessage]u2CommandID = %d End.\n", u2CommandID));
				
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
			_ClientCommandInfo* pClientCommandInfo = pClientCommandList->AddClientCommand(pClientCommand, pModuleName);
			if(NULL != pClientCommandInfo) 
			{
				//���������ID
				pClientCommandInfo->m_u2CommandID = u2CommandID;
				
				//��ӵ�ģ������
				string strModule = pModuleName;
				mapModuleClient::iterator f = m_mapModuleClient.find(strModule);
				if(f == m_mapModuleClient.end())
				{
					//�Ҳ����������µ�ģ����Ϣ
					_ModuleClient* pModuleClient = new _ModuleClient();
					if(NULL != pModuleClient)
					{
						pModuleClient->m_vecClientCommandInfo.push_back(pClientCommandInfo);
						m_mapModuleClient.insert(mapModuleClient::value_type(strModule, pModuleClient));
					}
				}
				else
				{
					//�ҵ��ˣ���ӽ�ȥ
					_ModuleClient* pModuleClient = (_ModuleClient* )f->second;
					if(NULL != pModuleClient)
					{
						pModuleClient->m_vecClientCommandInfo.push_back(pClientCommandInfo);
					}
				}
				OUR_DEBUG((LM_ERROR, "[CMessageManager::AddClientCommand] u2CommandID = %d Add OK***.\n", u2CommandID));
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}

	}

	//û���ҵ����е�����½�һ���б�
	CClientCommandList* pClientCommandList = new CClientCommandList();
	if(NULL == pClientCommandList)
	{
		return false;
	}
	else
	{
		_ClientCommandInfo* pClientCommandInfo = pClientCommandList->AddClientCommand(pClientCommand, pModuleName);
		if(NULL != pClientCommandInfo)
		{
			//���������ID
			pClientCommandInfo->m_u2CommandID = u2CommandID;			
			
			//��ӵ�ģ������
			string strModule = pModuleName;
			mapModuleClient::iterator f = m_mapModuleClient.find(strModule);
			if(f == m_mapModuleClient.end())
			{
				//�Ҳ����������µ�ģ����Ϣ
				_ModuleClient* pModuleClient = new _ModuleClient();
				if(NULL != pModuleClient)
				{
					pModuleClient->m_vecClientCommandInfo.push_back(pClientCommandInfo);
					m_mapModuleClient.insert(mapModuleClient::value_type(strModule, pModuleClient));
				}
			}
			else
			{
				//�ҵ��ˣ���ӽ�ȥ
				_ModuleClient* pModuleClient = (_ModuleClient* )f->second;
				if(NULL != pModuleClient)
				{
					pModuleClient->m_vecClientCommandInfo.push_back(pClientCommandInfo);
				}
			}
		}

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
				SAFE_DELETE(pClientCommandList);
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

bool CMessageManager::UnloadModuleCommand(const char* pModuleName, uint8 u1State)
{
	string strModuleName = pModuleName;

	mapModuleClient::iterator f = m_mapModuleClient.find(strModuleName);
	if(f != m_mapModuleClient.end())
	{
		//����ɾ�����в����Ӧ��Commandӳ��
		_ModuleClient* pModuleClient = (_ModuleClient* )f->second;
		if(NULL != pModuleClient)
		{
			//�Ӳ��Ŀǰע������������ҵ����иò������Ϣ��һ�����ͷ�
			for(uint32 u4Index = 0; u4Index < (uint32)pModuleClient->m_vecClientCommandInfo.size(); u4Index++)
			{
				_ClientCommandInfo* pClientCommandInfo = pModuleClient->m_vecClientCommandInfo[u4Index];
				if(NULL != pClientCommandInfo)
				{
					uint16 u2CommandID = pClientCommandInfo->m_u2CommandID;
					CClientCommandList* pCClientCommandList = GetClientCommandList(u2CommandID);
					if(NULL != pCClientCommandList)
					{
						for(int i = 0; i < pCClientCommandList->GetCount(); i++)
						{
							//�ҵ��Ǹ�Ψһ
							if(pCClientCommandList->GetClientCommandIndex(i) == pClientCommandInfo)
							{
								//�ҵ��ˣ��ͷ�֮
								pCClientCommandList->DelClientCommand(pClientCommandInfo->m_pClientCommand);
								break;
							}
						}
					}
				}
			}

			//���ɾ��ʵ��ģ���CommandInfo�Ĺ�ϵ
			SAFE_DELETE(pModuleClient);
			m_mapModuleClient.erase(f);
		}

		//�����Ƿ�Ҫ���¼���
		if(u1State == 2)
		{
			_ModuleInfo* pModuleInfo = App_ModuleLoader::instance()->GetModuleInfo(pModuleName);
			if(NULL != pModuleInfo)
			{
				//��ȡ������Ϣ
				string strModuleN    = pModuleInfo->strModuleName;
				string strModulePath = pModuleInfo->strModulePath;

				//���¼���
				App_ModuleLoader::instance()->LoadModule(strModulePath.c_str(), strModuleN.c_str());
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

int CMessageManager::GetCommandCount()
{
	return (int)m_mapClientCommand.size();
}

void CMessageManager::Close()
{
	//��رյ�������
	for(mapClientCommand::iterator b = m_mapClientCommand.begin(); b != m_mapClientCommand.end(); b++)
	{
		 CClientCommandList* pClientCommandList = (CClientCommandList* )b->second;
		 SAFE_DELETE(pClientCommandList);
	}

	for(mapModuleClient::iterator Mb = m_mapModuleClient.begin(); Mb != m_mapModuleClient.end(); Mb++)
	{
		_ModuleClient* pModuleClient = (_ModuleClient* )Mb->second;
		SAFE_DELETE(pModuleClient);
	}

	m_mapClientCommand.clear();
}

mapModuleClient* CMessageManager::GetModuleClient()
{
	return &m_mapModuleClient;
}

uint32 CMessageManager::GetWorkThreadCount()
{
	return App_MessageServiceGroup::instance()->GetWorkThreadCount();
}

uint32 CMessageManager::GetWorkThreadByIndex(uint32 u4Index)
{
	return App_MessageServiceGroup::instance()->GetWorkThreadIDByIndex(u4Index);
}

