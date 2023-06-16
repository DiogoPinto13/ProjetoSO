#pragma once

#include "utils.h"
#include "communications.h"

typedef struct frog {
    int x, y;
    TCHAR symbol;
    HANDLE hNamedPipeMovement, hNamedPipeMap;
    int points, level, currentLifes;
    BOOL isDead;
}Frog;

void initFrog(Frog *frogs, Frog *frog, int *numFrogs ,int startingRow);

BOOL resetFrog(Frog* frog, Frog* frogs, int numFrogs, int specialLaneStart);

void removeFrog(Frog *frog);
