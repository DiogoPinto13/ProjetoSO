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
#define NAME_CLOSE_CLIENTS_EVENT _T("closeClients")

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
    HANDLE hEventUpdateStartingLane;
    HANDLE hEventUpdateFinishingLane;
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
    int *closeCondition;
    DWORD velIniCarros;
    DWORD numFaixas;
    HANDLE hNamedPipe;
    HANDLE hMutexDLL;
    HANDLE dllHandle;
    HANDLE hConsole;
    HANDLE hEventCloseClients;
    HANDLE hEventUpdateStartingLane;
    HANDLE hEventUpdateFinishingLane;
    SharedMemory *shared;
}TCLIENTDADOS;

typedef struct {
    HANDLE hConsole;
    HANDLE hMutexDLL;
    HANDLE dllHandle;
    SharedMemory* shared;
    HANDLE hNamedPipeMap;
    HANDLE hNamedPipeMovements;
    HANDLE hEventUpdateStartingLane;
    HANDLE hEventUpdateFinishingLane;
    HANDLE hEventCloseClients;
    int *closeCondition;
    int clientNumber;
}TCLIENTE;

typedef struct{
    HANDLE hConsole;
    HANDLE hMutexDLL;
    HANDLE dllHandle;
    SharedMemory* shared;
    HANDLE hEvents[8];
    int *closeCondition;
    int numLanes;
}TCLIENTMAP;

//threads
DWORD WINAPI ThreadLane(LPVOID param) {
    TLANEDADOS* dados = (TLANEDADOS*)param;
    int* cc = dados->closeCondition;
    while (*cc) {
        //if (WaitForSingleObject(dados->hMutex, INFINITE) == WAIT_OBJECT_0) {
            //Sleep(500);
            Sleep((DWORD) (1000 / dados->shared->game.lanes[dados->indexLane].velCarros));
            //_tprintf(_T("\nthread quer o mutex: %d\n"), dados->indexLane);
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            //_tprintf(_T("\nthread ficou com mutex: %d\n"), dados->indexLane);
            if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
                ExitThread(0);
            }
            if(dados->shared->game.estado){
                /*if (moveCars(&dados->shared->game.lanes[dados->indexLane], dados->shared->game.frogs, dados->shared->game.numFrogs, dados->shared->game.specialLanes[1].y, dados->hEventUpdateStartingLane)) {
                    *cc = 0;
                    ExitThread(0);
                }*/
                moveCars(&dados->shared->game.lanes[dados->indexLane], dados->shared->game.frogs, dados->shared->game.numFrogs, dados->shared->game.specialLanes[1].y, dados->hEventUpdateStartingLane);
                if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                    *cc = 0;
                    ExitThread(0);
                }
                SetEvent(dados->hEventUpdateUI);
            }
            //_tprintf_s(_T("Lane %d: Carro x: %d\n"), dados->indexLane, dados->shared->game.lanes[dados->indexLane].cars[0].x);
            //_tprintf(_T("\nthread %d deu release\n"), dados->indexLane);
            ReleaseMutex(dados->hMutexDLL);
            //ReleaseMutex(dados->hMutex);
        //}
    }
    ExitThread(0);
}

DWORD WINAPI ThreadCliente(LPVOID param) {
    TCLIENTE* dados = (TCLIENTE*)param;

    if(!ConnectNamedPipe(dados->hNamedPipeMovements, NULL) && (GetLastError() != ERROR_PIPE_CONNECTED)){
        errorMessage(_T("\nErro ao conectar o pipe de movimentos."), dados->hConsole);
        *dados->closeCondition = 0;
        _tprintf_s(_T("Erro: %d\n"), GetLastError());
        ExitThread(-1);
    }

    enum Movement action;
    enum ResponseMovement response;
    DWORD byteNumber;
    int closeClient = 1;

    while(*dados->closeCondition && closeClient){
        if(!ReadFile(dados->hNamedPipeMovements, &action, sizeof(enum Movement), &byteNumber, NULL)){
            errorMessage(_T("\nErro ao ler do pipe."), dados->hConsole);
            *dados->closeCondition = 0;
            _tprintf_s(_T("Erro: %d\n"), GetLastError());
            ExitThread(-1);
        }

        if(action != END){
            //Movement of frogs
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
                ExitThread(0);
            }
            if(dados->shared->game.estado == FALSE){
                response = PAUSED;
            }
            else{
                //_tprintf_s(_T("\nantes: %d %d"), dados->shared->game.frogs[0].x, dados->shared->game.frogs[0].y);
                response = moveFrog(&dados->shared->game, &dados->shared->game.frogs[dados->clientNumber], action);
                //_tprintf_s(_T("\ndepois: %d %d"), dados->shared->game.frogs[0].x, dados->shared->game.frogs[0].y);
            }
            if(response == OK || response == WIN){
                SetEvent(dados->hEventUpdateStartingLane);
                SetEvent(dados->hEventUpdateFinishingLane);
            }
            if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(TEXT("Erro ao fazer update d mapa!"), dados->hConsole);
                *dados->closeCondition = 0;
                ExitThread(0);
            }
            ReleaseMutex(dados->hMutexDLL);
            if(!WriteFile(dados->hNamedPipeMovements, &response, sizeof(enum ResponseMovement), &byteNumber, NULL)){
                errorMessage(_T("\nErro ao escrever no pipe."), dados->hConsole);
                *dados->closeCondition = 0;
                _tprintf_s(_T("Erro: %d\n"), GetLastError());
                ExitThread(-1);
            }
            if(response == LOSE){
                closeClient = 0;
            }
        }
        else
            closeClient = 0;
    }
    FlushFileBuffers(dados->hNamedPipeMovements);
    CloseHandle(dados->hNamedPipeMovements);
    CloseHandle(dados->hNamedPipeMap);
    WaitForSingleObject(dados->hMutexDLL, INFINITE);
    if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
        errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
        *dados->closeCondition = 0;
        ExitThread(0);
    }
    if(dados->clientNumber == 0){
        dados->shared->game.estado = FALSE;
        if(dados->shared->game.numFrogs == 2)
            SetEvent(dados->hEventCloseClients);
    }
    dados->shared->game.numFrogs--;
    if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
        errorMessage(TEXT("Erro ao fazer update do mapa!"), dados->hConsole);
        *dados->closeCondition = 0;
        ExitThread(0);
    }
    ReleaseMutex(dados->hMutexDLL);
    SetEvent(dados->hEventUpdateStartingLane);
    SetEvent(dados->hEventUpdateFinishingLane);
    free(dados);
    ExitThread(0);
}

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
            errorMessage(_T("\nErro ao conectar o pipe de atendimento."), dados->hConsole);
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

        WaitForSingleObject(dados->hMutexDLL, INFINITE);
        if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
            errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
            *cc = 0;
            ExitThread(0);
        }
        numClients = dados->shared->game.numFrogs;
        //Create other pipes, setup thread
        if(numClients < 2){
            TCLIENTE *clientData = malloc(sizeof(TCLIENTE));
            clientData->closeCondition = cc;
            clientData->dllHandle = dados->dllHandle;
            clientData->hConsole = dados->hConsole;
            clientData->hMutexDLL = dados->hMutexDLL;
            clientData->shared = dados->shared;
            clientData->clientNumber = numClients;
            clientData->hEventCloseClients = dados->hEventCloseClients;
            clientData->hEventUpdateStartingLane = dados->hEventUpdateStartingLane;
            clientData->hEventUpdateFinishingLane = dados->hEventUpdateFinishingLane;
            clientData->hNamedPipeMap = setupFifoMap(pid);
            if(clientData->hNamedPipeMap == INVALID_HANDLE_VALUE){
                errorMessage(_T("erro ao criar o pipe de mapa!!"), dados->hConsole);
                _tprintf_s(_T("Erro: %d\n"), GetLastError());
                pid = -1;
            }
            clientData->hNamedPipeMovements = setupFifoMovement(pid);
            if(clientData->hNamedPipeMovements == INVALID_HANDLE_VALUE){
                errorMessage(_T("erro ao criar o pipe de movimentos!!"), dados->hConsole);
                _tprintf_s(_T("Erro: %d\n"), GetLastError());
                pid = -1;
            }
            if(pid != -1){
                clients[numClients] = CreateThread(NULL, 0, ThreadCliente, clientData, 0, NULL);
                if (clients[numClients] == NULL) {
                    errorMessage(_T("Erro ao lançar as threads de comunicação com os utilizadores!!"), dados->hConsole);
                    CloseHandle(clientData->hNamedPipeMap);
                    CloseHandle(clientData->hNamedPipeMovements);
                    free(clientData);
                    pid = -1;
                }
                else{
                    if(numClients == 0){
                        initGame(&dados->shared->game, dados->numFaixas, dados->velIniCarros);
                    }
                    initFrog(dados->shared->game.frogs, &dados->shared->game.frogs[numClients], &dados->shared->game.numFrogs, dados->shared->game.specialLanes[1].y);
                    dados->shared->game.frogs[numClients].hNamedPipeMovement = clientData->hNamedPipeMovements;
                    dados->shared->game.frogs[numClients].hNamedPipeMap = clientData->hNamedPipeMap;
                    dados->shared->game.estado = TRUE;
                    if (!updateMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                        errorMessage(TEXT("Erro ao fazer update do mapa!"), dados->hConsole);
                        *cc = 0;
                        ExitThread(0);
                    }
                    SetEvent(dados->hEventUpdateStartingLane);
                    SetEvent(dados->hEventUpdateFinishingLane);
                    pid = 0;
                }
            }
            else{
                if(clientData->hNamedPipeMap != INVALID_HANDLE_VALUE)
                    CloseHandle(clientData->hNamedPipeMap);
                else if(clientData->hNamedPipeMovements != INVALID_HANDLE_VALUE)
                    CloseHandle(clientData->hNamedPipeMovements);
                free(clientData);
            }
        }
        else
            pid = -2;
        ReleaseMutex(dados->hMutexDLL);
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
    WaitForMultipleObjects(numClients, clients, TRUE, INFINITE);
    CloseHandle(dados->hNamedPipe);
    free(dados);
    ExitThread(0);
}

DWORD WINAPI ThreadSendMap(LPVOID param){
    TCLIENTMAP *dados = (TCLIENTMAP*)param;
    int index;
    DWORD nBytes;
    while(*dados->closeCondition){
        index = WaitForMultipleObjects(dados->numLanes, dados->hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        WaitForSingleObject(dados->hMutexDLL, INFINITE);
        if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
            errorMessage(TEXT("Erro ao carregar o mapa!"), dados->hConsole);
            *dados->closeCondition = 0;
            ExitThread(0);
        }
        if(dados->shared->game.estado){
            for(int i = 0; i < dados->shared->game.numFrogs; i++){
                CLIENTMAP msg;
                //Estrutura o mapa
                msg.level = dados->shared->game.frogs[i].level;
                msg.numLifes = dados->shared->game.frogs[i].currentLifes;
                msg.points = dados->shared->game.frogs[i].points;
                msg.numFaixas = dados->shared->game.numFaixas;
                int flag;
                //finishing lane
                for (int k = 0; k < 20; k++) {
                    flag = 0;
                    for(int l = 0; l < dados->shared->game.numFrogs; l++){
                        if(dados->shared->game.frogs[l].x == k && dados->shared->game.frogs[l].y == dados->shared->game.specialLanes[0].y){
                            msg.map[0][k] = dados->shared->game.frogs[l].symbol;
                            flag = 1;
                        }
                    }
                    if(flag == 0){
                        msg.map[0][k] = dados->shared->game.specialLanes[0].caracter;
                    }
                }
                //Normal Lanes
                for (int j = 1; j <= dados->shared->game.numFaixas; j++) {
                    for(int k = 0; k < 20; k++){
                        flag = 0;
                        for(int l = 0; l < dados->shared->game.lanes[j - 1].numOfCars; l++){
                            if(dados->shared->game.lanes[j - 1].hasObstacle){
                                if(dados->shared->game.lanes[j - 1].obstacle.x == k){
                                    msg.map[j][k] = dados->shared->game.lanes[j - 1].obstacle.caracter;
                                    flag = 1;
                                }
                            }
                            if(dados->shared->game.lanes[j - 1].cars[l].x == k){
                                msg.map[j][k] = dados->shared->game.lanes[j - 1].cars[l].symbol;
                                flag = 1;
                            }
                            if(dados->shared->game.lanes[j - 1].numOfFrogs != 0){
                                for(int m = 0; m < dados->shared->game.lanes[j - 1].numOfFrogs; m++){
                                    if(dados->shared->game.lanes[j - 1].frogsOnLane[m].x == k){
                                        msg.map[j][k] = dados->shared->game.lanes[j - 1].frogsOnLane[m].symbol;
                                        flag = 1;
                                        break;
                                    }
                                }
                            }
                        }
                        if(flag == 0){
                            msg.map[j][k] = _T(' ');
                        }
                    }
                }
                //starting lane
                for (int k = 0; k < 20; k++) {
                    flag = 0;
                    for(int l = 0; l < dados->shared->game.numFrogs; l++){
                        if(dados->shared->game.frogs[l].x == k && dados->shared->game.frogs[l].y == dados->shared->game.specialLanes[1].y){
                            msg.map[dados->shared->game.numFaixas + 1][k] = dados->shared->game.frogs[l].symbol;
                            flag = 1;
                        }
                    }
                    if(flag == 0){
                        msg.map[dados->shared->game.numFaixas + 1][k] = dados->shared->game.specialLanes[1].caracter;
                    }
                }
                //Envia o mapa e os dados
                if(!WriteFile(dados->shared->game.frogs[i].hNamedPipeMap, &msg, sizeof(CLIENTMAP), &nBytes, NULL) && (GetLastError() != ERROR_INVALID_HANDLE)){
                    errorMessage(_T("\nErro ao escrever no pipe do mapa."), dados->hConsole);
                    _tprintf_s(_T("\nError: %d"), GetLastError());
                    *dados->closeCondition = 0;
                    ExitThread(-1);
                }
            }
        }
        ResetEvent(dados->hEvents[index]);
        ReleaseMutex(dados->hMutexDLL);
    }
    ExitThread(0);
}

BOOL InterpretCommand(BufferCell *cell, SharedMemory *shared, HANDLE hWaitableTimer, HANDLE *threadHandles, HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL) {
    
    if(_tcscmp(cell->command, _T("pause")) == 0){
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = (cell->param1)* -10000000;
        
        /*for (int i = 0; i < (int)shared->game.numFaixas; i++) {
            if (!SuspendThread(threadHandles[i])) {
                errorMessage(_T("erro ao pausar o jogo..."), hConsole);
                return FALSE;
            }
        }*/
        WaitForSingleObject(hMutexDLL, INFINITE);
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.estado = FALSE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        ReleaseMutex(hMutexDLL);
        if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)){
            return FALSE;
        }
        _tprintf_s(_T("\nGame has been paused for %d seconds."), cell->param1);
        WaitForSingleObject(hWaitableTimer, INFINITE);
        WaitForSingleObject(hMutexDLL, INFINITE);
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.estado = TRUE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        ReleaseMutex(hMutexDLL);

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
        WaitForSingleObject(hMutexDLL, INFINITE);
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.lanes[lane].obstacle.x = x;
        shared->game.lanes[lane].obstacle.caracter = _T('O');
        shared->game.lanes[lane].hasObstacle = TRUE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        ReleaseMutex(hMutexDLL);
    }
    else if (_tcscmp(cell->command, _T("invertLane")) == 0) {
        int lane = cell->param1;
        _tprintf(_T("\nInverting lane: %d.\nCommand :>"), lane);
        WaitForSingleObject(hMutexDLL, INFINITE);
        if(!getMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        shared->game.lanes[lane].isReverse = shared->game.lanes[lane].isReverse ? FALSE : TRUE;
        if(!updateMap(hConsole, dllHandle, shared)){
            return FALSE;
        }
        ReleaseMutex(hMutexDLL);
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
            /*if(!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(_T("Erro ao ir buscar a memoria partilhada!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }*/
            func(cell);
            //interpret command here
            if(!InterpretCommand(cell, dados->shared, dados->hWaitableTimer, dados->threadHandles, dados->hConsole, dados->dllHandle, dados->hMutexDLL)){
                errorMessage(_T("Erro ao interpretar o comando!"), dados->hConsole);
                *cc = 0;
                ExitThread(0);
            }
            ReleaseSemaphore(dados->hSemWriteBuffer, 1, NULL);
            
            //_tprintf_s(_T("\ncomando: %s\n"), cell->command);
        //}
    }
    free(cell);
    free(dados);
    ExitThread(0);
}

//função que vai fazer o setup do servidor
BOOL setupServer(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventClose, HANDLE *hWaitableTimer, HANDLE *threadHandles, HANDLE *hMutexDLL, HANDLE *hSemReadBuffer, HANDLE *hSemWriteBuffer, HANDLE *hEventUpdateStartingLane, HANDLE *hEventUpdateFinishingLane, HANDLE *hNamedPipe, HANDLE *hEventCloseClients, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared, int *closeCondition) {
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

    *hEventCloseClients = CreateEvent(NULL, TRUE, FALSE, NAME_CLOSE_CLIENTS_EVENT);
    if(*hEventCloseClients == NULL){
        errorMessage(TEXT("Erro ao criar evento de Close dos clientes!"), hConsole);
        return FALSE;
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
        data->hEventUpdateStartingLane = *hEventUpdateStartingLane;
        data->hEventUpdateFinishingLane = *hEventUpdateFinishingLane;
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
    dadosAtendeClientes->numFaixas = numFaixas;
    dadosAtendeClientes->velIniCarros = velIniCarros;
    dadosAtendeClientes->hEventCloseClients = *hEventCloseClients;
    dadosAtendeClientes->hEventUpdateStartingLane = *hEventUpdateStartingLane;
    dadosAtendeClientes->hEventUpdateFinishingLane = *hEventUpdateFinishingLane;
    CreateThread(NULL, 0, ThreadAtendeClientes, dadosAtendeClientes, 0, NULL);


    TCLIENTMAP* tClientMapData = malloc(sizeof(TCLIENTMAP));
    tClientMapData->closeCondition = closeCondition;
    tClientMapData->dllHandle = *dllHandle;
    tClientMapData->hConsole = hConsole;
    for (int i = 0; i < (int)numFaixas; i++) {
        tClientMapData->hEvents[i] = hEventUpdateUI[i];
    }
    tClientMapData->hMutexDLL = hMutexDLL;
    tClientMapData->numLanes = numFaixas;
    tClientMapData->shared = shared;
    CreateThread(NULL, 0, ThreadSendMap, tClientMapData, 0, NULL);
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
    HANDLE hEventCloseClients;
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
    if(!setupServer(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, &hWaitableTimer, threadHandles, &hMutexDLL, &hSemReadBuffer, &hSemWriteBuffer, &hEventUpdateStartingLane, &hEventUpdateFinishingLane, &hNamedPipe, &hEventCloseClients, numFaixas, velIniCarros, shared, &closeCondition)){
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