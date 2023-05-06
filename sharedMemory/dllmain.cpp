// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

// The DLL code
#include <windows.h> 
#include <memory.h> 
#define SHMEMSIZE 165536 // size of shared memory block 
static LPVOID lpvMem = NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping
//static bool isInitialized = false;



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
                lpvMem = MapViewOfFile( 
                    hMapObject,     // object to map view of
                    FILE_MAP_WRITE, // read/write access
                    0,              // high offset:  map from
                    0,              // low offset:   beginning
                    0);             // default: map entire file
                if (lpvMem == NULL) 
                    return FALSE; 
                // Initialize memory if this is the first process
                if (fInit) 
                    memset(lpvMem, '\0', SHMEMSIZE);
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
 
__declspec(dllexport) void SetSharedMem(LPVOID lpvVar)
{ 
    #pragma comment(linker, "/EXPORT:" __FUNCTION__"=" __FUNCDNAME__)
    //LPWSTR lpszTmp;
    //DWORD dwCount=1;
    // Get the address of the shared memory block
 
    lpvMem = lpvVar; 
 
    // Copy the null-terminated string into shared memory
 
    /*while (*lpszBuf && dwCount<SHMEMSIZE) 
    {
        *lpszTmp++ = *lpszBuf++; 
        dwCount++;
    }
    *lpszTmp = '\0';*/ 
} 
 
// GetSharedMem gets the contents of the shared memory
 
__declspec(dllexport) void GetSharedMem(LPVOID lpvVar/*, DWORD cchSize*/)
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



/*
#include <Windows.h>
#include <Tchar.h>
#include <stdio.h>

static bool isInitialized = false;

__declspec(dllexport) bool setupSharedMemory(){
    return false;
}



BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{

    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        if(!isInitialized){
            isInitialized = true;
            if(!setupSharedMemory()){
                MessageBoxA(NULL, "Error Message", "Error creating the shared memory!", MB_OK);
            }
        }
        else{
            //_tprintf_s(TEXT("Already initialized"));
        }
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        //_tprintf(TEXT("Thread attached"));
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        //_tprintf(TEXT("Thread detached"));
        break;

    case DLL_PROCESS_DETACH:
        //_tprintf(TEXT("Process detatched"));
        if (lpvReserved != NULL)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}*/

/*

*/