#pragma once
int getNumFaixas(HKEY key);

int getVelIniCarros(HKEY key);

void setVelIniCarros(HKEY key, DWORD velIniCarros);

void setNumFaixas(HKEY key, DWORD numFaixas);

HKEY getKey();