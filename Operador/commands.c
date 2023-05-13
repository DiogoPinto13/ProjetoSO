#include "commands.h"

void readCommands(int *close, HANDLE hConsole, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer){
	TCHAR cmd[64], *token = NULL, *nextToken = NULL, timer[64];
	fflush(stdin);
	_tscanf_s(_T("%s"), cmd, 64);
	cmd[_tcsclen(cmd)] = '\0';
	if (_tcscmp(cmd, _T("pause")) == 0){
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        int timerVal = _ttoi(timer);
        if(timerVal != 0)
            cmdPause(timerVal, SetMessageFunc, hEventUpdateBuffer);
        else
            errorMessage(_T("Invalid timer input."),hConsole);
    }
	else if (_tcscmp(cmd, _T("addObstacle")) == 0){
        TCHAR *temp[2];
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        if(_tcscmp(timer, _T(" ")) != 0){
            token = _tcstok_s(timer, _T(" "), &nextToken);
            _tprintf_s(_T("\nToken before: %s"), token);
            if(_tcscmp(timer, _T("")) != 0){
                for(int i = 0; i < 2; i++){
                    if(token != NULL){
                        temp[i] = token;
                        token = _tcstok_s(NULL, _T(" "), &nextToken);
                        _tprintf_s(_T("\nToken: %s"), token);
                    }
                    else{
                        errorMessage(_T("Sintaxe errada."),hConsole);
                    }
                }
                int numLane =_ttoi(temp[0]), x = _ttoi(temp[1]);
                if(numLane < 1 || x < 1)
                    errorMessage(_T("erro"), hConsole);
                else
                    cmdAddObstacle(numLane, x, SetMessageFunc, hEventUpdateBuffer);
            }
            else
                errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>"),hConsole);
        }
        else
                errorMessage(_T("Sintaxe errada.\naddObstacle <numLane> <numColumn>"),hConsole);
    }
    else if (_tcscmp(cmd, _T("invertLane")) == 0){
        _fgetts(timer, 64, stdin);
        timer[_tcsclen(timer) - 1] = '\0';
        int laneVal = _ttoi(timer);
        if(laneVal != 0)
            cmdInvertLane(laneVal, SetMessageFunc, hEventUpdateBuffer);
        else
            errorMessage(_T("Invalid lane input."),hConsole);
    }
	else if (_tcscmp(cmd, _T("help")) == 0)
		cmdHelp();
	else if (_tcscmp(cmd, _T("exit")) == 0)
		*close = 1;
	else
		errorMessage(TEXT("Unknown command.\nUse the command 'help' to list the commands."), hConsole);
		//_ftprintf_s(stderr, TEXT("\nUnknown command.\nUse the command 'help' to list the commands.\n"));
}

void cmdPause(int time, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer){
	BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("pause"));
    cell->param1 = time;
    cell->param2 = -1;
    SetMessageFunc(cell);
    SetEvent(hEventUpdateBuffer);
    free(cell);
	return;
}

void cmdAddObstacle(int numLane, int x, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer){
    BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("addObstacle"));
    //cell->command = _T("addObstacle");
    cell->param1 = numLane;
    cell->param2 = x;
    SetMessageFunc(cell);
    SetEvent(hEventUpdateBuffer);
	free(cell);
    return;
}

void cmdInvertLane(int numLane, SetMessageBufferFunc SetMessageFunc, HANDLE hEventUpdateBuffer){
    BufferCell* cell = malloc(sizeof(BufferCell));
    _tcscpy_s(cell->command, COMMAND_SIZE, _T("invertLane"));
    cell->param1 = numLane;
    cell->param2 = -1;
    SetMessageFunc(cell);
    SetEvent(hEventUpdateBuffer);
    free(cell);
    return;
}

void cmdHelp(){
	_tprintf_s(_T("\nList of Commands:\n"));
	_tprintf_s(_T("'pause <time>' -> Pauses ongoing game for x seconds\n"));
	_tprintf_s(_T("'addObstacle <numLane> <numColumn>' -> Adds an obstacle in a specific lane and column\n"));
	_tprintf_s(_T("'invertLane <numLane>' -> Inverts a specific lane\n"));
    _tprintf_s(_T("'exit' -> Exits the program\n"));
	return;
}

