#include "utils.h"

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"
#include "communications.h"
#include "dllLoader.h"

#define NAME_UI_EVENT _T("updateUIEvent")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_BUFFER_EVENT _T("updateBuffer")

typedef struct {
    SharedMemory* shared;
    int *closeCondition; //closeCondition = 1, quando for para exit closeCondition = 0
    int *endGame; //endGame = 1, quando for para exit endGame = 0
    int indexLane;
    HANDLE hMutex;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hEventUpdateUI;
    HANDLE hWaitableTimer;
}TLANEDADOS;

typedef struct{
    SharedMemory* shared;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hWaitableTimer;
    HANDLE hEventUpdateBuffer;
    HANDLE *threadHandles;
    int *closeCondition;
}TMESGDADOS;

//threads
DWORD WINAPI ThreadLane(LPVOID param){
    TLANEDADOS* dados = (TLANEDADOS*)param;
    int *cc = dados->closeCondition, *endGame = dados->endGame;
    HANDLE auxMutex = dados->shared->hMutexDLL;
    while(*cc && *endGame){
        if (WaitForSingleObject(dados->hMutex, INFINITE) == WAIT_OBJECT_0) {
            Sleep((1 / dados->shared->game.lanes[dados->indexLane].velCarros) * 1000);
            WaitForSingleObject(auxMutex, INFINITE);
            if(!getMap(dados->hConsole, dados->dllHandle, dados->shared)){
                errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
                ExitThread(0);
            }
            if (moveCars(&dados->shared->game.lanes[dados->indexLane])) {
                *endGame = 0;
                ExitThread(0);
            }
            if(!updateMap(dados->hConsole, dados->dllHandle, dados->shared)){
                *endGame = 0;
                ExitThread(0);
            }
            SetEvent(dados->hEventUpdateUI);
            //_tprintf_s(_T("Lane %d: Carro x: %d\n"), dados->indexLane, dados->shared->game.lanes[dados->indexLane].cars[0].x);
            ReleaseMutex(auxMutex);
            auxMutex = dados->shared->hMutexDLL;
            ReleaseMutex(dados->hMutex);
        }
    }
    ExitThread(0);
}

BOOL InterpretCommand(BufferCell *cell, SharedMemory *shared, HANDLE hWaitableTimer, HANDLE *threadHandles) {
    
    if(_tcscmp(cell->command, _T("pause")) == 0){
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = (cell->param1)* -10000000;
        
        for (int i = 0; i < (int)shared->game.numFaixas; i++) {
            SuspendThread(threadHandles[i]);
        }
        if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)){
            return FALSE;
        }
        WaitForSingleObject(hWaitableTimer, INFINITE);
        for (int i = 0; i < (int)shared->game.numFaixas; i++) {
            ResumeThread(threadHandles[i]);
        }
    }
    else if (_tcscmp(cell->command, _T("addObstacle")) == 0) {
        int lane = cell->param1;
        int x = cell->param2;
    }
    else if (_tcscmp(cell->command, _T("invertLane")) == 0) {
        int lane = cell->param1;
    }

    return TRUE;
}

//its gonna read messages from the circular buffer
DWORD WINAPI ThreadReadMessages(LPVOID param) {
    TMESGDADOS* dados = (TMESGDADOS*)param;
    int *cc = dados->closeCondition;
    HANDLE auxMutex = dados->shared->hMutexDLL;

    GetMessageBufferFunc func = (GetMessageBufferFunc)GetProcAddress(dados->dllHandle, "GetMessageBuffer");
    if (func == NULL) {
        errorMessage(_T("Error in getting the address of the function"), dados->hConsole);
        _tprintf_s(_T("Error code: %d\n"), GetLastError());
        *cc = 0;
        ExitThread(1);
    }

    BufferCell* cell = malloc(sizeof(BufferCell));
    while(*cc){
        if (WaitForSingleObject(dados->hEventUpdateBuffer, INFINITE) == WAIT_OBJECT_0) {
            WaitForSingleObject(auxMutex, INFINITE);
            if(!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(_T("Erro ao ir buscar a memoria partilhada!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }
            func(cell);
            //interpret command here
            if(!InterpretCommand(cell, dados->shared, dados->hWaitableTimer, dados->threadHandles)){
                errorMessage(_T("Erro ao interpretar o comando!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }
            ReleaseMutex(auxMutex);
            auxMutex = dados->shared->hMutexDLL;
            //_tprintf_s(_T("\ncomando: %s\n"), cell->command);
        }
    }
    free(cell);
    free(dados);
    ExitThread(0);
}

//função que vai fazer o setup do servidor
BOOL setupServer(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hMutexThreadsLane, HANDLE *hEventUpdateUI, HANDLE *hEventClose, HANDLE *hEventUpdateBuffer, HANDLE *hWaitableTimer, HANDLE *threadHandles, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared, int *closeCondition) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(TEXT("Erro ao carregar a DLL!"), hConsole);
        return FALSE;
    }
    if(!setMap(hConsole, *dllHandle, velIniCarros, numFaixas)){
        errorMessage(TEXT("Erro ao fazer o mapa!"), hConsole);
        return FALSE;
    }
    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(TEXT("Erro ao carregar o mapa!"), hConsole);
        return FALSE;
    }
    *hMutexThreadsLane = CreateMutex(NULL, FALSE, NULL);
    if(*hMutexThreadsLane == NULL){
        errorMessage(TEXT("Erro ao criar mutex!"), hConsole);
        return FALSE;
    }

    *hEventUpdateUI = CreateEvent(NULL, FALSE, FALSE, NAME_UI_EVENT);
    if(*hEventUpdateUI == NULL){
        errorMessage(TEXT("Erro ao criar evento do UI!"), hConsole);
        return FALSE;
    }

    *hEventClose = CreateEvent(NULL, TRUE, FALSE, NAME_CLOSE_EVENT);
    if(*hEventClose == NULL){
        errorMessage(TEXT("Erro ao criar evento de Close!"), hConsole);
        return FALSE;
    }

    *hEventUpdateBuffer = CreateEvent(NULL, FALSE, FALSE, NAME_BUFFER_EVENT);
    if(*hEventUpdateBuffer == NULL){
        errorMessage(TEXT("Erro ao criar o evento do Buffer!"), hConsole);
        return FALSE;
    }

    *hWaitableTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if(*hWaitableTimer == NULL){
        errorMessage(TEXT("Erro ao criar o waitable timer!"), hConsole);
        return FALSE;
    }
    CancelWaitableTimer(*hWaitableTimer);

    TMESGDADOS *dados = malloc(sizeof(TMESGDADOS));
    dados->dllHandle = *dllHandle;
    dados->hConsole = hConsole;
    dados->closeCondition = closeCondition;
    dados->hEventUpdateBuffer = *hEventUpdateBuffer;
    dados->hWaitableTimer = *hWaitableTimer;
    dados->shared = shared;
    dados->threadHandles = threadHandles;

    if(CreateThread(NULL, 0, ThreadReadMessages, dados, 0, NULL) == NULL){
        errorMessage(TEXT("Erro ao iniciar Thread de comunicação!"), hConsole);
        return FALSE;
    }

    return TRUE;
}


int _tmain(int argc, TCHAR** argv) {

    DWORD numFaixas, velIniCarros;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hEventClose; //Event to signal server Close
    HANDLE hEventUpdateUI; //Event to signal updated Data
    HANDLE hEventUpdateBuffer; //Event to know when buffer has data
    HANDLE hMutexThreadsLane; //Mutex for threads (so they dont write at the same time on the data)
    HANDLE hWaitableTimer; //Waitable timer to stop the threads from making the game progress
    HANDLE threadHandles[8];
    int closeCondition = 1; //Used to close everything
    int endGame = 1; //Used to close Game Threads
    int closeProg = 0; //Used to close Server
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

    //verificar se há mais do que uma instância, se sim, vamos suicidar
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        errorMessage(TEXT("Já existe uma instância do Servidor a correr..."), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    initRegistry(argc, argv, &numFaixas, &velIniCarros, hConsole);

    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupServer(hConsole, &dllHandle, &hMutexThreadsLane, &hEventUpdateUI, &hEventClose, &hEventUpdateBuffer, &hWaitableTimer, threadHandles, numFaixas, velIniCarros, shared, &closeCondition)){
        errorMessage(TEXT("Erro ao dar setup do servidor!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    //Wait for clients
    //quando os clientes se conectarem, vamos arrancar as threads mais tarde...

    //Setup Game
    for (int i = 0; i < (int)numFaixas; i++) {
        dados[i].shared = shared;
        dados[i].closeCondition = &closeCondition; //Comunication between everything
        dados[i].endGame = &endGame; //Communication between threads
        dados[i].hMutex = hMutexThreadsLane;
        dados[i].indexLane = i;
        dados[i].hConsole = hConsole;
        dados[i].dllHandle = dllHandle;
        dados[i].hEventUpdateUI = hEventUpdateUI;
        dados[i].hWaitableTimer = hWaitableTimer;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, &dados[i], 0, NULL);
    }

    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole);
    } while (closeProg == 0);
    closeCondition = 0;
    SetEvent(hEventClose);
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);
	return 0;
}