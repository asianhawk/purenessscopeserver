#include "UserValidManager.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	CUserValidManager objUserValidManager;
	objUserValidManager.Init();

	while(true)
	{
		OUR_DEBUG((LM_INFO, "[Watch]Valid Begin.\n"));
		objUserValidManager.Check_File2Memory();
		OUR_DEBUG((LM_INFO, "[Watch]Valid End.\n"));
		ACE_Time_Value tvSleep(60, 0);
		ACE_OS::sleep(tvSleep);
	}

	return 0;
}
