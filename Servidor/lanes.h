#pragma once

#include "utils.h"
#include "cars.h"

typedef struct lane {
    Car cars[8];
    int numOfCars;
    int y;  //y para escrever os carros
    DWORD velCarros;
    boolean isReverse;
}Lane;

//[O,O,C,O,O]
//[O,O,O,C,O]

void initLanes(Lane *lanes, DWORD numFaixas, DWORD velIniCarros);

