#include "frog.h"


void initFrog(Frog *frog, int *numFrogs, int startingRow) {

	//srand(time(NULL));
	(*numFrogs)++;
	frog->currentLifes = LIFES;
	frog->level = 0;
	frog->points = 0;
	frog->symbol = TEXT('S');
	frog->x = rand() % 20;
	frog->y = startingRow;
}
