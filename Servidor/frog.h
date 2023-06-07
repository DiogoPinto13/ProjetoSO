#pragma once

#include "utils.h"

typedef struct frog {
    int x, y;
    TCHAR symbol;
    //HANDLE hFrog, hThread;
    int points, level, currentLifes;
    BOOL isDead;
}Frog;

void initFrog(Frog *frogs, Frog *frog, int *numFrogs ,int startingRow);
