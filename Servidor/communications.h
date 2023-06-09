#pragma once

#include "utils.h"
#include "console.h"

#define FIFOBACKEND _T("\\\\.\\pipe\\FIFOBACKEND")
#define FIFOFROGMOVEMENT _T("\\\\.\\pipe\\FIFOMOVEMENT%d")
#define FIFOFROGMAP _T("\\\\.\\pipe\\FIFOFROGMAP%d")

typedef struct recvinitreq {
	int pid;
}InitReq;

//User sends movement
enum Movement{
	UP,
	DOWN,
	LEFT,
	RIGHT
};


//Server responds to movement
enum ResponseMovement{
	OK,
	DIE,
	LOSE,
	WIN
};

typedef struct {
	int numLifes, points, level;
	TCHAR map[10][20];
}CLIENTMAP;


/*typedef struct recvmovereq {
	enum Movement movement;
}MoveReq;*/


//enum Movement valor = UP;


//Setup Client Pipe
HANDLE setupFifoMovement(int pid, HANDLE hConsole);

//Setup Server Pipe
HANDLE setupFifoMap(int pid, HANDLE hConsole);

void removeFrogFifo(int pid, HANDLE hConsole);

void removeThreadFifo(int pid, HANDLE hConsole);

int receiveLogin(int fdBACKEND, BOOL* isMultiplayer, HANDLE hConsole);