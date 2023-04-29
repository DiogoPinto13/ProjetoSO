#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>
#include <io.h>
#include <time.h>

#include "console.h"

#define DLL_NAME TEXT("sharedMemory.dll")
#define BUFFER_SIZE 16
