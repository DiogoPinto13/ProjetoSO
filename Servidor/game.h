#pragma once

#include "utils.h"
#include "lanes.h"
#include "frog.h"
#include "points.h"

enum EstadoJogo {
    ATIVO,
    SUSPENSO
};

typedef struct game {
    Lane* lanes;
    DWORD timer; //not sure yet 
    int numFrogs;
    Frog* frogs;
    struct game *next;
    struct game *behind;
}Game;

Game* initNode();

Game* newNode(Game *list, int *gameNum);

void deleteNode(Game *dontDelNode, Game *delNode, int *gameNum);
