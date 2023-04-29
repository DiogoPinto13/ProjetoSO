#pragma once

//sexo orelhal

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>
#include <io.h>
#include <time.h>
#include "console.h"


#define LIFES 5
#define INITIAL_ROW	5
#define PIPE_NAME_UM TEXT("TUBO_DO_ESGOTO_UM")
#define PIPE_NAME_DOIS TEXT("TUBO_DO_ESGOTO_DOIS")
#define DLL_NAME TEXT("sharedMemory.dll")
#define BUFFER_SIZE 16


//buffer circular
/*
typedef struct {
    int buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
}CircularBuffer;

typedef struct{
    CircularBuffer buffer;
    HANDLE hMutexBuffer;
    HANDLE hSemRead;
    HANDLE hSemWrite;
    Game game;
}SharedMemory;
*/

