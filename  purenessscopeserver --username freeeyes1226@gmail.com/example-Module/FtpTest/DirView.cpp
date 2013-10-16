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
	//查看当前路径是否为一个目录
	struct ACE_DIR* objopendir = ACE_OS::opendir(pPath);
	if(!objopendir)
	{
		//当前是一个文件不是目录
		return DIR_ERR_ISFILE;
	}
	else
	{
		//是目录，打开目录
		struct ACE_DIRENT * objreaddir = ACE_OS::readdir(objopendir);
		//略过.和..
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

