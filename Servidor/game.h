#pragma once

#include "utils.h"
#include "lanes.h"
#include "frog.h"
#include "points.h"


typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; //not sure yet 
    int numFrogs;
    Frog frogs[2];
    boolean estado;
}Game;

void initGame(Game* list, DWORD numFaixas, DWORD velIniCarros);

