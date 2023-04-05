#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct car {
    int x, y;
    TCHAR symbol;
}Car;