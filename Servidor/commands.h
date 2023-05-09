#pragma once

#include "utils.h"
#include "console.h"

void readCommands(int *close, HANDLE hConsole);

void cmdToggleGameStatus();

void cmdRestartGame();

void cmdHelp();
