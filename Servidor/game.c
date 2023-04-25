#include "game.h"

Game* initNode() {
	Game *node = malloc(sizeof(Game));
	return node;
}

Game* newNode(Game *list, int *gameNum) {
	Game* node = malloc(sizeof(Game));
	list->next = node;
	node->behind = list;
	(*gameNum)++;
	return node;
}

void deleteNode(Game *dontDelNode, Game* delNode, int *gameNum) {
	dontDelNode->behind = NULL;
	dontDelNode->next = NULL;
	free(delNode);
	(*gameNum)--;
	return;
}

void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros, int frogPipe){
	Lane* lanes = malloc(sizeof(Lane) * numFaixas);
	list->lanes = lanes;
	if(!initLanes(lanes, numFaixas, velIniCarros)){
		//error
	}


}
