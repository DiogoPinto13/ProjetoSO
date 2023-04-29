#pragma once

#include "utils.h"

void readCommands(int *close, HANDLE hConsole);

void cmdPause(int time);

void cmdAddObstacle(int numLane, int x);

void cmdInvertLane(int numLane);

void cmdHelp();