#include "utils.h"

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"
#include "communications.h"
#include "dllLoader.h"

typedef struct {
    Lane *lane;
    int *closeCondition; //closeCondition = 1, quando for para exit closeCondition = 0
    int *endGame; //endGame = 1, quando for para exit endGame = 0
    //HANDLE hMutex;
}TDADOS;

//threads
DWORD WINAPI ThreadLane(LPVOID param){
    TDADOS* dados = (TDADOS*)param;
    int *cc = dados->closeCondition, *endGame = dados->endGame;
    while(*cc || *endGame){
        Sleep((1/dados->lane->velCarros) * 1000);
        if(moveCars(dados->lane)){
           *endGame = 0;
        }
        _tprintf_s(_T("Lane %d: Carro x: %d\n"),dados->lane->y, dados->lane->cars[0].x);
    }

    ExitThread(0);
}

//função que vai fazer o setup do servidor
BOOL setupServer(HANDLE hConsole, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared) {
    HANDLE dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(hConsole, TEXT("Erro ao carregar a DLL!"));
        return FALSE;
    }
    if(!setMap(hConsole, dllHandle, velIniCarros, numFaixas)){
        errorMessage(hConsole, TEXT("Erro ao fazer o mapa!"));
        return FALSE;
    }
    if(!getMap(hConsole, dllHandle, shared)){
        errorMessage(hConsole, TEXT("Erro ao fazer o mapa!"));
        return FALSE;
    }
    return TRUE;
}


int _tmain(int argc, TCHAR** argv) {

    DWORD numFaixas, velIniCarros;

    //extern "C" VOID __cdecl GetSharedMem(LPWSTR lpszBuf, DWORD cchSize);

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

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

    initRegistry(argc, argv, &numFaixas, &velIniCarros, hConsole);

    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupServer(hConsole, numFaixas, velIniCarros, shared)){
        errorMessage(hConsole, TEXT("Erro ao dar setup do servidor!"));
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    HANDLE threadHandles[8];
    int closeCondition = 1;
    int endGame = 1;
    TDADOS dados[8];
    for (int i = 0; i < (int)numFaixas; i++) {
        dados[i].lane = &shared->game.lanes[i];
        dados[i].closeCondition = &closeCondition;
        dados[i].endGame = &endGame;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, &dados[i], 0, NULL);
    }
    int closeProg = 0;
    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole);

    } while (closeProg == 0);
    closeCondition = 0;
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);

    /*Game game;
    initGame(&game, numFaixas, velIniCarros);
    HANDLE threadHandles[8];
    int closeCondition = 1;
    int endGame = 1;
    TDADOS dados[8];
    for(int i = 0; i < (int)numFaixas; i++){
        dados[i].lane = &game.lanes[i];
        dados[i].closeCondition = &closeCondition;
        dados[i].endGame = &endGame;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, &dados[i], 0, NULL);
    }
    int closeProg = 0;
    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole);
        
    } while (closeProg == 0);
    closeCondition = 0;
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);
    */
	return 0;
}