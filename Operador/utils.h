#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <Tlhelp32.h>
#include <io.h>
#include <time.h>

#define BUFFER_SIZE 20
#define COMMAND_SIZE 64
#define INITIAL_ROW	5
#define INITIAL_COLUMN 5

typedef struct {
    int x;
    TCHAR caracter;
}Obstacle;

typedef struct car {
    int x;
    TCHAR symbol;
}Car;

typedef struct frog {
    int x, y;
    TCHAR symbol;
    HANDLE hNamedPipeMovement, hNamedPipeMap;
    int points, level, currentLifes;
    BOOL isDead;
}Frog;

typedef struct {
    Car cars[8];
    int numOfCars, numOfFrogs;
    int y;  //y para escrever os carros (consola)
    Obstacle obstacle;  //assumimos que só pode haver um obstaculo por faixa
    DWORD velCarros;
    BOOL isReverse;
    Frog *frogsOnLane;
}Lane;

typedef struct {
    int y;
    TCHAR caracter;
    BOOL isFinish;
}SpecialLane;  //starting and finishing lane

typedef struct game {
    Lane lanes[8];
    SpecialLane specialLanes[2];
    DWORD timer; 
    int numFrogs, numFaixas;
    Frog frogs[2];
    BOOL estado;
}Game;

typedef struct {
    TCHAR command[COMMAND_SIZE];
    int param1, param2;
}BufferCell;

typedef struct {
    BufferCell buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
}CircularBuffer;

typedef struct{
    CircularBuffer buffer;
    HANDLE hMutexDLL;
    HANDLE hMutexBuffer;
    HANDLE hSemRead;    //semaforos
    HANDLE hSemWrite;   //semaforos
    Game game;
}SharedMemory;

int checkIfIsAlreadyRunning(TCHAR *processName);

