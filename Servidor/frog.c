#include "frog.h"

void initFrog(Frog *frogs, Frog *frog, int *numFrogs, int startingRow) {

	srand(time(NULL));
	(*numFrogs)++;
	frog->currentLifes = LIFES;
	frog->level = 0;
	frog->points = 0;
	frog->symbol = TEXT('S');
	frog->x = rand() % 20;
	//to avoid spawning in the same place than the previous frog
	if (*numFrogs != 1) {
		while (frogs[0].x == frog->x) {
			frog->x = rand() % 20;
		}
	}
	frog->isDead = FALSE;
	frog->y = startingRow;
}

//reset postion and check for lifes, if life = 0 return TRUE
BOOL resetFrog(Frog *frog, int specialLaneStart){
	frog->currentLifes--;
	frog->y = specialLaneStart;
	frog->x = rand() % 20;
	return frog->currentLifes == 0 ? TRUE : FALSE;
}

void removeFrog(Frog *frog){
	frog->currentLifes = 0;
	frog->level = 0;
	frog->points = 0;
	frog->symbol = TEXT('S');
	frog->x = 0;
	frog->y = 0;
	frog->hNamedPipeMovement = NULL;
	frog->hNamedPipeMap = NULL;
}