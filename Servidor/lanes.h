#pragma once

#include "utils.h"
#include "cars.h"
#include "frog.h"

typedef struct {
    int x;
    TCHAR caracter;
}Obstacle;

typedef struct {
    Car cars[8];
    int numOfCars, numOfFrogs;
    int y;  //y para escrever os carros (consola)
    Obstacle obstacle;  //assumimos que só pode haver um obstaculo por faixa
    float velCarros;
    BOOL isReverse;
    Frog frogsOnLane[2];
}Lane;

typedef struct {
    int y;
    TCHAR caracter;
    BOOL isFinish;
}SpecialLane;  //starting and finishing lane

void initLanes(Lane *lanes, SpecialLane *specialLanes, DWORD numFaixas, float velIniCarros);

BOOL moveCars(Lane* lane, Frog* frogs, int numFrogs, int startingLaneRow, HANDLE hEventUpdateStartingLane);

BOOL checkIfCarInFront(Lane *lane, int carPos);
