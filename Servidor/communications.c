#include "communications.h"

HANDLE setupFifoMovement(int pid){
	TCHAR buffer[64];
    _swprintf_p(buffer, 64, FIFOFROGMOVEMENT, pid);
	return CreateNamedPipe(buffer, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(enum ResponseMovement), sizeof(enum Movement), 1000, NULL);
}

HANDLE setupFifoMap(int pid){
	TCHAR buffer[64];
    _swprintf_p(buffer, 64, FIFOFROGMAP, pid);
	return CreateNamedPipe(buffer, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(CLIENTMAP), 0, 1000, NULL);
}

