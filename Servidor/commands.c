#include "commands.h"

void readCommands(int *close, HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, HANDLE hEventUpdateStartingLane, HANDLE hEventUpdateFinishingLane, DWORD numFaixas, DWORD velIniCarros){
	TCHAR cmd[64];
	fflush(stdin);
	_tscanf_s(_T("%s"), cmd, 64);
	cmd[_tcsclen(cmd)] = '\0';
	if (_tcscmp(cmd, _T("toggleGameStatus")) == 0)
		cmdToggleGameStatus(hConsole, dllHandle, hMutexDLL, numFaixas);
	else if (_tcscmp(cmd, _T("restartGame")) == 0)
		cmdRestartGame(hConsole, dllHandle, hMutexDLL, hEventUpdateStartingLane, hEventUpdateFinishingLane, numFaixas, velIniCarros);
	else if (_tcscmp(cmd, _T("help")) == 0)
		cmdHelp();
	else if (_tcscmp(cmd, _T("exit")) == 0)
		*close = 1;
	else
		errorMessage(TEXT("Unknown command.\nUse the command 'help' to list the commands."), hConsole);
		//_ftprintf_s(stderr, TEXT("\nUnknown command.\nUse the command 'help' to list the commands.\n"));
}

void cmdToggleGameStatus(HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, DWORD numFaixas){
	_tprintf_s(_T("\nEntered the toggleGameStatus command.\n"));
	SharedMemory shared;
	WaitForSingleObject(hMutexDLL, INFINITE);
	if(!getMap(hConsole, dllHandle, &shared)) {
		errorMessage(_T("Erro ao ir buscar a memoria partilhada!"), hConsole);
		ReleaseMutex(hMutexDLL);
		return;
	}
	if(shared.game.estado)
		shared.game.estado = FALSE;
	else
		shared.game.estado = TRUE;
	if(!updateMap(hConsole, dllHandle, &shared)) {
		errorMessage(_T("Erro ao dar update da memoria partilhada!"), hConsole);
		ReleaseMutex(hMutexDLL);
		return;
	}
	ReleaseMutex(hMutexDLL);
	return;
}

void cmdRestartGame(HANDLE hConsole, HANDLE dllHandle, HANDLE hMutexDLL, HANDLE hEventUpdateStartingLane, HANDLE hEventUpdateFinishingLane, DWORD numFaixas, DWORD velIniCarros) {
	_tprintf_s(_T("\nEntered the restartGame command.\n"));
	SharedMemory shared;
	WaitForSingleObject(hMutexDLL, INFINITE);
	if(!getMap(hConsole, dllHandle, &shared)) {
		errorMessage(_T("Erro ao ir buscar a memoria partilhada!"), hConsole);
		ReleaseMutex(hMutexDLL);
		return;
	}
	initGame(&shared.game, numFaixas, velIniCarros);
	if(!updateMap(hConsole, dllHandle, &shared)) {
		errorMessage(_T("Erro ao dar update da memoria partilhada!"), hConsole);
		ReleaseMutex(hMutexDLL);
		return;
	}
	ReleaseMutex(hMutexDLL);
	SetEvent(hEventUpdateStartingLane);
	SetEvent(hEventUpdateFinishingLane);
	return;
}
void cmdHelp(){
	_tprintf_s(_T("\nList of Commands:\n"));
	_tprintf_s(_T("'toggleGameStatus' -> Pauses/Resumes ongoing game\n"));
	_tprintf_s(_T("'restartGame' -> Restarts ongoing game\n"));
	_tprintf_s(_T("'exit' -> Exits the program\n"));
	return;
}

