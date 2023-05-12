#include "utils.h"
#include "dllLoader.h"


// Load a DLL from a file path

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