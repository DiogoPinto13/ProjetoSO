#pragma once

#include "utils.h"
#include "game.h"
#define BUFFER_SIZE 20
#define COMMAND_SIZE 64
#define DLL_NAME TEXT("sharedMemory.dll")

#define DllImport __declspec( dllimport )
#define DllExport __declspec( dllexport )

typedef struct {
    TCHAR command[COMMAND_SIZE];
    int param1, param2;
}BufferCell;

typedef struct {
    BufferCell buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
}CircularBuffer;

typedef struct{
    CircularBuffer buffer;
    HANDLE hMutexBuffer;
    HANDLE hSemRead;    //semaforos
    HANDLE hSemWrite;   //semaforos
    Game game;
}SharedMemory;

DllImport void SetSharedMem(LPVOID shared);

DllImport void GetSharedMem(LPVOID shared);


typedef void (*GetSharedMemFunc)(LPVOID lpvVar);

typedef void (*SetSharedMemFunc)(LPVOID lpvVar);


//typedef double (*applyFactor)(double v);

HANDLE dllLoader(HANDLE hConsole);

boolean setMap(HANDLE hConsole, HANDLE dllHandle, DWORD velIniCarros, DWORD numFaixas);

boolean getMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory *shared);
