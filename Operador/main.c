#include "utils.h"

#include "console.h"
#include "commands.h"
#include "dllLoader.h"

#define NAME_UI_EVENT _T("updateUIEvent")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_BUFFER_EVENT _T("updateBuffer")

typedef struct {
    HANDLE dllHandle;
    //HANDLE hMutex;  //pra escrever no ecra for now
    HANDLE hConsole;
    HANDLE hEventUpdateUI;
    HANDLE hEventUpdateBuffer;
    //SharedMemory* shared;
    int *closeCondition, *pauseUI;
}TMAPDADOS;

typedef struct {
    HANDLE hEventClose;
}TKILLDADOS;

//thread de escrita temporaria
DWORD WINAPI ThreadReadMap(LPVOID param) {
    TMAPDADOS* dados = (TMAPDADOS*)param;
    SharedMemory* shared = malloc(sizeof(SharedMemory));
    int *cc = dados->closeCondition, *pauseUI = dados->pauseUI;
    while (*cc) {
        //quando receber o evento do server, vai buscar o mapa
        if(WaitForSingleObject(dados->hEventUpdateUI, INFINITE) == WAIT_OBJECT_0){
            if (!getMap(dados->hConsole, dados->dllHandle, shared)) {
                errorMessage(_T("\nErro ao ir buscar o mapa do server...\n"), dados->hConsole);
                *cc = 0;
            }
            else if(*pauseUI == 0){
                _tprintf_s(_T("Lane 0: Carro x: %d\n"), shared->game.lanes[0].cars[0].x);
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
BOOL setupOperator(HANDLE hConsole, HANDLE *dllHandle, HANDLE *hEventUpdateUI, HANDLE *hEventCLose, HANDLE *hEventUpdateBuffer, SharedMemory *shared, SetMessageBufferFunc *SetMessageFunc, int *closeCondition, int *pauseUI) {
    *dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(TEXT("Erro ao carregar a DLL!"), hConsole);
        return FALSE;
    }

    if(!getMap(hConsole, *dllHandle, shared)){
        errorMessage(TEXT("Erro ao carregar o mapa!"), hConsole);
        return FALSE;
    }

    *hEventUpdateUI = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_UI_EVENT);
    if (*hEventUpdateUI == NULL) {
        errorMessage(TEXT("Erro ao abrir o evento do UI!"), hConsole);
        return FALSE;
    }

    *hEventCLose = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_CLOSE_EVENT);
    if (*hEventCLose == NULL) {
        errorMessage(TEXT("Erro ao abrir o evento de Close!"), hConsole);
        return FALSE;
    }

    *hEventUpdateBuffer = OpenEvent(EVENT_ALL_ACCESS, FALSE, NAME_BUFFER_EVENT);
    if (*hEventUpdateBuffer == NULL) {
        errorMessage(TEXT("Erro ao abrir o evento do update do buffer!"), hConsole);
        return FALSE;
    }

    *SetMessageFunc = (SetMessageBufferFunc)GetProcAddress(*dllHandle, "SetMessageBuffer");
    if (*SetMessageFunc == NULL) {
        errorMessage(_T("Error in getting the address of the function"), hConsole);
        return FALSE;
    }

    TMAPDADOS *dados = malloc(sizeof(TMAPDADOS));
    dados->closeCondition = closeCondition;
    dados->pauseUI = pauseUI;
    dados->dllHandle = *dllHandle;
    dados->hConsole = hConsole;
    dados->hEventUpdateUI = *hEventUpdateUI;
    CreateThread(NULL, 0, ThreadReadMap, dados, 0, NULL);

    TKILLDADOS* data = malloc(sizeof(TKILLDADOS));
    data->hEventClose = *hEventCLose;
    CreateThread(NULL, 0, KillThread, data, 0, NULL);


    return TRUE;
}

int _tmain(int argc, TCHAR** argv) {

    HANDLE hConsole;
    HANDLE dllHandle;
    HANDLE hEventUpdateUI; //Event to updateUI
    HANDLE hEventClose; //Event that signals Server is over
    HANDLE hEventUpdateBuffer; //Event that signals a new entry on buffer
    SetMessageBufferFunc SetMessageFunc;
    int closeCondition = 1;
    int pauseUI = 0;
    int closeProg = 0;

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
    SharedMemory *shared = malloc(sizeof(SharedMemory));
    if(!setupOperator(hConsole, &dllHandle, &hEventUpdateUI, &hEventClose, &hEventUpdateBuffer, shared, &SetMessageFunc, &closeCondition, &pauseUI)){
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
        readCommands(&closeProg, hConsole, SetMessageFunc, hEventUpdateBuffer);
        pauseUI = 0;
    }while(closeProg == 0 && closeCondition);
    closeCondition = 0;
    return 0;
}