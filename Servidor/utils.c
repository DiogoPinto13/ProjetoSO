#include "utils.h"

typedef struct threadData {
    //thread data
    BOOL isMultiplayer;
}TDADOS;

typedef struct threadInfo {
    HANDLE handle;
    TDADOS *data;
}TINFO;

int checkIfIsAlreadyRunning(TCHAR *processName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int counter = 0;
    
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)){
        CloseHandle(hProcessSnap);
        return(FALSE);
    }

    do {
        if (!wcscmp(pe32.szExeFile, processName)) {
            counter++;
        }

    } while (Process32Next(hProcessSnap, &pe32));
    
    return counter;
}
