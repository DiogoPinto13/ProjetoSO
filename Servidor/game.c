#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros, int frogPipe){
	
	initLanes(list->lanes, list->specialLanes, numFaixas, velIniCarros);
	initFrog(list->frogs[0], list->numFrogs, list->specialLanes[1].y);

	return;
}
