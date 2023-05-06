#include "utils.h"

//Component includes

#include "commands.h"
#include "game.h"
#include "registry.h"
#include "communications.h"
#include "dllLoader.h"

boolean setupServer(HANDLE hConsole, DWORD numFaixas, DWORD velIniCarros, SharedMemory *shared) {
    HANDLE dllHandle = dllLoader(hConsole);
    if(dllHandle == NULL) {
        errorMessage(hConsole, TEXT("Erro ao carregar a DLL!"));
        return FALSE;
    }
    if(!setMap(hConsole, dllHandle, velIniCarros, numFaixas)){
        errorMessage(hConsole, TEXT("Erro ao fazer o mapa!"));
        return FALSE;
    }
    if(!getMap(hConsole, dllHandle, shared)){
        errorMessage(hConsole, TEXT("Erro ao fazer o mapa!"));
        return FALSE;
    }
    return TRUE;
}


int _tmain(int argc, TCHAR** argv) {

    DWORD numFaixas, velIniCarros;

    //extern "C" VOID __cdecl GetSharedMem(LPWSTR lpszBuf, DWORD cchSize);

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    //para as corzinhas lindas
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        _ftprintf_s(stderr, TEXT("Error getting hConsole handle\n"));
        return 1;
    }

    //verificar se há mais do que uma instância, se sim, vamos suicidar
    if (checkIfIsAlreadyRunning(argv[0]) >= 2) {
        errorMessage(hConsole, TEXT("Já existe uma instância do Servidor a correr..."));
        ExitProcess(0);
    }
    
    //buscar as cenas através da linha de comandos
    HKEY regKey = getKey();
    if (regKey == NULL) {
        ExitProcess(0);
    }

    const DWORD limInfFaixas = 1, limSupFaixas = 8;
    const DWORD limInfVelIni = 1, limSupVelIni = 5;

    //num faixas: 1 a 8 inclusive
    //velocidade inicial: 1 a 5 inclusive 
    if (argc == 3) {
        numFaixas = _tcstoul(argv[1], NULL, 0);
        velIniCarros = _tcstoul(argv[2], NULL, 0);

        if (numFaixas < limInfFaixas  || numFaixas > limSupFaixas) {
            TCHAR bufferMessage[64];
            numFaixas = getNumFaixas(regKey);
            errorMessage(hConsole, TEXT("O número de faixas tem que ser entre 1 a 8!"));
            _swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), numFaixas);
            errorMessage(hConsole, bufferMessage);
        }
        else {
            setNumFaixas(regKey, numFaixas);
        }
        if (velIniCarros < limInfVelIni || velIniCarros > limSupVelIni) {
            TCHAR bufferMessage[512];
            velIniCarros = getVelIniCarros(regKey);
            errorMessage(hConsole, TEXT("O número da velocidade inicial do carro tem que ser entre 1 e 5!"));
            _swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), velIniCarros);
            errorMessage(hConsole, bufferMessage);
        }
        else {
            setVelIniCarros(regKey, velIniCarros);
        }
    }
    else {
        velIniCarros = getVelIniCarros(regKey);  //registry
        if (argc == 2) {
            //numFaixas = (DWORD)argv[1];
            numFaixas = _tcstoul(argv[1], NULL, 0);
            if (numFaixas < limInfFaixas || numFaixas > limSupFaixas) {
				TCHAR bufferMessage[64];
				numFaixas = getNumFaixas(regKey);
				errorMessage(hConsole, TEXT("O número de faixas tem que ser entre 1 a 8!"));
				_swprintf_p(bufferMessage, 64, _T("Usando os valores por default: %d"), numFaixas);
				errorMessage(hConsole, bufferMessage);
            }
            else {
                setNumFaixas(regKey, numFaixas);
            }
        }
        else {
            numFaixas = getNumFaixas(regKey);
        }
    }
    CloseHandle(regKey);
    SharedMemory *shared = NULL;
    if(!setupServer(hConsole, numFaixas, velIniCarros, shared)){
        errorMessage(hConsole, TEXT("Erro ao dar setup do servidor!"));
        CloseHandle(hConsole);
        ExitProcess(0);
    }

    //game = game.c/.h has frog, lanes, start, finish, points
    //points = points.c/.h
    //frog = frog.c/.h
    //lanes = lanes.c/.h inside cars.c/.h

    int closeProg = 0;
    //fd_set selectParams;
	//int fdFIFOBACKEND = setupBaseFifo(hConsole);
    //int startTime = time(NULL);
    //_tprintf_s(TEXT("\nStartup complete.\n\nCommand :> \n"));
    do {
        _tprintf_s(_T("\nCommand :> "));
        readCommands(&closeProg, hConsole);
        /*FD_ZERO(&selectParams);
        FD_SET(stdin, &selectParams);
        select(0, &selectParams, NULL, NULL, NULL);
        if (FD_ISSET(stdin, &selectParams)) {
            readCommands(&closeProg);
            if (closeProg == 0)
                _tprintf_s(TEXT("\nCommand :> \n"));
        }
		if(FD_ISSET(fdFIFOBACKEND, &selectParams)){
			int frontendPid = receiveLogin(fdFIFOBACKEND);
			int result;
			if(frontendPid > -1){
				TDADOS *dados;
				dados = malloc(sizeof(TDADOS));
				dados->fdFrontend = setupFrontendFifo(frontendPid);
				char *username = getLastUsername();
				if(dados->fdFrontend != -1){
					dados->fdThread = setupThreadFifo(username);
					if(dados->fdThread == -1){
						logoutUser(username);
						close(dados->fdFrontend);
						removeFrontendFifo(frontendPid);
						removeThreadFifo(username);
					}
					else{
						strcpy(dados->username, username);
						dados->frontendPid = frontendPid;
						dados->th = addthList();
						if(pthread_create(dados->th, NULL, &threadRoutine, dados) == 0){
							result = 0;
							addfdThread(dados->fdThread, dados->username);
							write(dados->fdThread, &result, sizeof(int));
							printf("\nThread has launched properly\n\nCommand :> \n");
						}
						else{
							logoutUser(username);
							result = 1;
							write(dados->fdThread, &result, sizeof(int));
							close(dados->fdFrontend);
							close(dados->fdThread);
							removeFrontendFifo(frontendPid);
							removeThreadFifo(username);
						}
					}
				}
				else
					logoutUser(username);
			}
			else{
				char *username = getLastUsername();
				logoutUser(username);
			}
        if (startTime < time(NULL)) {
            startTime = time(NULL);
            //func de instantes
        }*/
        
    } while (closeProg == 0);

	return 0;
}