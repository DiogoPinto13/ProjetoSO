// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

// The DLL code
#include <windows.h> 
#include <memory.h> 
#define SHMEMSIZE 4096 // size of shared memory block 
#define BUFFER_SIZE 20
//static bool isInitialized = false;

typedef struct {
    int y;
    TCHAR caracter;
    BOOL isFinish;
}SpecialLane;  //starting and finishing lane
typedef struct {
    int x;
    TCHAR caracter;
}Obstacle;
typedef struct frog {
    int x, y;
    TCHAR symbol;
    //HANDLE hFrog, hThread;
    int points, level, currentLifes;
}Frog;
typedef struct car {
    int x;
    TCHAR symbol;
}Car;
typedef struct {
    Car cars[8];
    int numOfCars;
    int y;  //y para escrever os carros (consola)
    Obstacle obstacle;  //assumimos que s� pode haver um obstaculo por faixa
    DWORD velCarros;
    BOOL isReverse;
}Lane;
typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; //not sure yet 
    int numFrogs;
    Frog frogs[2];
    BOOL estado;
}Game;
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

static SharedMemory* lpvMem = NULL;      // pointer to shared memory
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
                    SHMEMSIZE,              // size: low 32-bits
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
                    0);             // default: map entire file
                if (lpvMem == NULL) 
                    return FALSE; 
            //}

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
 
__declspec(dllexport) void SetSharedMem(LPBYTE lpvVar)
{ 
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
    //SharedMemory* lpszTmp;
    //DWORD dwCount=1;
 
    // Get the address of the shared memory block
 
    //lpszTmp = (SharedMemory*) lpvMem;
    lpvMem = (SharedMemory*) lpvVar;
 
    // Copy the null-terminated string into shared memory
 
    /*while (lpvVar != NULL && dwCount<SHMEMSIZE) 
    {
        lpszTmp = lpvVar;
        dwCount++;
    }*/
} 
 
// GetSharedMem gets the contents of the shared memory
 
__declspec(dllexport) void GetSharedMem(SharedMemory* lpvVar)
{ 
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
    //LPWSTR lpszTmp;
 
    // Get the address of the shared memory block
    lpvVar = lpvMem;
 
    //lpszTmp = (LPWSTR) lpvMem;
 
    // Copy from shared memory into the caller's buffer
 
    /*while (*lpszTmp && --cchSize) 
        *lpszBuf++ = *lpszTmp++; 
    *lpszBuf = '\0'; */
}