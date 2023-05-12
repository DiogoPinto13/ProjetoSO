#pragma once

#include "utils.h"

#define DLL_NAME TEXT("sharedMemory.dll")
#define NAME_READ_SEMAPHORE TEXT("SEM�FORO_LER")
#define NAME_WRITE_SEMAPHORE TEXT("SEM�FORO_ESCREVER")
#define NAME_MUTEX_CIRCULAR_BUFFER TEXT("REDONDO")

#define DllImport __declspec( dllimport )
#define DllExport __declspec( dllexport )

DllImport void GetSharedMem(SharedMemory* shared);

DllImport void SetMessageBuffer(BufferCell* cell);

typedef void (*GetSharedMemFunc)(SharedMemory* lpvVar);

typedef void (*SetMessageBufferFunc)(BufferCell* cell);

HANDLE dllLoader(HANDLE hConsole);

BOOL getMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory *shared);