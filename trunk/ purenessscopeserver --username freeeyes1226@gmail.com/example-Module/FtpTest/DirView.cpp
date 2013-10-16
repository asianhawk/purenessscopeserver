#include "DirView.h"

CDirView::CDirView()
{
}

CDirView::~CDirView()
{
}

uint32 CDirView::GetDir( const char* pPath, vevFileInfo& objvevFileInfo )
{
	objvevFileInfo.clear();
	//�鿴��ǰ·���Ƿ�Ϊһ��Ŀ¼
	struct ACE_DIR* objopendir = ACE_OS::opendir(pPath);
	if(!objopendir)
	{
		//��ǰ��һ���ļ�����Ŀ¼
		return DIR_ERR_ISFILE;
	}
	else
	{
		//��Ŀ¼����Ŀ¼
		struct ACE_DIRENT * objreaddir = ACE_OS::readdir(objopendir);
		//�Թ�.��..
		while(objreaddir && objreaddir->d_name[0] == '.')
		{
			objreaddir = ACE_OS::readdir(objopendir);
		}

		while(objreaddir)     
		{
			char szFilePath[500] = {'\0'};
			sprintf_s(szFilePath, "%s%s", pPath, objreaddir->d_name);
			_FileInfo objFileInfo;
			sprintf_s(objFileInfo.m_szFileName, 500, "%s", objreaddir->d_name);

			struct ACE_DIR* objIsdir = ACE_OS::opendir(szFilePath);
			if(objIsdir == NULL)
			{
				objFileInfo.m_u4FileType = IS_FILE;
				objFileInfo.m_u4FileSize = (int)ACE_OS::filesize(szFilePath);
			}
			else
			{
				objFileInfo.m_u4FileType = IS_DIRRNT;
				objFileInfo.m_u4FileSize = 0;
			}

			objvevFileInfo.push_back(objFileInfo);

			objreaddir = ACE_OS::readdir_emulation(objopendir);
		}
		ACE_OS::closedir_emulation(objopendir);
	}


	return DIR_OK;
}

