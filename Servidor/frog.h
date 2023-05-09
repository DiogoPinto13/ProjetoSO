#pragma once

#include "utils.h"

typedef struct frog {
    int x, y;
    TCHAR symbol;
    //HANDLE hFrog, hThread;
    int points, level, currentLifes;
    boolean isDead;
}Frog;

void initFrog(Frog frog, int numFrogs ,int startingRow);
