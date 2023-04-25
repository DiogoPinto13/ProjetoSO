#include "lanes.h"

//quero java
void initLanes(Lane* lanes, DWORD numFaixas, DWORD velIniCarros) {

	srand(time(NULL));
	//faixas
	for (int i = 0; i < numFaixas; i++) {
		lanes[i].numOfCars = rand() % 8 + 1;
		lanes[i].isReverse = (boolean) rand() % 1;
		lanes[i].velCarros = velIniCarros;
		
		//carros de cada faixa
		for (int j = 0; j < lanes[i].numOfCars; j++) {
			lanes[i].cars[j].symbol = TEXT("C");
			//evitar spawnarem em cima uns dos outros (carros)
			int randomPosition, contador = 0;
			do {
				contador = 0;
				randomPosition = j * 2 + (rand() % 45 + 1) + (int)velIniCarros; //para evitar ficarem seguidos
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
