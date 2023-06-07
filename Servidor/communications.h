#pragma once

#include "utils.h"
#include "console.h"

#define FIFOBACKEND "FIFOBACKEND"
#define FIFOTHREAD "FIFOTHREAD%d"
#define FIFOFROG "FIFOFROG%d"

typedef struct recvinitreq {
	int pid;
	BOOL isMultiplayer;
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


/*typedef struct recvmovereq {
	enum Movement movement;
}MoveReq;*/


//enum Movement valor = UP;

//Setup starting Server Pipe
HANDLE setupBaseFifo(HANDLE hConsole);

//Setup Client Pipe
HANDLE setupFrogFifo(int pid, HANDLE hConsole);

//Setup Server Pipe
HANDLE setupThreadFifo(int pid, HANDLE hConsole);

void removeFrogFifo(int pid, HANDLE hConsole);

void removeThreadFifo(int pid, HANDLE hConsole);

int receiveLogin(int fdBACKEND, BOOL* isMultiplayer, HANDLE hConsole);