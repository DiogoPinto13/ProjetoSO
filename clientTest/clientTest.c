#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>
#include <io.h>
#include <time.h>

#define FIFOBACKEND _T("\\\\.\\pipe\\FIFOBACKEND")
#define FIFOFROGMOVEMENT _T("\\\\.\\pipe\\FIFOMOVEMENT%d")
#define FIFOFROGMAP _T("\\\\.\\pipe\\FIFOFROGMAP%d")

typedef struct recvinitreq {
	int pid;
}InitReq;

//User sends movement
enum Movement {
	UP,
	DOWN,
	LEFT,
	RIGHT,
    END
};

//Server responds to movement
enum ResponseMovement {
	OK,
	DIE,
	LOSE,
	WIN
};

int checkIfIsAlreadyRunning(TCHAR* processName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int counter = 0;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return(FALSE);
    }

    do {
        if (!wcscmp(pe32.szExeFile, processName)) {
            counter++;
        }

    } while (Process32Next(hProcessSnap, &pe32));

    return counter;
}


int main(){
    HANDLE hNamedPipe;
    int pid;
    DWORD nBytes;

    if (checkIfIsAlreadyRunning(_T("Servidor.exe")) == 0) {
        _tprintf_s(_T("No server."));
        return 1;
    }

    if (!WaitNamedPipe(FIFOBACKEND, 2000)) {
		_tprintf_s(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), FIFOBACKEND);
        _tprintf_s(TEXT("Error: %d\n"), GetLastError());
		exit(-1);
	}

    hNamedPipe = CreateFile(FIFOBACKEND, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(hNamedPipe == INVALID_HANDLE_VALUE) {
        _tprintf_s(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), FIFOBACKEND);
        _tprintf_s(TEXT("Error: %d\n"), GetLastError());
        exit(-1);
    }
    
    pid = (int) GetProcessId(GetCurrentProcess());
    
    if (!WriteFile(hNamedPipe, &pid, sizeof(int), &nBytes, NULL)){
        _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
        _tprintf_s(TEXT("Error: %d\n"), GetLastError());
    }
    else
        _tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao servidor... (WriteFile)\n"), nBytes);

    if(!ReadFile(hNamedPipe, &pid, sizeof(int), &nBytes, NULL)){
        _tprintf(TEXT("[ERRO] Ler do pipe! (ReadFile)\n"));
        _tprintf_s(TEXT("Error: %d\n"), GetLastError());
    }
    else
        _tprintf(TEXT("[ESCRITOR] Li [%d] do servidor... (WriteFile)\n"), pid);

    if(pid == 0){
        TCHAR buffer[64];

        //Frog movement
        _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, (int) GetProcessId(GetCurrentProcess()));
        if (!WaitNamedPipe(buffer, 2000)) {
            _tprintf_s(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), buffer);
            _tprintf_s(TEXT("Error: %d\n"), GetLastError());
            exit(-1);
        }

        hNamedPipe = CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hNamedPipe == INVALID_HANDLE_VALUE) {
            _tprintf_s(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), buffer);
            _tprintf_s(TEXT("Error: %d\n"), GetLastError());
            exit(-1);
        }

        int user = 0;
        enum Movement action;
        enum ResponseMovement response;
        while(user != 4){
            while(1){
                _tprintf_s(_T("\nPick a number from 0 to 4: "));
                fflush(stdin);
                _tscanf_s(_T("%d"), &user);
                if(user >= 0 && user <= 4){
                    break;
                }
            }
            action = user;
            if (!WriteFile(hNamedPipe, &action, sizeof(enum Movement), &nBytes, NULL)){
                _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
                _tprintf_s(TEXT("Error: %d\n"), GetLastError());
            }
            else
                _tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao servidor... (WriteFile)\n"), nBytes);
            
            if(action != END){
                if(!ReadFile(hNamedPipe, &response, sizeof(enum ResponseMovement), &nBytes, NULL)){
                    _tprintf(TEXT("[ERRO] Ler do pipe! (ReadFile)\n"));
                    _tprintf_s(TEXT("Error: %d\n"), GetLastError());
                }
                else
                    _tprintf(TEXT("[ESCRITOR] Li [%d] do servidor... (WriteFile)\n"), response);
            }
            else
                break;
        }
    }
    return 0;
}

