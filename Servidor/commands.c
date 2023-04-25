#include "commands.h"

void readCommands(int *close, HANDLE hConsole){
	TCHAR cmd[64];
	fflush(stdin);
	_tscanf_s(_T("%s"), cmd, 64);
	cmd[_tcsclen(cmd)] = '\0';
	if (_tcscmp(cmd, _T("toggleGameStatus")) == 0)
		cmdToggleGameStatus();
	else if (_tcscmp(cmd, _T("restartGame")) == 0)
		cmdRestartGame();
	else if (_tcscmp(cmd, _T("help")) == 0)
		cmdHelp();
	else if (_tcscmp(cmd, _T("exit")) == 0)
		*close = 1;
	else
		errorMessage(TEXT("Unknown command.\nUse the command 'help' to list the commands.", hConsole));
		//_ftprintf_s(stderr, TEXT("\nUnknown command.\nUse the command 'help' to list the commands.\n"));
}

void cmdToggleGameStatus(){
	_tprintf_s(_T("\nEntered the toggleGameStatus command.\n"));
	return;
}

void cmdRestartGame() {
	_tprintf_s(_T("\nEntered the restartGame command.\n"));
	return;
}
void cmdHelp(){
	_tprintf_s(_T("\nList of Commands:\n"));
	_tprintf_s(_T("'toggleGameStatus' -> Pauses/Resumes ongoing game\n"));
	_tprintf_s(_T("'restartGame' -> Restarts ongoing game\n"));
	_tprintf_s(_T("'exit' -> Exits the program\n"));
	return;
}

