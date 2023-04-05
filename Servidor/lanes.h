#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "cars.h"

typedef struct lane {
    Car* cars;
    boolean isReverse;
}Lane;

