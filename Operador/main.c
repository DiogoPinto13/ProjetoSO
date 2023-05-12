#include "utils.h"

#include "console.h"
#include "commands.h"
#include "dllLoader.h"

typedef struct {
    HANDLE dllHandle;
    //HANDLE hMutex;  //pra escrever no ecra for now
    HANDLE hConsole;
    //SharedMemory* shared;
    int *closeCondition;
}TMAPDADOS;

//thread de escrita temporaria
DWORD WINAPI ThreadReadMap(LPVOID param) {
    TMAPDADOS* dados = (TMAPDADOS*)param;
    SharedMemory* shared = malloc(sizeof(SharedMemory));
    int *cc = dados->closeCondition;
    while (*cc) {
        Sleep(1000);
        if (!getMap(dados->hConsole, dados->dllHandle, shared)) {
            errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
            *cc = 0;
        }
        _tprintf_s(_T("Lane 0: Carro x: %d\n"), shared->game.lanes[0].cars[0].x);
    }

    ExitThread(0);
}




//função que vai fazer o setup do operador
BOOL setupOperator(HANDLE hConsole, HANDLE *dllHandle, SharedMemory *shared, SetMessageBufferFunc *SetMessageFunc, int *closeCondition) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(hConsole, TEXT("Erro ao carregar a DLL!"));
        return FALSE;
    }

    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(hConsole, TEXT("Erro ao carregar o mapa!"));
        return FALSE;
    }
    *SetMessageFunc = (SetMessageBufferFunc)GetProcAddress(*dllHandle, "SetMessageBuffer");

    TMAPDADOS *dados = malloc(sizeof(TMAPDADOS));
    dados->closeCondition = closeCondition;
    dados->dllHandle = *dllHandle;
    dados->hConsole = hConsole;
    CreateThread(NULL, 0, ThreadReadMap, dados, 0, NULL);
    return TRUE;
}

int _tmain(int argc, TCHAR** argv) {

    HANDLE hConsole;
    HANDLE dllHandle;
    SetMessageBufferFunc SetMessageFunc;
    int closeCondition = 1;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    //se n houver nenhuma instância do server...
    if(checkIfIsAlreadyRunning(TEXT("Servidor.exe")) == 0){
        errorMessage(TEXT("O servidor não está ligado!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Get DLL stuff
    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupOperator(hConsole, &dllHandle, shared, &SetMessageFunc, &closeCondition)){
        errorMessage(hConsole, TEXT("Erro ao dar setup do servidor!"));
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    // Setup UI Threads, each thread is going to print a specific lane

    int closeProg = 0;

    //command loop
    //_tprintf_s(_T("\nStartup Complete.\nCommand :> \n"));
    do{
        _tprintf_s(_T("Command :> "));
        readCommands(&closeProg, hConsole);

    }while(closeProg == 0);
    //errorMessage(TEXT("fodase"), hConsole);

    return 0;
}