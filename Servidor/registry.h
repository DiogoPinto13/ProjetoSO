#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 200

DWORD getNumFaixas(HKEY key);

DWORD getVelIniCarros(HKEY key);

void setVelIniCarros(HKEY key, DWORD velIniCarros);

void setNumFaixas(HKEY key, DWORD numFaixas);

HKEY getKey();