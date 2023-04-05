#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct frog {
    int x, y;
    TCHAR symbol;
    int points, level, lifes;
}Frog;
