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
    //Game game;
}SharedMemory;
/*
typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; //not sure yet 
    int numFrogs;
    Frog frogs[2];
    boolean estado;
}Game;
*/




int checkIfIsAlreadyRunning(TCHAR *processName);

