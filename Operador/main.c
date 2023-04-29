#include "utils.h"

//operador de cenas

int _tmain(int argc, TCHAR** argv) {

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get DLL stuff

    // Setup UI Threads, each thread is going to print a specific lane

    int closeProg = 0;

    //command loop
    //_tprintf_s(_T("\nStartup Complete.\nCommand :> \n"));
    do{
        _tprintf_s(_T("Command :> "));
        readCommands(&closeProg, hConsole);

    }while(closeProg == 0);
    //errorMessage(TEXT("fodase"), hConsole);

    return 0;
}