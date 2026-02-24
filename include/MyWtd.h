#ifndef __MYWTD_H__
#define __MYWTD_H__

#include "main.h"

void stopWatchdog();
void startWatchdog();
void refreshWatchdog();
void watchdogCallback(TimerHandle_t xTimer);

#endif