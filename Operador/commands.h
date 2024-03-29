#pragma once

#include "utils.h"
#include "console.h"
#include "dllLoader.h"

TCHAR* readCommands(int *close, int numActiveLanes, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL);

void cmdPause(int time, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL);

void cmdAddObstacle(int numLane, int x, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL);

void cmdInvertLane(int numLane, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL);

void cmdHelp();