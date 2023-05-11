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
    /*if((SetSharedMemFunc)GetProcAddress(dllHandle, "SetSharedMem") != NULL){
        _tprintf_s(_T("Got the address.\n"));
    }*/
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        _tprintf_s(_T("Error code: %d\n"), GetLastError());
        return FALSE;
    }

    SharedMemory *share = malloc(sizeof(SharedMemory));
    //ZeroMemory(&share, sizeof(SharedMemory));
    initGame(&share->game, numFaixas, velIniCarros);
    //LPBYTE binary_shared[sizeof(SharedMemory)];
    //memcpy(binary_shared, &share, sizeof(SharedMemory));
    int size = sizeof(SharedMemory);
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
    if(shared->game.estado == TRUE){
        _tprintf_s(_T("Esta merda é true."));
    }
    else{
        _tprintf_s(_T("Esta merda não é true."));
    }
    //calls the function
    return TRUE;
    
}

