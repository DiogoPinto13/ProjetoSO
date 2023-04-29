#pragma once

#include "utils.h"
#include "game.h"


HANDLE dllLoader(HANDLE hConsole);

boolean setMap(HANDLE hConsole, HANDLE hDLL, Game game);
