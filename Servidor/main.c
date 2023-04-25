#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"

int checkIfIsAlreadyRunning(TCHAR *processName) {
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;
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

void errorMessage(HANDLE hConsole, TCHAR* errorMessage) {
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    _ftprintf_s(stderr, TEXT("\n%s\n"), errorMessage);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

int _tmain(int argc, TCHAR** argv) {

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    DWORD numFaixas, velIniCarros;

    //para as corzinhas lindas
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        _ftprintf_s(stderr, TEXT("Error getting hConsole handle\n"));
        return 1;
    }


    //verificar se há mais do que uma instância, se sim, vamos suicidar
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        errorMessage(hConsole, TEXT("Já existe uma instância do Servidor a correr..."));
        ExitProcess(0);
    }
    //buscar as cenas através da linha de comandos
    HKEY regKey = getKey();
    if (regKey == NULL) {
        ExitProcess(0);
    }

    //num faixas: 1 a 8 inclusive
    //velocidade inicial: 1 a 5 inclusive 
    if (argc == 3) {
        numFaixas = (DWORD)argv[1];
        velIniCarros = (DWORD)argv[2];

        if (numFaixas < 1 || numFaixas > 8) {
            TCHAR bufferMessage[512];
            numFaixas = getNumFaixas(regKey);
            errorMessage(hConsole, TEXT("O número de faixas tem que ser entre 1 a 8!"));
            _sprintf_p(bufferMessage, sizeof(bufferMessage), TEXT("Usando os valores por default: %d"), numFaixas);
            errorMessage(hConsole, bufferMessage);
        }
        else {
            setNumFaixas(regKey, numFaixas);
        }
        if (velIniCarros < 1 || velIniCarros > 5) {
            TCHAR bufferMessage[512];
            velIniCarros = getVelIniCarros(regKey);
            errorMessage(hConsole, TEXT("O número da velocidade inicial do carro tem que ser entre 1 e 5!"));
            _sprintf_p(bufferMessage, sizeof(bufferMessage), TEXT("Usando os valores por default: %d"), velIniCarros);
            errorMessage(hConsole, bufferMessage);
        }
        else {
            setVelIniCarros(regKey, velIniCarros);
        }
    }
    else {
        velIniCarros = getVelIniCarros(regKey);  //registry
        if (argc == 2) {
            numFaixas = (DWORD)argv[1];
            if (numFaixas < 1 || numFaixas > 8) {
                numFaixas = getNumFaixas(regKey);
                TCHAR bufferMessage[512];
                errorMessage(hConsole, TEXT("O número de faixas tem que ser entre 1 a 8!"));
                _sprintf_p(bufferMessage, sizeof(bufferMessage), TEXT("Usando os valores por default: %d"), numFaixas);
                errorMessage(hConsole, bufferMessage);
            }
            else {
                setNumFaixas(regKey, numFaixas);
            }
        }
        //se nao, vai ao registry
        else {
            //valores by default que vao estar guardados
            numFaixas = getNumFaixas(regKey);
        }
    }
    CloseHandle(regKey);

    //game = game.c/.h has frog, lanes, start, finish, points
    //points = points.c/.h
    //frog = frog.c/.h
    //lanes = lanes.c/.h inside cars.c/.h


    int closeProg = 0;
    fd_set selectParams;
    int startTime = time(NULL);
    _tprintf_s(TEXT("\nStartup complete.\n\nCommand :> \n"));
    do {
        readCommands(&closeProg, hConsole);
        /*
        FD_ZERO(&selectParams);
        FD_SET(0, &selectParams);
        select(1, &selectParams, NULL, NULL, NULL);
        if (FD_ISSET(0, &selectParams)) {
            readCommands(&closeProg);
            if (closeProg == 0)
                _tprintf_s(TEXT("\nCommand :> \n"));
        }
        if (startTime < time(NULL)) {
            startTime = time(NULL);
            //func de instantes
        }
        */
    } while (closeProg == 0);

	return 0;
}