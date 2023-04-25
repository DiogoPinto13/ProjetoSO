#pragma once

#include "utils.h"

typedef struct frog {
    int x, y;
    TCHAR symbol;
    int fdFIFOFROG, fdFIFOTHREAD;
    int points, level, lifes;
}Frog;
