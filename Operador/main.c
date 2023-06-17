#include "utils.h"

#include "console.h"
#include "commands.h"
#include "dllLoader.h"

#define NAME_UI_EVENT _T("updateUIEvent")
#define NAME_READ_SEMAPHORE TEXT("readSemaphore")
#define NAME_WRITE_SEMAPHORE TEXT("writeSemaphore")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_BUFFER_EVENT _T("updateBuffer")
#define NAME_UPDATE_EVENT _T("updateEvent%d")
#define NAME_UPDATE_EVENT_STARTING_LANE _T("updateEventStartingLane")
#define NAME_UPDATE_EVENT_FINISHING_LANE _T("updateEventFinishingLane")

typedef struct {
    HANDLE dllHandle;
    HANDLE hMutexConsole;
    HANDLE hConsole;
    HANDLE hEventUpdateUI;
    HANDLE hMutexDLL;
    SharedMemory* shared;
    int *closeCondition, numLane;
}TMAPDADOS;

typedef struct {
    HANDLE dllHandle;
    HANDLE hMutexConsole;
    HANDLE hConsole;
    HANDLE hMutexDLL;
    HANDLE hEventUpdate;
    SharedMemory *shared;
    int *closeCondition;
    boolean isFinishing;
}TSPECIALLANESREFRESH;

typedef struct {
    HANDLE hEventClose;
}TKILLDADOS;
 
//thread de escrita das faixas normais 
DWORD WINAPI ThreadReadMap(LPVOID param) {
    TMAPDADOS* dados = (TMAPDADOS*)param;
    int *cc = dados->closeCondition;
    COORD pos;
    pos.X = INITIAL_COLUMN;
    pos.Y = dados->shared->game.lanes[dados->numLane].y;
    //CONSOLE_SCREEN_BUFFER_INFO csbi;
    TCHAR buffer[21];
    while (*cc) {
        //quando receber o evento do server, vai buscar o mapa
        if (WaitForSingleObject(dados->hEventUpdateUI, 1000) == WAIT_OBJECT_0) {
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
                *cc = 0;
            }
            ReleaseMutex(dados->hMutexDLL);
            WaitForSingleObject(dados->hMutexConsole, INFINITE);
            //GetConsoleScreenBufferInfo(dados->hConsole, &csbi);
            SetConsoleCursorPosition(dados->hConsole, pos);
            int flag = 0;
            for(int j = 0; j < 20; j++){
                flag = 0;
                for(int k = 0; k < dados->shared->game.lanes[dados->numLane].numOfCars; k++){
                    if(dados->shared->game.lanes[dados->numLane].cars[k].x == j){
                        buffer[j] = dados->shared->game.lanes[dados->numLane].cars[k].symbol;
                        flag = 1;
                        break;
                    }
                }
                if(dados->shared->game.lanes[dados->numLane].hasObstacle){
                    if(dados->shared->game.lanes[dados->numLane].obstacle.x == j){
                        buffer[j] = dados->shared->game.lanes[dados->numLane].obstacle.caracter;
                        flag = 1;
                    }
                }
                if(dados->shared->game.lanes[dados->numLane].numOfFrogs != 0){
                    for(int i = 0; i < dados->shared->game.lanes[dados->numLane].numOfFrogs; i++){
                        if(dados->shared->game.lanes[dados->numLane].frogsOnLane[i].x == j){
                            buffer[j] = dados->shared->game.lanes[dados->numLane].frogsOnLane[i].symbol;
                            flag = 1;
                            break;
                        }
                    }
                }
                if(flag == 0){
                    buffer[j] = _T(' ');
                }
            }
            buffer[20] = '\0';
            _tprintf_s(_T("%d ||%s||"), dados->numLane, buffer);
            //SetConsoleCursorPosition(dados->hConsole, csbi.dwCursorPosition);
            ReleaseMutex(dados->hMutexConsole);
            ResetEvent(dados->hEventUpdateUI);
        }
    }
    ExitThread(0);
}

DWORD WINAPI ThreadSpecialLanesRefresh(LPVOID param) {
    TSPECIALLANESREFRESH* dados = (TSPECIALLANESREFRESH*) param;
    int *cc = dados->closeCondition;
    int index = (dados->isFinishing ? 0 : 1);
    TCHAR buffer[21];
    COORD pos;
    pos.X = INITIAL_COLUMN;
    pos.Y = dados->shared->game.specialLanes[index].y;

    int flag = 0;

    WaitForSingleObject(dados->hMutexConsole, INFINITE);
    SetConsoleCursorPosition(dados->hConsole, pos);
    for(int j = 0; j < 20; j++){
        flag = 0;
        for(int k = 0; k < dados->shared->game.numFrogs; k++){
            if(dados->shared->game.frogs[k].x == j){
                if(dados->shared->game.frogs[k].y == dados->shared->game.specialLanes[index].y){
                    buffer[j] = dados->shared->game.frogs[k].symbol;
                    flag = 1;
                }
            }
        }
        if(flag == 0){
            buffer[j] = dados->shared->game.specialLanes[index].caracter;
        }
    }
    buffer[20] = '\0';
    _tprintf_s(_T("%s ||%s||"), _T("S"), buffer);
    ReleaseMutex(dados->hMutexConsole);

    while(*cc){
        if (WaitForSingleObject(dados->hEventUpdate, 2000) == WAIT_OBJECT_0) {
            WaitForSingleObject(dados->hMutexDLL, INFINITE);
            if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
                *cc = 0;
            }
            ReleaseMutex(dados->hMutexDLL);
            WaitForSingleObject(dados->hMutexConsole, INFINITE);
            SetConsoleCursorPosition(dados->hConsole, pos);
            for(int j = 0; j < 20; j++){
                flag = 0;
                for(int k = 0; k < dados->shared->game.numFrogs; k++){
                    if(dados->shared->game.frogs[k].x == j){
                        if(dados->shared->game.frogs[k].y == dados->shared->game.specialLanes[index].y){
                            buffer[j] = dados->shared->game.frogs[k].symbol;
                            flag = 1;
                        }
                    }
                }
                if(flag == 0){
                    buffer[j] = dados->shared->game.specialLanes[index].caracter;
                }
            }
            buffer[20] = '\0';
            _tprintf_s(_T("%s ||%s||"), _T("S"), buffer);
            //enter pra escrever um comando...
            ReleaseMutex(dados->hMutexConsole);
            ResetEvent(dados->hEventUpdate);
        }
    }
    free(dados);
    ExitThread(0);
}

//thread que vai matar o programa
DWORD WINAPI KillThread(LPVOID param) {
    TKILLDADOS* dados = (TKILLDADOS*)param;

    if (WaitForSingleObject(dados->hEventClose, INFINITE) == WAIT_OBJECT_0) {
        ExitProcess(0);
    }

    ExitThread(0);
}

//função que vai fazer o setup do operador
BOOL setupOperator(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventCLose, HANDLE *hThreadsUI, HANDLE *hMutexConsole, HANDLE *hMutexDLL, HANDLE *hSemReadBuffer, HANDLE *hSemWriteBuffer, HANDLE *hEventUpdateStartingLane, HANDLE *hEventUpdateFinishingLane, SetMessageBufferFunc *SetMessageFunc, int *closeCondition, int *numActiveLanes, SharedMemory* shared) {
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
    //SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(TEXT("Erro ao carregar o mapa!"), hConsole);
        return FALSE;
    }
    ReleaseMutex(*hMutexDLL);

    *hEventCLose = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_EVENT);
    if (*hEventCLose == NULL) {
        errorMessage(TEXT("Erro ao abrir o evento de Close!"), hConsole);
        return FALSE;
    }

    *hSemReadBuffer = CreateSemaphore(NULL, 0, BUFFER_SIZE, NAME_READ_SEMAPHORE);
    if(*hSemReadBuffer == NULL){
        errorMessage(TEXT("Erro ao criar o semáfero de leitura do Buffer!"), hConsole);
        //_tprintf_s(_T("%d"), GetLastError());
        return FALSE;
    }

    *hSemWriteBuffer = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NAME_WRITE_SEMAPHORE);
    if(*hSemWriteBuffer == NULL){
        errorMessage(TEXT("Erro ao criar o semáfero de escrita do Buffer!"), hConsole);
        return FALSE;
    }


    *SetMessageFunc = (SetMessageBufferFunc)GetProcAddress(*dllHandle, "SetMessageBuffer");
    if (*SetMessageFunc == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        return FALSE;
    }

    *hMutexConsole = CreateMutex(NULL, FALSE, NULL);
    if (*hMutexConsole == NULL) {
        errorMessage(_T("Error in creating the console Mutex."), hConsole);
        return FALSE;
    }

    *hEventUpdateStartingLane = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_UPDATE_EVENT_STARTING_LANE);
    if (*hEventUpdateStartingLane == NULL) {
        errorMessage(_T("Erro ao abrir o evento de refresh da starting lane."), hConsole);
        return FALSE;
    }

    *hEventUpdateFinishingLane = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_UPDATE_EVENT_FINISHING_LANE);
    if (*hEventUpdateFinishingLane == NULL) {
        errorMessage(_T("Erro ao abrir o evento de refresh da finishing lane."), hConsole);
        return FALSE;
    }
    
    for(int i = 0; i < shared->game.numFaixas; i++){
        TCHAR buffer[20];
        _swprintf_p(buffer, 20, NAME_UPDATE_EVENT, i);
        hEventUpdateUI[i] = OpenEvent(EVENT_ALL_ACCESS, FALSE, buffer);
        if(hEventUpdateUI[i] == NULL){
            errorMessage(TEXT("Erro ao criar evento do UI!"), hConsole);
            return FALSE;
        }
    }
    
    for(int i = 0; i < shared->game.numFaixas; i++){
        TMAPDADOS *dados = malloc(sizeof(TMAPDADOS));
        dados->hMutexConsole = *hMutexConsole;
        dados->closeCondition = closeCondition;
        dados->dllHandle = *dllHandle;
        dados->hConsole = hConsole;
        dados->hEventUpdateUI = hEventUpdateUI[i];
        dados->shared = shared;
        dados->numLane = i;
        dados->hMutexDLL = *hMutexDLL;
        hThreadsUI[i] = CreateThread(NULL, 0, ThreadReadMap, dados, 0, NULL);
        if(hThreadsUI[i] == NULL){
            errorMessage(TEXT("Erro ao criar a thread!"), hConsole);
            return FALSE;
        }
        *numActiveLanes = i + 1;
    }

    for(int i = 0; i < 2; i++){
        TSPECIALLANESREFRESH *dados = malloc(sizeof(TSPECIALLANESREFRESH));
        if(i == 0){
            dados->hEventUpdate = *hEventUpdateFinishingLane;
            dados->isFinishing = TRUE;
        }
        else{
            dados->hEventUpdate = *hEventUpdateStartingLane;
            dados->isFinishing = FALSE;
        }
        dados->hMutexConsole = *hMutexConsole;
        dados->closeCondition = closeCondition;
        dados->dllHandle = *dllHandle;
        dados->hConsole = hConsole;
        dados->hMutexDLL = *hMutexDLL;
        dados->shared = shared;
        hThreadsUI[(*numActiveLanes) + i] = CreateThread(NULL, 0, ThreadSpecialLanesRefresh, dados, 0, NULL);
    }

    TKILLDADOS* data = malloc(sizeof(TKILLDADOS));
    data->hEventClose = *hEventCLose;
    CreateThread(NULL, 0, KillThread, data, 0, NULL);


    return TRUE;
}

int _tmain(int argc, TCHAR** argv) {

    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hEventUpdateUI[8]; //Events to updateUI
    HANDLE hEventClose; //Event that signals Server is over
    HANDLE hEventUpdateStartingLane;
    HANDLE hEventUpdateFinishingLane;
    HANDLE hSemWriteBuffer; //Semaphore for writing
    HANDLE hSemReadBuffer; //Semaphore for reading
    HANDLE hMutexConsole; //Acesso à consola
    HANDLE hMutexDLL;
    HANDLE hThreadsUI[10];
    SetMessageBufferFunc SetMessageFunc;
    TCHAR *msg;
    int closeCondition = 1;
    int closeProg = 0;
    int numActiveLanes = 0;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole == NULL){
        errorMessage(TEXT("Erro ao obter o handle da consola!"), hConsole);
        ExitProcess(0);
    }

    COORD pos;
    DWORD res; 
    pos.X = 0;
    pos.Y = 0;

    FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 26, pos, &res);

    pos.X = 0;
    pos.Y = 0;
    SetConsoleCursorPosition(hConsole, pos);

    //se n houver nenhuma instância do server...
    if(checkIfIsAlreadyRunning(TEXT("Servidor.exe")) == 0){
        errorMessage(TEXT("O servidor não está ligado!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Get DLL stuff
    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupOperator(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, hThreadsUI, &hMutexConsole, &hMutexDLL, &hSemReadBuffer, &hSemWriteBuffer, &hEventUpdateStartingLane, &hEventUpdateFinishingLane, &SetMessageFunc, &closeCondition, &numActiveLanes, shared)){
        errorMessage(TEXT("Erro ao dar setup do servidor!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Setup UI Threads, each thread is going to print a specific lane

    //command loop
    //_tprintf_s(_T("\nStartup Complete.\nCommand :> \n"));
    do{
        fgetwc(stdin);
        pos.X = 0;
        pos.Y = 16;
        WaitForSingleObject(hMutexConsole, INFINITE);
        SetConsoleCursorPosition(hConsole, pos);
        _tprintf_s(_T("Command :> "));
        msg = readCommands(&closeProg, numActiveLanes, hConsole, SetMessageFunc, hSemReadBuffer, hSemWriteBuffer, dllHandle, hMutexDLL);
        if(msg != NULL)
            _tprintf_s(msg);
        fgetwc(stdin);
        FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 26, pos, &res);
        pos.X = 0;
        pos.Y = 0;
        SetConsoleCursorPosition(hConsole, pos);
        ReleaseMutex(hMutexConsole);
    }while(closeProg == 0 && closeCondition);
    closeCondition = 0;
    WaitForMultipleObjects(numActiveLanes + 2, hThreadsUI, TRUE, INFINITE);
    return 0;
}