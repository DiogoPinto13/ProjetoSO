#include "utils.h"

#include "console.h"
#include "commands.h"
#include "dllLoader.h"

#define NAME_UI_EVENT _T("updateUIEvent")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_BUFFER_EVENT _T("updateBuffer")
#define NAME_UPDATE_EVENT _T("updateEvent%d")

typedef struct {
    HANDLE dllHandle;
    HANDLE hMutexConsole;
    HANDLE hConsole;
    HANDLE hEventUpdateUI;
    HANDLE hMutexDLL;
    SharedMemory* shared;
    int *closeCondition, *pauseUI, numLane;
}TMAPDADOS;

typedef struct {
    HANDLE hEventClose;
}TKILLDADOS;
 
//thread de escrita das faixas normais 
DWORD WINAPI ThreadReadMap(LPVOID param) {
    TMAPDADOS* dados = (TMAPDADOS*)param;
    int *cc = dados->closeCondition, *pauseUI = dados->pauseUI;
    COORD pos;
    pos.X = INITIAL_COLUMN;
    pos.Y = dados->shared->game.lanes[dados->numLane].y;
    //CONSOLE_SCREEN_BUFFER_INFO csbi;
    TCHAR buffer[21];

    while (*cc) {
        //quando receber o evento do server, vai buscar o mapa
        if (WaitForSingleObject(dados->hEventUpdateUI, INFINITE) == WAIT_OBJECT_0) {
            if(*pauseUI == 0){
                WaitForSingleObject(dados->hMutexConsole, INFINITE);
                WaitForSingleObject(dados->hMutexDLL, INFINITE);
                if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                    errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
                    *cc = 0;
                }
                
                //GetConsoleScreenBufferInfo(dados->hConsole, &csbi);
                SetConsoleCursorPosition(dados->hConsole, pos);
                int flag = 0;
                for(int j = 0; j < 20; j++){
                    flag = 0;
                    for(int k = 0; k < dados->shared->game.lanes[dados->numLane].numOfCars; k++){
                        if(dados->shared->game.lanes[dados->numLane].obstacle.x == j){
                            buffer[j] = dados->shared->game.lanes[dados->numLane].obstacle.caracter;
                            flag = 1;
                        }
                        else if(dados->shared->game.lanes[dados->numLane].cars[k].x == j){
                            buffer[j] = dados->shared->game.lanes[dados->numLane].cars[k].symbol;
                            flag = 1;
                        }
                    }
                    if(flag == 0){
                        buffer[j] = _T(' ');
                    }
                }
                buffer[20] = '\0';
                _tprintf_s(_T("%d ||%s||"), dados->numLane, buffer);
                //SetConsoleCursorPosition(dados->hConsole, csbi.dwCursorPosition);
                ReleaseMutex(dados->hMutexDLL);
                ReleaseMutex(dados->hMutexConsole);
            }
        }
    }
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
BOOL setupOperator(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventCLose, HANDLE *hEventUpdateBuffer, HANDLE *hThreadsUI, HANDLE *hMutexConsole, HANDLE *hMutexDLL, SetMessageBufferFunc *SetMessageFunc, int *closeCondition, int *pauseUI, int *numActiveLanes, SharedMemory* shared) {
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

    *hEventUpdateBuffer = CreateEvent(NULL, FALSE, FALSE, NAME_BUFFER_EVENT);
    if (*hEventUpdateBuffer == NULL) {
        errorMessage(TEXT("Erro ao abrir o evento do update do buffer!"), hConsole);
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
        dados->closeCondition = closeCondition;
        dados->pauseUI = pauseUI;
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
    HANDLE hEventUpdateBuffer; //Event that signals a new entry on buffer
    HANDLE hMutexConsole; //Acesso à consola
    HANDLE hMutexDLL;
    HANDLE hThreadsUI[8];
    SetMessageBufferFunc SetMessageFunc;
    TCHAR *msg;
    TCHAR buffer[21];
    int closeCondition = 1;
    int pauseUI = 0;
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
    if(!setupOperator(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, &hEventUpdateBuffer, hThreadsUI, &hMutexConsole, &hMutexDLL, &SetMessageFunc, &closeCondition, &pauseUI, &numActiveLanes, shared)){
        errorMessage(TEXT("Erro ao dar setup do servidor!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Setup UI Threads, each thread is going to print a specific lane

    //command loop
    //_tprintf_s(_T("\nStartup Complete.\nCommand :> \n"));
    do{
        pauseUI = 1;
        WaitForSingleObject(hMutexConsole, INFINITE);
        for(int i = 0; i < 2; i++){
            pos.X = INITIAL_COLUMN;
            pos.Y = shared->game.specialLanes[i].y;
            SetConsoleCursorPosition(hConsole, pos);
            int flag = 0;
            for(int j = 0; j < 20; j++){
                if (!shared->game.specialLanes[i].isFinish) {
                    flag = 0;
                    for(int k = 0; k < shared->game.numFrogs; k++){
                        if(shared->game.frogs[k].x == j){
                            buffer[j] = shared->game.frogs[k].symbol;
                            flag = 1;
                        }
                    }
                    if(flag == 0){
                        buffer[j] = shared->game.specialLanes[i].caracter;
                    }
                }
                else
                    buffer[j] = shared->game.specialLanes[i].caracter;
            }
            buffer[20] = '\0';
            _tprintf_s(_T("%s ||%s||"), _T("S"), buffer);
        }
        //enter pra escrever um comando...
        ReleaseMutex(hMutexConsole);
        pauseUI = 0;
        fgetwc(stdin);
        pauseUI = 1;
        pos.X = 0;
        pos.Y = 16;
        WaitForSingleObject(hMutexConsole, INFINITE);
        SetConsoleCursorPosition(hConsole, pos);

        _tprintf_s(_T("Command :> "));
        msg = readCommands(&closeProg, numActiveLanes, hConsole, SetMessageFunc, hEventUpdateBuffer, dllHandle, hMutexDLL);
        if(msg != NULL)
            _tprintf_s(msg);
        fgetwc(stdin);
        FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 26, pos, &res);
        pos.X = 0;
        pos.Y = 0;
        SetConsoleCursorPosition(hConsole, pos);
        pauseUI = 0;
        ReleaseMutex(hMutexConsole);
    }while(closeProg == 0 && closeCondition);
    closeCondition = 0;
    WaitForMultipleObjects(numActiveLanes, hThreadsUI, TRUE, INFINITE);
    return 0;
}