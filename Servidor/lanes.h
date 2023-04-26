#pragma once

#include "utils.h"
#include "cars.h"

typedef struct {
    Car cars[8];
    int numOfCars;
    int y;  //y para escrever os carros (consola)
    DWORD velCarros;
    boolean isReverse;
}Lane;

typedef struct {
    int y;
    TCHAR caracter;
    boolean isFinish;
}SpecialLane;  //starting and finishing lane

//[O,O,C,O,O]
//[O,O,O,C,O]

void initLanes(Lane *lanes, SpecialLane *specialLanes, DWORD numFaixas, DWORD velIniCarros);

