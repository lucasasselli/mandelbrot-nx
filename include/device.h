#ifndef __DEVICE__
#define __DEVICE__

#ifdef __SWITCH__
#include <switch.h>
#define RES(_str) "romfs:/" _str
#else
#define RES(_str) "./res/" _str
#endif

void deviceInit();

void deviceStop();

#endif