#pragma once

#include "resource.h"


#define FIFOBACKEND _T("\\\\.\\pipe\\FIFOBACKEND")
#define FIFOFROGMOVEMENT _T("\\\\.\\pipe\\FIFOMOVEMENT%d")
#define FIFOFROGMAP _T("\\\\.\\pipe\\FIFOFROGMAP%d")
#define NAME_CLOSE_EVENT _T("closeEvent")
#define NAME_CLOSE_CLIENTS_EVENT _T("closeClients")

typedef struct recvinitreq {
	int pid;
}InitReq;

typedef struct {
	HANDLE hEventClose;
	HANDLE hEventCloseClient;
}TKILL;

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
    PAUSED,
	WIN
};

//Server send map
typedef struct {
	int numLifes, points, level, numFaixas;
	TCHAR map[10][20];
}CLIENTMAP;

int checkIfIsAlreadyRunning(TCHAR* processName);
