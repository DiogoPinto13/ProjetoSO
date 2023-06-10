#pragma once

#include "utils.h"
#include "lanes.h"
#include "frog.h"


typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; 
    int numFrogs, numFaixas;
    Frog frogs[2];
    BOOL estado;
}Game;

void initGame(Game* list, DWORD numFaixas, DWORD velIniCarros);

void passToTheNextLevel(Game* game, Frog* frog);

enum ResponseMovement moveFrog(Game* game, Frog* frog, enum Movement action);

void removeFromLane(Lane* lane, Frog* frog);

void addToLane(Lane* lane, Frog* frog);
