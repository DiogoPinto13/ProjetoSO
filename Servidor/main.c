#include "utils.h"

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"
#include "communications.h"
#include "dllLoader.h"

#define NAME_UI_EVENT _T("updateUIEvent")
#define NAME_READ_SEMAPHORE TEXT("readSemaphore")
#define NAME_WRITE_SEMAPHORE TEXT("writeSemaphore")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_BUFFER_EVENT _T("updateBuffer")
#define NAME_UPDATE_EVENT _T("updateEvent%d")
#define NAME_UPDATE_EVENT_STARTING_LANE _T("updateEventStartingLane")
#define NAME_UPDATE_EVENT_FINISHING_LANE _T("updateEventFinishingLane")

#define MAX_LANES 8

typedef struct {
    SharedMemory* shared;
    int *closeCondition; //closeCondition = 1, quando for para exit closeCondition = 0
    int *endGame; //endGame = 1, quando for para exit endGame = 0
    int indexLane;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hEventUpdateUI;
    HANDLE hWaitableTimer;
    HANDLE hMutexDLL;
}TLANEDADOS;

typedef struct{
    SharedMemory* shared;
    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hWaitableTimer;
    HANDLE hSemReadBuffer;
    HANDLE hSemWriteBuffer;
    HANDLE *threadHandles;
    HANDLE hMutexDLL;
    int *closeCondition;
}TMESGDADOS;

typedef struct {
    HANDLE threadHandles[MAX_LANES];
    int numFaixas;
}TCLIENTDADOS;

//threads
DWORD WINAPI ThreadLane(LPVOID param) {
    TLANEDADOS* dados = (TLANEDADOS*)param;
    int* cc = dados->closeCondition, *endGame = dados->endGame;
    while (*cc && *endGame) {
        //if (WaitForSingleObject(dados->hMutex, INFINITE) == WAIT_OBJECT_0) {
            //Sleep(500);
            float timer1 = (float)(1 / (float)dados->shared->game.lanes[dados->indexLane].velCarros);
            int timer =  (int)(timer1 * 1000);
            Sleep(timer);
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
                ExitThread(0);
            }
            if(dados->shared->game.estado){
                if (moveCars(&dados->shared->game.lanes[dados->indexLane])) {
                    *endGame = 0;
                    ExitThread(0);
                }
                if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                    *endGame = 0;
                    ExitThread(0);
                }
                SetEvent(dados->hEventUpdateUI);
            }
            //_tprintf_s(_T("Lane %d: Carro x: %d\n"), dados->indexLane, dados->shared->game.lanes[dados->indexLane].cars[0].x);
            ReleaseMutex(dados->hMutexDLL);
            //ReleaseMutex(dados->hMutex);
        //}
    }
    ExitThread(0);
}

DWORD WINAPI ThreadAtendeClientes(LPVOID param) {
    TCLIENTDADOS* dados = (TCLIENTDADOS*)param;
    
    //quando chega o primeiro cliente e enquanto houver pessoas a jogar...
    //Sleep(5000);
    for(int i = 0; i < dados->numFaixas; i++){
        ResumeThread(dados->threadHandles[i]);
    }

    //lançar uma thread de atender um actual client?

    ExitThread(0);
}

BOOL InterpretCommand(BufferCell *cell, SharedMemory *shared, HANDLE hWaitableTimer, HANDLE *threadHandles, HANDLE hConsole, HANDLE dllHandle) {
    
    if(_tcscmp(cell->command, _T("pause")) == 0){
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = (cell->param1)* -10000000;
        
        /*for (int i = 0; i < (int)shared->game.numFaixas; i++) {
            if (!SuspendThread(threadHandles[i])) {
                errorMessage(_T("erro ao pausar o jogo..."), hConsole);
                return FALSE;
            }
        }*/

        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }

        shared->game.estado = FALSE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)){
            return FALSE;
        }
        _tprintf_s(_T("\nGame has been paused for %d seconds."), cell->param1);
        WaitForSingleObject(hWaitableTimer, INFINITE);
        shared->game.estado = TRUE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }

        /*for (int i = 0; i < (int)shared->game.numFaixas; i++) {
            if (!ResumeThread(threadHandles[i])) {
                errorMessage(_T("erro ao retomar o jogo..."), hConsole);
                return FALSE;
            }
        }*/
        _tprintf_s(_T("\nGame has resumed.\nCommand :>"));
    }
    else if (_tcscmp(cell->command, _T("addObstacle")) == 0) {
        _tprintf(_T("\nAdding an obstacle in lane: %d, x: %d.\nCommand :>"), cell->param1, cell->param2);
        int lane = cell->param1;
        int x = cell->param2;
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.lanes[lane].obstacle.x = x;
        shared->game.lanes[lane].obstacle.caracter = _T('O');
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
    }
    else if (_tcscmp(cell->command, _T("invertLane")) == 0) {
        int lane = cell->param1;
        _tprintf(_T("\nInverting lane: %d.\nCommand :>"), lane);
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.lanes[lane].isReverse = shared->game.lanes[lane].isReverse ? FALSE : TRUE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
    }
    return TRUE;
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

    BufferCell* cell = malloc(sizeof(BufferCell));
    while(*cc){
        //if (WaitForSingleObject(dados->hEventUpdateBuffer, INFINITE) == WAIT_OBJECT_0) {
            WaitForSingleObject(dados->hSemReadBuffer, INFINITE);
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            /*if(!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(_T("Erro ao ir buscar a memoria partilhada!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }*/
            func(cell);
            //interpret command here
            if(!InterpretCommand(cell, dados->shared, dados->hWaitableTimer, dados->threadHandles, dados->hConsole, dados->dllHandle)){
                errorMessage(_T("Erro ao interpretar o comando!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }
            ReleaseSemaphore(dados->hSemWriteBuffer, 1, NULL);
            ReleaseMutex(dados->hMutexDLL);
            //_tprintf_s(_T("\ncomando: %s\n"), cell->command);
        //}
    }
    free(cell);
    free(dados);
    ExitThread(0);
}

//função que vai fazer o setup do servidor
BOOL setupServer(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventClose, HANDLE *hWaitableTimer, HANDLE *threadHandles, HANDLE *hMutexDLL, HANDLE *hSemReadBuffer, HANDLE *hSemWriteBuffer, HANDLE *hEventUpdateStartingLane, HANDLE *hEventUpdateFinishingLane, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared, int *closeCondition) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(TEXT("Erro ao carregar a DLL!"), hConsole);
        return FALSE;
    }

    *hMutexDLL = CreateMutex(NULL, FALSE, NAME_MUTEX_DLL);
    if(*hMutexDLL == NULL){
        errorMessage(TEXT("Erro ao criar o mutex da DLL!"), hConsole);
        return FALSE;
    }
    WaitForSingleObject(*hMutexDLL, INFINITE);
    if(!setMap(hConsole, *dllHandle, velIniCarros, numFaixas)){
        errorMessage(TEXT("Erro ao fazer o mapa!"), hConsole);
        return FALSE;
    }
    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(TEXT("Erro ao carregar o mapa!"), hConsole);
        return FALSE;
    }
    ReleaseMutex(*hMutexDLL);

    for(int i = 0; i < (int)numFaixas; i++){
        TCHAR buffer[20];
        _swprintf_p(buffer, 20, NAME_UPDATE_EVENT, i);
        hEventUpdateUI[i] = CreateEvent(NULL, TRUE, FALSE, buffer);
        if(hEventUpdateUI[i] == NULL){
            errorMessage(TEXT("Erro ao criar evento do UI!"), hConsole);
            return FALSE;
        }
    }

    *hEventClose = CreateEvent(NULL, TRUE, FALSE, NAME_CLOSE_EVENT);
    if(*hEventClose == NULL){
        errorMessage(TEXT("Erro ao criar evento de Close!"), hConsole);
        return FALSE;
    }

    *hEventUpdateStartingLane = CreateEvent(NULL, TRUE, FALSE, NAME_UPDATE_EVENT_STARTING_LANE);
    if (*hEventUpdateStartingLane == NULL) {
        errorMessage(TEXT("Erro ao criar evento do update do starting lane!"), hConsole);
        return FALSE;
    }

    *hEventUpdateFinishingLane = CreateEvent(NULL, TRUE, FALSE, NAME_UPDATE_EVENT_FINISHING_LANE);
    if (*hEventUpdateFinishingLane == NULL) {
        errorMessage(TEXT("Erro ao criar evento do update do finishing lane"), hConsole);
        return FALSE;
    }

    *hSemReadBuffer = CreateSemaphore(NULL, 0, BUFFER_SIZE, NAME_READ_SEMAPHORE);
    if(*hSemReadBuffer == NULL){
        errorMessage(TEXT("Erro ao criar o semáfero de leitura do Buffer!"), hConsole);
        return FALSE;
    }

    *hSemWriteBuffer = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NAME_WRITE_SEMAPHORE);
    if(*hSemWriteBuffer == NULL){
        errorMessage(TEXT("Erro ao criar o semáfero de escrita do Buffer!"), hConsole);
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
    dados->hSemReadBuffer = *hSemReadBuffer;
    dados->hSemWriteBuffer = *hSemWriteBuffer;
    dados->hWaitableTimer = *hWaitableTimer;
    dados->shared = shared;
    dados->threadHandles = threadHandles;
    dados->hMutexDLL = *hMutexDLL;
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
    HANDLE hEventUpdateUI[8]; //Event to signal updated Data
    HANDLE hEventUpdateStartingLane; //Event to signal updated Starting Lane
    HANDLE hEventUpdateFinishingLane; //Event to signal updated Finishing Lane
    HANDLE hSemWriteBuffer; //Semaphore for writing
    HANDLE hSemReadBuffer; //Semaphore for reading
    HANDLE hMutexDLL; //Mutex to control access to the DLL
    HANDLE hWaitableTimer; //Waitable timer to stop the threads from making the game progress
    HANDLE threadHandles[8];
    int closeCondition = 1; //Used to close everything
    int endGame = 1; //Used to close Game Threads
    int closeProg = 0; //Used to close Server
    TLANEDADOS dados[8];
    TCLIENTDADOS dadosAtendeClientes;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        _ftprintf_s(stderr, TEXT("Error getting hConsole handle\n"));
        return 1;
    }

    //verificar se há mais do que uma instância, se sim, vamos fechar o programa
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        errorMessage(TEXT("Já existe uma instância do Servidor a correr..."), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    initRegistry(argc, argv, &numFaixas, &velIniCarros, hConsole);

    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupServer(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, &hWaitableTimer, threadHandles, &hMutexDLL, &hSemReadBuffer, &hSemWriteBuffer, &hEventUpdateStartingLane, &hEventUpdateFinishingLane, numFaixas, velIniCarros, shared, &closeCondition)){
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
        dados[i].indexLane = i;
        dados[i].hConsole = hConsole;
        dados[i].dllHandle = dllHandle;
        dados[i].hEventUpdateUI = hEventUpdateUI[i];
        dados[i].hWaitableTimer = hWaitableTimer;
        dados[i].hMutexDLL = hMutexDLL;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, &dados[i], CREATE_SUSPENDED, NULL);
    }

    //atende cliente
    dadosAtendeClientes.numFaixas = (int)numFaixas;
    for (int i = 0; i < (int)numFaixas; i++) {
        dadosAtendeClientes.threadHandles[i] = threadHandles[i];
    }
    CreateThread(NULL, 0, ThreadAtendeClientes, &dadosAtendeClientes, 0, NULL);

    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole, dllHandle, threadHandles, hEventUpdateStartingLane, hEventUpdateFinishingLane, numFaixas, velIniCarros);
    } while (closeProg == 0);
    closeCondition = 0;
    SetEvent(hEventClose);
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);
	return 0;
}