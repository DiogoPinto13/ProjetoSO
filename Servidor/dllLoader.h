#pragma once

#include "utils.h"
#include "console.h"
#include "game.h"

#define BUFFER_SIZE 20
#define COMMAND_SIZE 64
#define DLL_NAME TEXT("sharedMemory.dll")
#define NAME_READ_SEMAPHORE TEXT("SEMÁFORO_LER")
#define NAME_WRITE_SEMAPHORE TEXT("SEMÁFORO_ESCREVER")
#define NAME_MUTEX_CIRCULAR_BUFFER TEXT("REDONDO")

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

DllImport void SetSharedMem(SharedMemory* shared);

DllImport void GetSharedMem(SharedMemory* shared);

DllImport void GetMessageBuffer(BufferCell* cell);

typedef void (*SetSharedMemFunc)(SharedMemory* lpvVar);

typedef void (*GetSharedMemFunc)(SharedMemory* lpvVar);

typedef void (*GetMessageBufferFunc)(BufferCell* cell);

//typedef double (*applyFactor)(double v);

HANDLE dllLoader(HANDLE hConsole);

BOOL setMap(HANDLE hConsole, HANDLE dllHandle, DWORD velIniCarros, DWORD numFaixas);

BOOL updateMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory* shared);

BOOL getMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory *shared);
