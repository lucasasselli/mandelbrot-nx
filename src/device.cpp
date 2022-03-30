#include "device.h"

void deviceInit()
{
#ifdef __SWITCH__
	romfsInit();
	socketInitializeDefault();
	nxlinkStdio();
#endif
}

void deviceStop()
{
#ifdef __SWITCH__
	socketExit();
	romfsExit();
#endif
}