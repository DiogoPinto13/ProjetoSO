#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

void readCommands(int *close);

void cmdToggleGameStatus();

void cmdRestartGame();

void cmdHelp();
