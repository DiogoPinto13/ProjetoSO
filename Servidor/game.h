#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
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

}Game;
