#pragma once

#include "utils.h"
#include "console.h"
#include "dllLoader.h"

void readCommands(int *close, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer);

void cmdPause(int time, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer);

void cmdAddObstacle(int numLane, int x, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer);

void cmdInvertLane(int numLane, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer);

void cmdHelp();