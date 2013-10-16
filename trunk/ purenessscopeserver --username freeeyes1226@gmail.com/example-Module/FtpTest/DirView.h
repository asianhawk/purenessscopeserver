#ifndef _DIRVIEW_H
#define _DIRVIEW_H

#include "define.h"
#include <ace/Dirent.h>

#define DIR_OK             0
#define DIR_ERR_ISFILE     1
#define DIR_ERR_ISNOEXIST  2

#define IS_FILE            0
#define IS_DIRRNT          1

//文件列表结构
struct _FileInfo
{
	char   m_szFileName[MAX_BUFF_500];
	uint32 m_u4FileType;
	uint32 m_u4FileSize;

	_FileInfo()
	{
		m_szFileName[0] = '\0';
		m_u4FileType     = 0;
		m_u4FileSize     = 0;
	}
};

typedef vector<_FileInfo> vevFileInfo;

class CDirView
{
public:
	CDirView();
	~CDirView();

	uint32 GetDir(const char* pPath, vevFileInfo& objvevFileInfo);

};

#endif
