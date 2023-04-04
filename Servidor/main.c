#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>


int checkIfIsAlreadyRunning(TCHAR *processName) {
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;
    int counter = 0;
    
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32))
    {
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

int _tmain(int argc, TCHAR **argv){

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    
    int numFaixas = 2, velIniCarros;

    //verificar se há mais do que uma instância, se sim, vamos suicidar
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        _tprintf(TEXT("\nJá existe uma instância do Servidor a correr...\n"));
        ExitProcess(0);
    }
    //buscar as cenas através da linha de comandos
    if (argc == 2) {
        numFaixas += (int) argv[1];
        velIniCarros = 1;
    }
    else if (argc == 3) {
        numFaixas += (int) argv[1];
        velIniCarros = (int) argv[2];
    }

    //se nao, vai ao registry
    else {
        //valores by default que vao estar guardados
        numFaixas = 5;
        velIniCarros = 1;

    }

    //game = game.c/.h has frog, lanes, start, finish, points
    //points = points.c/.h
    //frog = frog.c/.h
    //lanes = lanes.c/.h inside cars.c/.h



	return 0;
}