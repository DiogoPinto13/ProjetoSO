#include "game.h"


void initGame(Game *list, DWORD numFaixas, DWORD velIniCarros){
	srand(time(NULL));
	list->estado = FALSE;
	list->numFaixas = (int)numFaixas;
	list->numFrogs = 0;
	initLanes(list->lanes, list->specialLanes, numFaixas, (float)velIniCarros);
	return;
}

void passToTheNextLevel(Game *game, Frog *frog){
	frog->level++;
	frog->points += 10;
	frog->currentLifes = LIFES;

	int randomIntAux = rand();
	float multiplier = 0.8 + ((double)randomIntAux / RAND_MAX) * 1.7;

	initLanes(game->lanes, game->specialLanes, game->numFaixas, multiplier);

}

void removeFromLane(Lane *lane, Frog* frog){
	Frog *frogAux = malloc(sizeof(Frog));
	for(int i = 0; i < lane->numOfFrogs; i++){
		if(frog == &lane->frogsOnLane[i]){
			lane->frogsOnLane[i] = *frogAux;
			if(lane->numOfFrogs == 2 && i == 0){
				lane->frogsOnLane[i] = lane->frogsOnLane[i + 1];
				lane->numOfFrogs--;
				return;
			}
		}
	}
	free(frogAux);
	lane->numOfFrogs--;
}

void addToLane(Lane *lane, Frog* frog){
	lane->frogsOnLane[lane->numOfFrogs] = *frog;
	lane->numOfFrogs++;
}

enum ResponseMovement moveFrog(Game* game, Frog* frog, enum Movement action) {
	switch (action) {
		//frog->y - 1 - INITIAL_ROW é o indice da lane em que ele está
		case UP:
			if ((frog->y - 1) == game->specialLanes[0].y) {
				removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				frog->y--;
				//Ele está a ir para o lane final
				//passToTheNextLevel(game, frog);
				return WIN;
			}
			else{
				//verificar se vai bater num carro
				if(frog->y == game->specialLanes[1].y){
					//tem de fazer uma conta diferente porque está fora das lanes de carros
					for(int i = 0; i < game->lanes[game->numFaixas - 1].numOfCars; i++){
						if (game->lanes[game->numFaixas - 1].cars[i].x == frog->x) {
							//check lifes and reset frog
							return DIE;
						}
					}
					if(frog->x == game->lanes[game->numFaixas - 1].obstacle.x){
						return OK;
					}
				}
				else{
					//frog->y - 2 - INITIAL_ROW é o indice da lane em cima
					for (int i = 0; i < (game->lanes[frog->y - 2 - INITIAL_ROW].numOfCars); i++) {
						if (game->lanes[frog->y - 2 - INITIAL_ROW].cars[i].x == frog->x) {
							return DIE;
						}
					}
					if(frog->x == game->lanes[frog->y - 2 - INITIAL_ROW].obstacle.x){
						return OK;
					}
					removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				}
				//mexer
				addToLane(&game->lanes[frog->y - 2 - INITIAL_ROW], frog);
				frog->y--;
			}
			break;
		case DOWN:
			if((frog->y + 1) == game->specialLanes[1].y){
				//Ele está a voltar para a lane inicial
				removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				frog->y++;
			}
			else{
				if(frog->y == game->specialLanes[0].y){
					//tem de fazer uma conta diferente porque está fora das lanes de carros
					for (int i = 0; i < (game->lanes[0].numOfCars); i++) {
						if (game->lanes[0].cars[i].x == frog->x) {
							return DIE;
						}
					}
					if(frog->x == game->lanes[0].obstacle.x){
						return OK;
					}
				}
				else{
					//frog->y - INITIAL_ROW é o indice da lane em baixo
					for (int i = 0; i < (game->lanes[frog->y - INITIAL_ROW].numOfCars); i++) {
						if (game->lanes[frog->y - INITIAL_ROW].cars[i].x == frog->x) {
							return DIE;
						}
					}
					if(frog->x == game->lanes[frog->y - INITIAL_ROW].obstacle.x){
						return OK;
					}
					removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				}
				addToLane(&game->lanes[frog->y - INITIAL_ROW], frog);
				frog->y++;
			}
			break;
		case LEFT:
			//primeira coluna
			if (frog->x == 0) {
				//está na borda da esquerda
				return OK;
			}
			if(frog->y == game->specialLanes[0].y || frog->y == game->specialLanes[1].y){
				frog->x--;
			}
			else{
				for (int i = 0; i < (game->lanes[frog->y - 1 - INITIAL_ROW].numOfCars); i++) {
					if (game->lanes[frog->y - 1 - INITIAL_ROW].cars[i].x == frog->x - 1) {
						return DIE;
					}
				}
				if(frog->x - 1 == game->lanes[frog->y - 1 - INITIAL_ROW].obstacle.x){
					return OK;
				}
				frog->x--;
			}
			break;
		case RIGHT:
			//ultima coluna
			if (frog->x == 19) {
				//está na borda da direita
				return OK;
			}
			if(frog->y == game->specialLanes[0].y || frog->y == game->specialLanes[1].y){
				frog->x++;
			}
			else{
				for (int i = 0; i < (game->lanes[frog->y - 1 - INITIAL_ROW].numOfCars); i++) {
					if (game->lanes[frog->y - 1 - INITIAL_ROW].cars[i].x == frog->x + 1) {
						return DIE;
					}
				}
				if(frog->x + 1 == game->lanes[frog->y - 1 - INITIAL_ROW].obstacle.x){
					return OK;
				}
				frog->x++;
			}
			break;
		default:
			return OK;
	}
	return OK;
}