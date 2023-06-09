#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros){
	
	list->estado = FALSE;
	list->numFaixas = (int)numFaixas;
	list->numFrogs = 0;
	initLanes(list->lanes, list->specialLanes, numFaixas, velIniCarros);
	initFrog(list->frogs, &list->frogs[0], &list->numFrogs, list->specialLanes[1].y);
	initFrog(list->frogs, &list->frogs[1], &list->numFrogs, list->specialLanes[1].y);
	return;
}
