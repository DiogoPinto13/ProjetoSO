#include "communications.h"

HANDLE setupFifoMovement(int pid, HANDLE hConsole){
	TCHAR buffer[64];
    _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, pid);
	return CreateNamedPipe(buffer, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(enum ResponseMovement), sizeof(enum Movement), 1000, NULL);
}

HANDLE setupFifoMap(int pid, HANDLE hConsole){
	TCHAR buffer[64];
    _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, pid);
	return CreateNamedPipe(buffer, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(CLIENTMAP), sizeof(int), 1000, NULL);
}

/*int setupBaseFifo(HANDLE console) {
	if (mkfifo(FIFOBACKEND, 0600) == -1) {
		errorMessage(console, "Error creating Server Fifo.");
		exit(-1);
	}
	return open(FIFOBACKEND, O_RDWR);
}

int setupFrontendFifo(int pid, HANDLE console) {
	char fifoname[32];
	sprintf_s(fifoname, FIFOFROG, pid);
	if (mkfifo(fifoname, 0600) == -1) {
		if (errno == EEXIST) {
			errorMessage(console, "Fifo already exists.");
		}
		errorMessage(console, "Error while creating the Frontend Fifo.");
		return -1;
	}
	return open(fifoname, O_RDWR);
}

int setupThreadFifo(int pid, HANDLE console) {
	char fifoname[32];
	sprintf_s(fifoname, FIFOTHREAD, pid);
	if (mkfifo(fifoname, 0600) == -1) {
		if (errno == EEXIST) {
			errorMessage(console, "Fifo already exists.");
		}
		errorMessage(console, "Error while creating the Thread Fifo.");
		return -1;
	}
	return open(fifoname, O_WRONLY);
}

void removeFrogFifo(int pid, HANDLE console) {
	char fifoname[32];
	sprintf_s(fifoname, FIFOFROG, pid);
	unlink(fifoname);
	return;
}

void removeThreadFifo(int pid, HANDLE console) {
	char fifoname[32];
	sprintf_s(fifoname, FIFOTHREAD, pid);
	unlink(fifoname);
	return;
}

int receiveRequest(int fdFIFOBACKEND, boolean *isMultiplayer, HANDLE console) {
	InitReq request;
	if (read(fdFIFOBACKEND, &request, sizeof(InitReq)) < 1) {
		errorMessage(console, "Error while reading from Backend Fifo.");
		return -1;
	}
	*isMultiplayer = request.isMultiplayer;
	return request.pid;
}*/
