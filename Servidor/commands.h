#pragma once

#include "utils.h"
#include "dllLoader.h"
#include "game.h"
#include "console.h"

void readCommands(int *close, HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, HANDLE hEventUpdateStartingLane, HANDLE hEventUpdateFinishingLane, DWORD numFaixas, DWORD velIniCarros);

void cmdToggleGameStatus(HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, DWORD numFaixas);

void cmdRestartGame(HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, HANDLE hEventUpdateStartingLane, HANDLE hEventUpdateFinishingLane, DWORD numFaixas, DWORD velIniCarros);

void cmdHelp();
