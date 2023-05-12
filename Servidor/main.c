#include "utils.h"

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"
#include "communications.h"
#include "dllLoader.h"

typedef struct {
    SharedMemory* shared;
    int *closeCondition; //closeCondition = 1, quando for para exit closeCondition = 0
    int *endGame; //endGame = 1, quando for para exit endGame = 0
    int indexLane;
    HANDLE hMutex;
    HANDLE hConsole;
    HANDLE dllHandle;
}TLANEDADOS;

typedef struct{
    SharedMemory* shared;
    HANDLE hConsole;
    HANDLE dllHandle;
    int *closeCondition;
}TMESGDADOS;

//threads
DWORD WINAPI ThreadLane(LPVOID param){
    TLANEDADOS* dados = (TLANEDADOS*)param;
    int *cc = dados->closeCondition, *endGame = dados->endGame;
    while(*cc || *endGame){
        if (WaitForSingleObject(dados->hMutex, INFINITE) == WAIT_OBJECT_0) {
            Sleep((1 / dados->shared->game.lanes[dados->indexLane].velCarros) * 1000);
            if (moveCars(&dados->shared->game.lanes[dados->indexLane])) {
                *endGame = 0;
            }
            if(!updateMap(dados->hConsole, dados->dllHandle, dados->shared)){
                *endGame = 0;
            }
            _tprintf_s(_T("Lane %d: Carro x: %d\n"), dados->indexLane, dados->shared->game.lanes[dados->indexLane].cars[0].x);
            ReleaseMutex(dados->hMutex);
        }
    }

    ExitThread(0);
}

//its gonna read messages from the circular buffer
DWORD WINAPI ThreadReadMessages(LPVOID param) {
    TMESGDADOS* dados = (TMESGDADOS*)param;
    int *cc = dados->closeCondition;

    GetMessageBufferFunc func = (GetMessageBufferFunc)GetProcAddress(dados->dllHandle, "GetMessageBuffer");
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), dados->hConsole);
        _tprintf_s(_T("Error code: %d\n"), GetLastError());
        *cc = 0;
        ExitThread(1);
    }

    BufferCell cell;
    while(*cc){
        func(&cell);
        _tprintf_s(_T("\ncomando: %s\n"), cell.command);
    }
    free(dados);
    ExitThread(0);
}

//fun��o que vai fazer o setup do servidor
BOOL setupServer(HANDLE hConsole, HANDLE *dllHandle, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared, int *closeCondition) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(hConsole, TEXT("Erro ao carregar a DLL!"));
        return FALSE;
    }
    if(!setMap(hConsole, *dllHandle, velIniCarros, numFaixas)){
        errorMessage(hConsole, TEXT("Erro ao fazer o mapa!"));
        return FALSE;
    }
    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(hConsole, TEXT("Erro ao carregar o mapa!"));
        return FALSE;
    }

    TMESGDADOS *dados = malloc(sizeof(TMESGDADOS));
    dados->dllHandle = *dllHandle;
    dados->hConsole = hConsole;
    dados->closeCondition = closeCondition;
    dados->shared = shared;

    CreateThread(NULL, 0, ThreadReadMessages, dados, 0, NULL);

    return TRUE;
}


int _tmain(int argc, TCHAR** argv) {

    DWORD numFaixas, velIniCarros;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE threadHandles[8];
    int closeCondition = 1;
    int endGame = 1;
    TLANEDADOS dados[8];

    //extern "C" VOID __cdecl GetSharedMem(LPWSTR lpszBuf, DWORD cchSize);

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    //para as corzinhas lindas
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        _ftprintf_s(stderr, TEXT("Error getting hConsole handle\n"));
        return 1;
    }

    //verificar se h� mais do que uma inst�ncia, se sim, vamos suicidar
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        errorMessage(hConsole, TEXT("J� existe uma inst�ncia do Servidor a correr..."));
        ExitProcess(0);
    }

    initRegistry(argc, argv, &numFaixas, &velIniCarros, hConsole);

    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupServer(hConsole, &dllHandle, numFaixas, velIniCarros, shared, &closeCondition)){
        errorMessage(hConsole, TEXT("Erro ao dar setup do servidor!"));
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    //Wait for clients
    //quando os clientes se conectarem, vamos arrancar as threads mais tarde...

    //Setup Game
    HANDLE hMutexThreadsLane = CreateMutex(NULL, FALSE, NULL);
    if(hMutexThreadsLane == NULL){
        errorMessage(hConsole, TEXT("Erro ao criar mutex!"));
        ExitProcess(0);
    }

    for (int i = 0; i < (int)numFaixas; i++) {
        dados[i].shared = shared;
        dados[i].closeCondition = &closeCondition; //Comunication between everything
        dados[i].endGame = &endGame; //Communication between threads
        dados[i].hMutex = hMutexThreadsLane;
        dados[i].indexLane = i;
        dados[i].hConsole = hConsole;
        dados[i].dllHandle = dllHandle;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, &dados[i], 0, NULL);
    }
    int closeProg = 0;
    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole);

    } while (closeProg == 0);
    closeCondition = 0;
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);
	return 0;
}