#include "lanes.h"

//quero java
void initLanes(Lane* lanes, SpecialLane* specialLanes, DWORD numFaixas, DWORD velIniCarros) {

	//finishing lane
	specialLanes[0].y = INITIAL_ROW;
	specialLanes[0].caracter = TEXT("-");
	specialLanes[0].isFinish = TRUE;

	//starting lane
	specialLanes[1].y = INITIAL_ROW + numFaixas + 1;
	specialLanes[1].caracter = TEXT("_");
	specialLanes[1].isFinish = FALSE;


	//normal lanes (do meio)
	srand(time(NULL));
	//faixas
	for (int i = 0; i < numFaixas; i++) {
		lanes[i].numOfCars = rand() % 8 + 1;
		lanes[i].isReverse = (boolean) rand() % 1;
		lanes[i].velCarros = velIniCarros;
		lanes[i].y = INITIAL_ROW + 1 + i;
		//lanes[i].obstacle = NULL;  //por enquanto kekw
		//carros de cada faixa
		for (int j = 0; j < lanes[i].numOfCars; j++) {
			lanes[i].cars[j].symbol = TEXT("C");
			//evitar spawnarem em cima uns dos outros (carros)
			int randomPosition, contador = 0;
			do {
				contador = 0;
				randomPosition = (rand() % 20 + 1); //para evitar ficarem seguidos
				for (int k = 0; k < j; k++) {
					if (randomPosition == lanes[i].cars[k].x) {
						contador++;
						break;
					}
				}
				if (contador == 0) {
					break;
				}
			} while (1);
			lanes[i].cars[j].x = randomPosition;
		}
	}
;
}
