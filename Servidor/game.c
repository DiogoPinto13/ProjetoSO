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
	//frog que chegou à finishing lane
	frog->points += 10;
	for (int i = 0; i < game->numFrogs; i++) {
		game->frogs[i].level++;
		game->frogs[i].currentLifes = LIFES;
		game->frogs[i].y = game->specialLanes[1].y;
	}

	initLanes(game->lanes, game->specialLanes, game->numFaixas, game->lanes->velCarros * generateRandomNumber((float)1.05, (float)1.3));

}

void addToLane(Lane *lane, Frog *frog) {
	//lane->frogsOnLane[lane->numOfFrogs] = *frog;
	//deep copy
	lane->frogsOnLane[lane->numOfFrogs].x = frog->x;
	lane->frogsOnLane[lane->numOfFrogs].y = frog->y;
	lane->frogsOnLane[lane->numOfFrogs].hNamedPipeMovement = frog->hNamedPipeMovement;
	lane->frogsOnLane[lane->numOfFrogs].hNamedPipeMap = frog->hNamedPipeMap;
	lane->frogsOnLane[lane->numOfFrogs].symbol = frog->symbol;
	lane->frogsOnLane[lane->numOfFrogs].points = frog->points;
	lane->frogsOnLane[lane->numOfFrogs].level = frog->level;
	lane->frogsOnLane[lane->numOfFrogs].currentLifes = frog->currentLifes;
	lane->frogsOnLane[lane->numOfFrogs].isDead = frog->isDead;
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
				passToTheNextLevel(game, frog);
				return WIN;
			}
			else if(frog->y == game->specialLanes[0].y){
				return OK;
			}
			else{
				//verificar se vai bater num carro
				if(frog->y == game->specialLanes[1].y){
					//tem de fazer uma conta diferente porque está fora das lanes de carros
					//starting lane
					for(int i = 0; i < game->lanes[game->numFaixas - 1].numOfCars; i++){
						if (game->lanes[game->numFaixas - 1].cars[i].x == frog->x) {
							//check lifes and reset 
							if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
								return LOSE;
							return DIE;
						}
					}
					if(frog->x == game->lanes[game->numFaixas - 1].obstacle.x){
						return OK;
					}
					frog->y--;
					addToLane(&game->lanes[game->numFaixas - 1], frog);
				}
				else{
					//frog->y - 2 - INITIAL_ROW é o indice da lane em cima
					for (int i = 0; i < (game->lanes[frog->y - 2 - INITIAL_ROW].numOfCars); i++) {
						if (game->lanes[frog->y - 2 - INITIAL_ROW].cars[i].x == frog->x) {
							removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
							if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
								return LOSE;
							return DIE;
						}
					}
					if(frog->x == game->lanes[frog->y - 2 - INITIAL_ROW].obstacle.x){
						return OK;
					}
					removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
					frog->y--;
					addToLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				}
				//mexer
			}
			break;
		case DOWN:
			if((frog->y + 1) == game->specialLanes[1].y){
				//Ele está a voltar para a lane inicial
				removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				frog->y++;
			}
			else if(frog->y == game->specialLanes[1].y){
				return OK;
			}
			else{
				if(frog->y == game->specialLanes[0].y){
					//tem de fazer uma conta diferente porque está fora das lanes de carros
					for (int i = 0; i < (game->lanes[0].numOfCars); i++) {
						if (game->lanes[0].cars[i].x == frog->x) {
							if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
								return LOSE;
							return DIE;
						}
					}
					if(frog->x == game->lanes[0].obstacle.x){
						return OK;
					}
					frog->y++;
					addToLane(&game->lanes[0], frog);
				}
				else{
					//frog->y - INITIAL_ROW é o indice da lane em baixo
					for (int i = 0; i < (game->lanes[frog->y - INITIAL_ROW].numOfCars); i++) {
						if (game->lanes[frog->y - INITIAL_ROW].cars[i].x == frog->x) {
							removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
							if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
								return LOSE;
							return DIE;
						}
					}
					if(frog->x == game->lanes[frog->y - INITIAL_ROW].obstacle.x){
						return OK;
					}
					removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
					frog->y++;
					addToLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
				}
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
						removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
						if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
							return LOSE;
						return DIE;
					}
				}
				if(frog->x - 1 == game->lanes[frog->y - 1 - INITIAL_ROW].obstacle.x){
					return OK;
				}
				for(int i = 0; i < game->lanes[frog->y - 1 - INITIAL_ROW].numOfFrogs; i++) {
					if(game->lanes[frog->y - 1 - INITIAL_ROW].frogsOnLane[i].hNamedPipeMovement == frog->hNamedPipeMovement){
						game->lanes[frog->y - 1 - INITIAL_ROW].frogsOnLane[i].x--;
					}
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
						removeFromLane(&game->lanes[frog->y - 1 - INITIAL_ROW], frog);
						if(resetFrog(frog, game->frogs, game->numFrogs, game->specialLanes[1].y))
							return LOSE;
						return DIE;
					}
				}
				if(frog->x + 1 == game->lanes[frog->y - 1 - INITIAL_ROW].obstacle.x){
					return OK;
				}
				for(int i = 0; i < game->lanes[frog->y - 1 - INITIAL_ROW].numOfFrogs; i++){
					if(game->lanes[frog->y - 1 - INITIAL_ROW].frogsOnLane[i].hNamedPipeMovement == frog->hNamedPipeMovement){
						game->lanes[frog->y - 1 - INITIAL_ROW].frogsOnLane[i].x++;
					}
				}
				frog->x++;
			}
			break;
		default:
			return OK;
	}
	return OK;
}