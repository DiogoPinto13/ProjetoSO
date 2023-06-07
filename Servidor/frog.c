#include "frog.h"


void initFrog(Frog *frogs, Frog *frog, int *numFrogs, int startingRow) {

	//srand(time(NULL));
	(*numFrogs)++;
	frog->currentLifes = LIFES;
	frog->level = 0;
	frog->points = 0;
	frog->symbol = TEXT('S');
	frog->x = rand() % 20;
	if (*numFrogs != 1) {
		while (frogs[0].x == frog->x) {
			frog->x = rand() % 20;
		}
	}
	frog->y = startingRow;
}
