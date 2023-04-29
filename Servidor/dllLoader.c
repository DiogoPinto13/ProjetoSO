#include "dllLoader.h"

HANDLE dllLoader(HANDLE hConsole){

    char dllPath[MAX_PATH] = { 0 }; //to store the full path of the dll
	//gets the full path
	if (!GetFullPathName(DLL_NAME, MAX_PATH, dllPath, NULL)) {
		errorMessage("Error in getting full path of the dll", hConsole);
		return NULL;
	}

    //loads the dll
    HANDLE dllHandle = LoadLibraryEx(DLL_NAME, NULL, NULL);
    if (dllHandle == NULL) {
        errorMessage("Error in loading the dll", hConsole);
        return NULL;
    }

	return dllHandle;
    
}

boolean setMap(HANDLE hConsole, HANDLE dllHandle, Game game){

    //gets the address of the function
    void* funcAddress = GetProcAddress(dllHandle, "setMap");
    if (funcAddress == NULL) {
        errorMessage("Error in getting the address of the function", hConsole);
        return FALSE;
    }

    //casts the function to the correct type
    boolean (*setMap)(Game) = (boolean (*)(Game))funcAddress;

    //calls the function
    return setMap(game);
    
}

