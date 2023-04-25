#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#define FIFOBACKEND "FIFOBACKEND"
#define FIFOTHREAD "FIFOTHREAD%d"
#define FIFOFROG "FIFOFROG%d"

typedef struct recvinitreq {
	int pid;
	boolean isMultiplayer;
}InitReq;

int setupBaseFifo(HANDLE console);

int setupFrogFifo(int pid, HANDLE console);

int setupThreadFifo(int pid, HANDLE console);

void removeFrogFifo(int pid, HANDLE console);

void removeThreadFifo(int pid, HANDLE console);

int receiveLogin(int fdBACKEND, boolean* isMultiplayer, HANDLE console);