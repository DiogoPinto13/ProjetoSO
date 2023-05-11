#pragma once

#include "utils.h"
#include "console.h"

#define TAM 200

DWORD getNumFaixas(HKEY key);

DWORD getVelIniCarros(HKEY key);

void setVelIniCarros(HKEY key, DWORD velIniCarros);

void setNumFaixas(HKEY key, DWORD numFaixas);

HKEY getKey();

BOOL initRegistry(int argc, TCHAR **argv, DWORD *numFaixas, DWORD *velIniCarros, HANDLE hConsole);