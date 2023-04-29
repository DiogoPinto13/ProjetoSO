#include "console.h"

//merda pra este tp 

void errorMessage(TCHAR* errorMessage, HANDLE hConsole) {
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	_ftprintf_s(stderr, TEXT("\n%s\n"), errorMessage);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
