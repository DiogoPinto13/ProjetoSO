#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros){
	
	list->estado = TRUE;
	list->numFaixas = (int)numFaixas;
	initLanes(list->lanes, list->specialLanes, numFaixas, velIniCarros);
	initFrog(list->frogs[0], list->numFrogs, list->specialLanes[1].y);
	initFrog(list->frogs[1], list->numFrogs, list->specialLanes[1].y);

	return;
}
