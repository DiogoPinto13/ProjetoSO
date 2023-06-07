#include "commands.h"

TCHAR* readCommands(int *close, int numActiveLanes, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL){
	TCHAR cmd[64], *token = NULL, *nextToken = NULL, timer[64];
	fflush(stdin);
	_tscanf_s(_T("%s"), cmd, 64);
	cmd[_tcsclen(cmd)] = '\0';
	if (_tcscmp(cmd, _T("pause")) == 0){
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        int timerVal = _ttoi(timer);
        if(timerVal != 0 && timerVal > 0){
            cmdPause(timerVal, hConsole, SetMessageFunc, hSemReadBuffer, hSemWriteBuffer, dllHandle, hMutexDLL);
            return _T("Timer set.\nPress Enter to continue.");
        }
        else
            errorMessage(_T("Invalid timer input.\nPress Enter to continue."), hConsole);
    }
	else if (_tcscmp(cmd, _T("addObstacle")) == 0){
        TCHAR *temp[2];
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        if(_tcscmp(timer, _T(" ")) != 0){
            token = _tcstok_s(timer, _T(" "), &nextToken);
            if(_tcscmp(timer, _T("")) != 0){
                if(token != NULL){
                    temp[0] = token;
                    token = _tcstok_s(NULL, _T(" "), &nextToken);
                    if(token == NULL){
                        errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>\nPress Enter to continue."), hConsole);
                    }
                    else{
                        temp[1] = token;
                        int numLane =_ttoi(temp[0]), x = _ttoi(temp[1]);
                        if(numLane >= 0 || x >= 0){
                            cmdAddObstacle(numLane, x, hConsole, SetMessageFunc, hSemReadBuffer, hSemWriteBuffer, dllHandle, hMutexDLL);
                            return _T("Obstacle set.\nPress Enter to continue.");
                        }
                        else
                            errorMessage(_T("Valores inválidos.\naddObstacle <numLane> <numColumn>\nPress Enter to continue."), hConsole);
                        }
                }
                else{
                    errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>\nPress Enter to continue."),hConsole);
                }
            }
            else
                errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>\nPress Enter to continue."),hConsole);
        }
        else
            errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>\nPress Enter to continue."),hConsole);
    }
    else if (_tcscmp(cmd, _T("invertLane")) == 0){
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        int laneVal = _ttoi(timer);
        if(laneVal != 0 && (laneVal >= 0 && laneVal < numActiveLanes))
            cmdInvertLane(laneVal, hConsole, SetMessageFunc, hSemReadBuffer, hSemWriteBuffer, dllHandle, hMutexDLL);
        else
            errorMessage(_T("Invalid lane input.\nPress Enter to continue."),hConsole);
    }
	else if (_tcscmp(cmd, _T("help")) == 0){
        cmdHelp();
        _fgetts(timer, 64, stdin);
    }
	else if (_tcscmp(cmd, _T("exit")) == 0)
		*close = 1;
	else
		errorMessage(TEXT("Unknown command.\nUse the command 'help' to list the commands.\nPress Enter to continue."), hConsole);
		//_ftprintf_s(stderr, TEXT("\nUnknown command.\nUse the command 'help' to list the commands.\n"));
    return NULL;
}

void cmdPause(int time, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL){
	BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("pause"));
    cell->param1 = time;
    cell->param2 = -1;
    WaitForSingleObject(hSemWriteBuffer, INFINITE);
    WaitForSingleObject(hMutexDLL, INFINITE);
    SetMessageFunc(cell);
    ReleaseSemaphore(hSemReadBuffer, 1, NULL);
    //SetEvent(hEventUpdateBuffer);
    ReleaseMutex(hMutexDLL);
    free(cell);
	return;
}

void cmdAddObstacle(int numLane, int x, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL){
    BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("addObstacle"));
    cell->param1 = numLane;
    cell->param2 = x;
    WaitForSingleObject(hSemWriteBuffer, INFINITE);
    WaitForSingleObject(hMutexDLL, INFINITE);
    SetMessageFunc(cell);
    ReleaseSemaphore(hSemReadBuffer, 1, NULL);
    //SetEvent(hEventUpdateBuffer);
    ReleaseMutex(hMutexDLL);
	free(cell);
    return;
}

void cmdInvertLane(int numLane, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hSemReadBuffer, HANDLE hSemWriteBuffer, HANDLE dllHandle, HANDLE hMutexDLL){
    BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("invertLane"));
    cell->param1 = numLane;
    cell->param2 = -1;
    WaitForSingleObject(hSemWriteBuffer, INFINITE);
    WaitForSingleObject(hMutexDLL, INFINITE);
    SetMessageFunc(cell);
    ReleaseSemaphore(hSemReadBuffer, 1, NULL);
    //SetEvent(hEventUpdateBuffer);
    ReleaseMutex(hMutexDLL);
    free(cell);
    return;
}

void cmdHelp(){
	_tprintf_s(_T("\nList of Commands:\n"));
	_tprintf_s(_T("'pause <time>' -> Pauses ongoing game for x seconds\n"));
	_tprintf_s(_T("'addObstacle <numLane> <numColumn>' -> Adds an obstacle in a specific lane and column\n"));
	_tprintf_s(_T("'invertLane <numLane>' -> Inverts a specific lane\n"));
    _tprintf_s(_T("'exit' -> Exits the program\nPress Enter to continue."));
	return;
}

