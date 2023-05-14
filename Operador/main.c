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
    HANDLE hMutexConsole;  //pra escrever no ecra for now
    HANDLE hConsole;
    HANDLE hEventUpdateUI;
    SharedMemory* shared;
    int *closeCondition, *pauseUI, numLane;
}TMAPDADOS;

typedef struct {
    HANDLE hEventClose;
}TKILLDADOS;
 
//thread de escrita temporaria
DWORD WINAPI ThreadReadMap(LPVOID param) {
    TMAPDADOS* dados = (TMAPDADOS*)param;
    //SharedMemory* shared = malloc(sizeof(SharedMemory));
    int *cc = dados->closeCondition, *pauseUI = dados->pauseUI;
    HANDLE auxMutex = dados->shared->hMutexDLL;

    while (*cc) {
        //quando receber o evento do server, vai buscar o mapa
        if(*pauseUI == 0){
            if (WaitForSingleObject(dados->hEventUpdateUI, INFINITE) == WAIT_OBJECT_0) {
                WaitForSingleObject(auxMutex, INFINITE);
                if (!getMap(dados->hConsole, dados->dllHandle, dados->shared)) {
                    errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
                    *cc = 0;
                }
                _tprintf_s(_T("Lane %d: Carro x: %d\n"), dados->numLane, dados->shared->game.lanes[0].cars[0].x);
                ReleaseMutex(auxMutex);
                auxMutex = dados->shared->hMutexDLL;
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
BOOL setupOperator(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventCLose, HANDLE *hEventUpdateBuffer, HANDLE *hThreadsUI, HANDLE *hMutexConsole, SetMessageBufferFunc *SetMessageFunc, int *closeCondition, int *pauseUI, int *numActiveLanes) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(TEXT("Erro ao carregar a DLL!"), hConsole);
        return FALSE;
    }

    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(TEXT("Erro ao carregar o mapa!"), hConsole);
        return FALSE;
    }

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
        hEventUpdateUI[i] = CreateEvent(NULL, FALSE, FALSE, buffer);
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
        hThreadsUI[i] = CreateThread(NULL, 0, ThreadReadMap, dados, 0, NULL);
        if(hThreadsUI[i] == NULL){
            errorMessage(TEXT("Erro ao criar a thread!"), hConsole);
            return FALSE;
        }
        *numActiveLanes = i;
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
    HANDLE hThreadsUI[8];
    SetMessageBufferFunc SetMessageFunc;
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

    //se n houver nenhuma instância do server...
    if(checkIfIsAlreadyRunning(TEXT("Servidor.exe")) == 0){
        errorMessage(TEXT("O servidor não está ligado!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Get DLL stuff
    //SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupOperator(hConsole, &dllHandle, hEventUpdateUI, &hEventClose, &hEventUpdateBuffer, hThreadsUI, &hMutexConsole, &SetMessageFunc, &closeCondition, &pauseUI, &numActiveLanes)){
        errorMessage(TEXT("Erro ao dar setup do servidor!"), hConsole);
        CloseHandle(hConsole);
        ExitProcess(0);
    }
    
    // Setup UI Threads, each thread is going to print a specific lane

    //command loop
    //_tprintf_s(_T("\nStartup Complete.\nCommand :> \n"));
    do{
        fgetwc(stdin);
        pauseUI = 1;
        _tprintf_s(_T("Command :> "));
        readCommands(&closeProg, hConsole, SetMessageFunc, hEventUpdateBuffer, dllHandle);
        pauseUI = 0;
    }while(closeProg == 0 && closeCondition);
    closeCondition = 0;
    WaitForMultipleObjects(numActiveLanes, hThreadsUI, TRUE, INFINITE);
    return 0;
}