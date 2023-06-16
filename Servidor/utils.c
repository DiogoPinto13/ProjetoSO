#include "utils.h"

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

float generateRandomNumber(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}
