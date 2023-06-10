// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

// The DLL code
#include <windows.h> 
#include <memory.h> 
#include <stdlib.h>

#define SHMEMSIZE 3672 // size of shared memory block 
#define BUFFER_SIZE 20
#define COMMAND_SIZE 64
//static bool isInitialized = false;

typedef struct {
    int x;
    TCHAR caracter;
}Obstacle;
typedef struct car {
    int x;
    TCHAR symbol;
}Car;
typedef struct frog {
    int x, y;
    TCHAR symbol;
    HANDLE hNamedPipeMovement, hNamedPipeMap;
    int points, level, currentLifes;
    BOOL isDead;
}Frog;
typedef struct {
    Car cars[8];
    int numOfCars, numOfFrogs;
    int y;  //y para escrever os carros (consola)
    Obstacle obstacle;  //assumimos que só pode haver um obstaculo por faixa
    float velCarros;
    BOOL isReverse;
    Frog **frogsOnLane;
}Lane;
typedef struct {
    int y;
    TCHAR caracter;
    BOOL isFinish;
}SpecialLane;  //starting and finishing lane
typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; //not sure yet 
    int numFrogs, numFaixas;
    Frog frogs[2];
    BOOL estado;
}Game;

//shared memory structures
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
    HANDLE hMutexDLL;
    HANDLE hMutexBuffer;
    HANDLE hSemRead;    //semaforos
    HANDLE hSemWrite;   //semaforos
    Game game;
}SharedMemory;


static SharedMemory* lpvMem; //= NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping

// The DLL entry-point function sets up shared memory using a 
// named file-mapping object.  

BOOL WINAPI DllMain(HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{ 
    BOOL fInit, fIgnore;
    switch (fdwReason) 
    { 
        // DLL load due to process initialization or LoadLibrary
          case DLL_PROCESS_ATTACH: 
              //MessageBox(NULL, TEXT("A server is already running"), TEXT("Error"), MB_OK);
            //if(!isInitialized){
                //isInitialized = true;
                hMapObject = CreateFileMapping( 
                    INVALID_HANDLE_VALUE,   // use paging file
                    NULL,                   // default security attributes
                    PAGE_READWRITE,         // read/write access
                    0,                      // size: high 32-bits
                    sizeof(SharedMemory),   // size: low 32-bits
                    TEXT("dllmemfilemap")); // name of map object
                if (hMapObject == NULL) 
                    return FALSE; 
                // The first process to attach initializes memory
                fInit = (GetLastError() != ERROR_ALREADY_EXISTS);
                // Get a pointer to the file-mapped shared memory
                lpvMem = (SharedMemory*)MapViewOfFile( 
                    hMapObject,     // object to map view of
                    FILE_MAP_ALL_ACCESS, // read/write access
                    0,              // high offset:  map from
                    0,              // low offset:   beginning
                    sizeof(SharedMemory));             // default: map entire file
                if (lpvMem == NULL) 
                    return FALSE; 
            //} 
                if (fInit) 
                    memset(lpvMem, '\0', SHMEMSIZE);

            // Create a named file mapping object
            break;
// The attached process creates a new thread
        case DLL_THREAD_ATTACH: 
            break; 
        // The thread of the attached process terminates
        case DLL_THREAD_DETACH: 
            break; 
        // DLL unload due to process termination or FreeLibrary
        case DLL_PROCESS_DETACH: 
            //MessageBox(NULL, TEXT(""), TEXT("Error"), MB_OK);

            // Unmap shared memory from the process's address space
            fIgnore = UnmapViewOfFile(lpvMem);
            // Close the process's handle to the file-mapping object
            fIgnore = CloseHandle(hMapObject);
            break;
        default:
        break;
    }
    return TRUE;
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);
}
// The export mechanism used here is the __declspec(export)
// method supported by Microsoft Visual Studio, but any
// other export method supported by your development
// environment may be substituted.
 
// SetSharedMem sets the contents of the shared memory 
 
__declspec(dllexport) void SetSharedMem(SharedMemory* lpvVar)
{ 
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)

    CopyMemory(lpvMem, lpvVar, sizeof(SharedMemory));
} 
 
// GetSharedMem gets the contents of the shared memory
 
__declspec(dllexport) void GetSharedMem(SharedMemory* lpvVar)
{ 
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)

    CopyMemory(lpvVar, lpvMem, sizeof(SharedMemory));
}

//vai escrever mensagens no circular buffer
__declspec(dllexport) void SetMessageBuffer(BufferCell *cell) {
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
    
    WaitForSingleObject(lpvMem->hMutexBuffer, INFINITE);

    
    //vamos copiar a variavel cel para a memoria partilhada (para a posição de escrita)
    CopyMemory(&lpvMem->buffer.buffer[lpvMem->buffer.writeIndex], cell, sizeof(BufferCell));

    lpvMem->buffer.writeIndex++;

    //se apos o incremento a posicao de escrita chegar ao fim, tenho de voltar ao inicio
    if (lpvMem->buffer.writeIndex == BUFFER_SIZE)
        lpvMem->buffer.writeIndex = 0;

    //libertamos o mutex
    ReleaseMutex(lpvMem->hMutexBuffer);

}

//vai ler mensagens no circular buffer
__declspec(dllexport) void GetMessageBuffer(BufferCell* cell) {
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)

    WaitForSingleObject(lpvMem->hMutexBuffer, INFINITE);

    CopyMemory(cell, &lpvMem->buffer.buffer[lpvMem->buffer.readIndex], sizeof(BufferCell));
    lpvMem->buffer.readIndex++;

    if (lpvMem->buffer.readIndex == BUFFER_SIZE)
        lpvMem->buffer.readIndex = 0;

    ReleaseMutex(lpvMem->hMutexBuffer);

}

