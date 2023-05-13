#include "dllLoader.h"

HANDLE dllLoader(HANDLE hConsole){

    TCHAR dllPath[MAX_PATH] = { 0 }; //to store the full path of the dll
	//gets the full path
	if (!GetFullPathName(DLL_NAME, MAX_PATH, dllPath, NULL)) {
		errorMessage(_T("Error in getting full path of the dll"), hConsole);
		return NULL;
	}

    //loads the dll
    HANDLE dllHandle = LoadLibraryEx(DLL_NAME, NULL, 0);
    if (dllHandle == NULL) {
        errorMessage(_T("Error in loading the dll"), hConsole);
        return NULL;
    }

	return dllHandle;
    
}

BOOL setMap(HANDLE hConsole, HANDLE dllHandle, DWORD velIniCarros, DWORD numFaixas){
    //_tprintf_s(_T("DLL Handle: %d\n"), dllHandle);
    //gets the address of the function
    /*void* funcAddress = GetProcAddress(dllHandle, "setMap");
    if (funcAddress == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        return FALSE;
    }*/

    SetSharedMemFunc func = (SetSharedMemFunc)GetProcAddress(dllHandle, "SetSharedMem");
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        _tprintf_s(_T("Error code: %d\n"), GetLastError());
        return FALSE;
    }

    SharedMemory *share = malloc(sizeof(SharedMemory));
    //ZeroMemory(&share, sizeof(SharedMemory));
    initGame(&share->game, numFaixas, velIniCarros);

    //create semaphores and mutexes for the shared memory
    share->hMutexBuffer = CreateMutex(NULL, FALSE, NAME_MUTEX_CIRCULAR_BUFFER);
    share->hSemWrite = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NAME_WRITE_SEMAPHORE);
    share->hSemRead = CreateSemaphore(NULL, 0, BUFFER_SIZE, NAME_READ_SEMAPHORE);
    
    share->buffer.readIndex = 0;
    share->buffer.writeIndex = 0;
    
    //casts the function to the correct type

    func(share);
    //calls the function
    return TRUE;
    
}

BOOL updateMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory* shared) {
    
    SetSharedMemFunc func = (SetSharedMemFunc)GetProcAddress(dllHandle, "SetSharedMem");
    /*if((SetSharedMemFunc)GetProcAddress(dllHandle, "SetSharedMem") != NULL){
        _tprintf_s(_T("Got the address.\n"));
    }*/
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        _tprintf_s(_T("Error code: %d\n"), GetLastError());
        return FALSE;
    }

    func(shared);

    return TRUE;
}


BOOL getMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory *shared){
    GetSharedMemFunc func = (GetSharedMemFunc)GetProcAddress(dllHandle, "GetSharedMem");
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        return FALSE;
    }
    //casts the function to the correct type
    func(shared);
    //shared = (SharedMemory*)share;
    //calls the function
    return TRUE;
    
}

