#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros){
	srand(time(NULL));
	list->estado = FALSE;
	list->numFaixas = (int)numFaixas;
	list->numFrogs = 0;
	initLanes(list->lanes, list->specialLanes, numFaixas, velIniCarros);
	return;
}
