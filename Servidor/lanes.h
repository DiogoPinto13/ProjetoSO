#pragma once

#include "utils.h"
#include "cars.h"

typedef struct lane {
    Car* cars;
    boolean isReverse;
}Lane;

boolean initLanes(Lane *lanes, DWORD numFaixas, DWORD velIniCarros);

