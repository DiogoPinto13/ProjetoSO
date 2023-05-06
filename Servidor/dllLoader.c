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

boolean setMap(HANDLE hConsole, HANDLE dllHandle, DWORD velIniCarros, DWORD numFaixas){
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

    SharedMemory share;
    initGame(&share.game, numFaixas, velIniCarros);
    //casts the function to the correct type
    func((LPVOID)&share);

    //calls the function
    return TRUE;
    
}

boolean getMap(HANDLE hConsole, HANDLE dllHandle, SharedMemory *shared){
    GetSharedMemFunc func = (GetSharedMemFunc)GetProcAddress(dllHandle, "GetSharedMem");
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        return FALSE;
    }
    LPVOID share;
    //casts the function to the correct type
    func(&share);
    shared = (SharedMemory*)share;
    //calls the function
    return TRUE;
    
}

