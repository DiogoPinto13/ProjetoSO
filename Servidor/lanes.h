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
    Obstacle obstacle;  //assumimos que s� pode haver um obstaculo por faixa
    DWORD velCarros;
    BOOL isReverse;
    Frog *frogsOnLane;
}Lane;

typedef struct {
    int y;
    TCHAR caracter;
    BOOL isFinish;
}SpecialLane;  //starting and finishing lane

//[O,O,C,O,O]
//[O,O,O,C,O]

void initLanes(Lane *lanes, SpecialLane *specialLanes, DWORD numFaixas, DWORD velIniCarros);

BOOL moveCars(Lane* lane);

BOOL checkIfCarInFront(Lane *lane, int carPos);
