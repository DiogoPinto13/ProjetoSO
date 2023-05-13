#pragma once

#include "utils.h"
#include "console.h"
#include "dllLoader.h"

void readCommands(int *close, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer, HANDLE dllHandle);

void cmdPause(int time, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer, HANDLE dllHandle);

void cmdAddObstacle(int numLane, int x, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer, HANDLE dllHandle);

void cmdInvertLane(int numLane, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer, HANDLE dllHandle);

void cmdHelp();