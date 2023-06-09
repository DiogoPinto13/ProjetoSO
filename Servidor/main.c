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
    int* closeCondition;
    HANDLE hNamedPipe;
    HANDLE hMutexDLL;
    HANDLE dllHandle;
    HANDLE hConsole;
    SharedMemory *shared;
}TCLIENTDADOS;

typedef struct {
    HANDLE hConsole;
    HANDLE hMutexDLL;
    HANDLE dllHandle;
    SharedMemory* shared;
    HANDLE hNamedPipeMap;
    HANDLE hNamedPipeMovements;
}TCLIENTE;

//threads
DWORD WINAPI ThreadLane(LPVOID param) {
    TLANEDADOS* dados = (TLANEDADOS*)param;
    int* cc = dados->closeCondition;
    while (*cc) {
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
                    *cc = 0;
                    ExitThread(0);
                }
                if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                    *cc = 0;
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

DWORD WINAPI ThreadCliente(LPVOID param) {
    TCLIENTE* dados = (TCLIENTE*)param;
    
    WaitForSingleObject(dados->hMutexDLL, INFINITE);
    if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
        errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
        ExitThread(0);
    }

    if (dados->shared->game.numFrogs == 1) {
        dados->shared->game.estado = TRUE;
    }
    ReleaseMutex(dados->hMutexDLL);

    //vai fzr cenas
    free(dados);
    ExitThread(0);
}

/*DWORD WINAPI ThreadSendMap(LPVOID param){
    Exit
}*/

DWORD WINAPI ThreadAtendeClientes(LPVOID param) {
    TCLIENTDADOS* dados = (TCLIENTDADOS*)param;
    int *cc = dados->closeCondition;
    HANDLE clients[2];
    int numClients = 0;
    int pid;
    DWORD byteNumber;
    while(*cc){
        //ligar ao pipe, esperar por pessoas, ler do pipe, criar thread de cliente
        if(!ConnectNamedPipe(dados->hNamedPipe, NULL) && (GetLastError() != ERROR_PIPE_CONNECTED)){
            errorMessage(_T("\nErro ao conectar o pipe."), dados->hConsole);
            *cc = 0;
            _tprintf_s(_T("Erro: %d\n"), GetLastError());
            ExitThread(-1);
        }
        if(!ReadFile(dados->hNamedPipe, &pid, sizeof(int), &byteNumber, NULL)){
            errorMessage(_T("\nErro ao ler do pipe."), dados->hConsole);
            *cc = 0;
            _tprintf_s(_T("Erro: %d\n"), GetLastError());
            ExitThread(-1);
        }

        //Create other pipes, setup thread
        if(numClients < 2){
            TCLIENTE *clientData = malloc(sizeof(TCLIENTE));
            clientData->dllHandle = dados->dllHandle;
            clientData->hConsole = dados->hConsole;
            clientData->hMutexDLL = dados->hMutexDLL;
            clientData->shared = dados->shared;
            clientData->hNamedPipeMap = NULL;
            if(clientData->hNamedPipeMap == NULL){
                pid = -1;
            }
            clientData->hNamedPipeMovements = NULL;
            if(clientData->hNamedPipeMovements == NULL){
                pid = -1;
            }
            if(pid != -1){
                clients[numClients] = CreateThread(NULL, 0, ThreadCliente, clientData, 0, NULL);
                if (clients[numClients] == NULL) {
                    errorMessage(_T("erro ao lançar as threads de comunicação com os utilizadores!!"), dados->hConsole);
                    pid = -1;
                }
                else{
                    pid = 0;
                    numClients++;
                }
            }
        }
        else
            pid = -2;
        //Sends answer to client
        if(!WriteFile(dados->hNamedPipe, &pid, sizeof(int), &byteNumber, NULL)){
            errorMessage(_T("\nErro ao ler do pipe."), dados->hConsole);
            *cc = 0;
            ExitThread(-1);
        }

        FlushFileBuffers(dados->hNamedPipe);

        if(!DisconnectNamedPipe(dados->hNamedPipe)){
            errorMessage(_T("\nErro ao disconectar o pipe."), dados->hConsole);
            *cc = 0;
            ExitThread(-1);
        }
    }
    
    //quando chega o primeiro cliente e enquanto houver pessoas a jogar...
    //Sleep(5000);
    
    //lançar uma thread de atender um actual client?
    CloseHandle(dados->hNamedPipe);
    free(dados);
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
BOOL setupServer(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventClose, HANDLE *hWaitableTimer, HANDLE *threadHandles, HANDLE *hMutexDLL, HANDLE *hSemReadBuffer, HANDLE *hSemWriteBuffer, HANDLE *hEventUpdateStartingLane, HANDLE *hEventUpdateFinishingLane, HANDLE *hNamedPipe, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared, int *closeCondition) {
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

    *hNamedPipe = CreateNamedPipe(FIFOBACKEND, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(int), sizeof(int), 1000, NULL);
    if(*hNamedPipe == INVALID_HANDLE_VALUE){
        errorMessage(TEXT("Erro ao criar o Named Pipe!"), hConsole);
        return FALSE;
    }

    *hWaitableTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if(*hWaitableTimer == NULL){
        errorMessage(TEXT("Erro ao criar o waitable timer!"), hConsole);
        return FALSE;
    }
    CancelWaitableTimer(*hWaitableTimer);

    //Setup Game
    for (int i = 0; i < (int)numFaixas; i++) {
        TLANEDADOS *data = malloc(sizeof(TLANEDADOS));
        data->shared = shared;
        data->closeCondition = closeCondition; //Comunication between everything
        data->indexLane = i;
        data->hConsole = hConsole;
        data->dllHandle = *dllHandle;
        data->hEventUpdateUI = hEventUpdateUI[i];
        data->hWaitableTimer = *hWaitableTimer;
        data->hMutexDLL = *hMutexDLL;
        threadHandles[i] = CreateThread(NULL, 0, ThreadLane, data, 0, NULL);
    }

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
    
    TCLIENTDADOS *dadosAtendeClientes = malloc(sizeof(TCLIENTDADOS));
    dadosAtendeClientes->closeCondition = closeCondition;
    dadosAtendeClientes->dllHandle = *dllHandle;
    dadosAtendeClientes->hConsole = hConsole;
    dadosAtendeClientes->hMutexDLL = *hMutexDLL;
    dadosAtendeClientes->hNamedPipe = *hNamedPipe;
    dadosAtendeClientes->shared = shared;
    CreateThread(NULL, 0, ThreadAtendeClientes, dadosAtendeClientes, 0, NULL);

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
    HANDLE hNamedPipe;
    int closeCondition = 1; //Used to close everything
    int closeProg = 0; //Used to close Server

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
    if(!setupServer(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, &hWaitableTimer, threadHandles, &hMutexDLL, &hSemReadBuffer, &hSemWriteBuffer, &hEventUpdateStartingLane, &hEventUpdateFinishingLane, &hNamedPipe, numFaixas, velIniCarros, shared, &closeCondition)){
        errorMessage(TEXT("Erro ao dar setup do servidor!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole, dllHandle, threadHandles, hEventUpdateStartingLane, hEventUpdateFinishingLane, numFaixas, velIniCarros);
    } while (closeProg == 0 && closeCondition);
    closeCondition = 0;
    SetEvent(hEventClose);
    WaitForMultipleObjects(numFaixas, threadHandles, TRUE, INFINITE);
	return 0;
}