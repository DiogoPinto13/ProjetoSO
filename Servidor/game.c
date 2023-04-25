#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros, int frogPipe){
	
	initLanes(list->lanes, numFaixas, velIniCarros);

	return;
}
