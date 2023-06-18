#include "lanes.h"

void initLanes(Lane* lanes, SpecialLane* specialLanes, DWORD numFaixas, float velIniCarros) {

	//finishing lane
	specialLanes[0].y = INITIAL_ROW;
	specialLanes[0].caracter = TEXT('-');
	specialLanes[0].isFinish = TRUE;

	//starting lane
	specialLanes[1].y = INITIAL_ROW + numFaixas + 1;
	specialLanes[1].caracter = TEXT('_');
	specialLanes[1].isFinish = FALSE;


	//normal lanes (do meio)
	srand(time(NULL));
	//faixas
	for (int i = 0; i < numFaixas; i++) {
		lanes[i].numOfCars = (rand() % 3) + 1;
		lanes[i].numOfFrogs = 0;
		lanes[i].isReverse = (BOOL) (rand() % 2 + 1) == 1 ? TRUE : FALSE;
		lanes[i].velCarros = velIniCarros;
		//lanes[i].frogsOnLane = NULL;
		lanes[i].hasObstacle = FALSE;
		lanes[i].y = INITIAL_ROW + 1 + i;
		//lanes[i].obstacle = NULL;
		//carros de cada faixa
		for (int j = 0; j < lanes[i].numOfCars; j++) {
			lanes[i].cars[j].symbol = TEXT('C');
			//evitar spawnarem em cima uns dos outros (carros)
			int randomPosition, contador = 0;
			do {
				contador = 0;
				randomPosition = (rand() % COLUMN_SIZE + 1);
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
}

BOOL checkIfCarInFront(Lane *lane, int carPos){
    for(int i = 0; i < lane->numOfCars; i++){
        if(lane->cars[i].x == carPos){
            return TRUE;
        }
    }
    return FALSE;
}

BOOL moveCars(Lane* lane, Frog* frogs, int numFrogs, int startingLaneRow, HANDLE hEventUpdateStartingLane){
    if(lane->isReverse){
        for(int i = 0; i < lane->numOfCars; i++){
			if(lane->hasObstacle){
				if((lane->cars[i].x - 1) != lane->obstacle.x){
					if(!checkIfCarInFront(lane, lane->cars[i].x - 1)){
						if(lane->cars[i].x == 0)
							lane->cars[i].x = COLUMN_SIZE - 1;
						else
							lane->cars[i].x--;
					}
				}
			}
			else{
				if(!checkIfCarInFront(lane, lane->cars[i].x - 1)){
					if(lane->cars[i].x == 0)
						lane->cars[i].x = COLUMN_SIZE - 1;
					else
						lane->cars[i].x--;
				}
			}
        }
    }
    else{
        for(int i = 0; i < lane->numOfCars; i++){
			if(lane->hasObstacle){
				if((lane->cars[i].x + 1) != lane->obstacle.x){
					if(!checkIfCarInFront(lane, lane->cars[i].x + 1)){
						if(lane->cars[i].x == COLUMN_SIZE - 1)
							lane->cars[i].x = 0;
						else
							lane->cars[i].x++;
					}
				}
			}
			else{
				if(!checkIfCarInFront(lane, lane->cars[i].x + 1)){
					if(lane->cars[i].x == COLUMN_SIZE - 1)
						lane->cars[i].x = 0;
					else
						lane->cars[i].x++;
				}
			}
        }
    }
    //verificaçao de colisao com sapos se o sapo tiver parado numa lane
    if(lane->numOfFrogs > 0){
        for(int i = 0; i < lane->numOfCars; i++){
            for(int j = 0; j < lane->numOfFrogs; j++){
                if(lane->cars[i].x == lane->frogsOnLane[j].x){
                    //reset frog position
                    for(int k = 0; k < numFrogs; k++){
                        if(lane->frogsOnLane[j].hNamedPipeMovement == frogs[k].hNamedPipeMovement){
                            if(!resetFrog(&frogs[k], frogs, numFrogs, startingLaneRow)){
                                removeFrog(&lane->frogsOnLane[j]);
                                if(lane->numOfFrogs == 2 && j == 0){
                                    lane->frogsOnLane[0] = lane->frogsOnLane[1];
                                    //removeFrog(&lane->frogsOnLane[1]);
                                }
                                lane->numOfFrogs--;
                                SetEvent(hEventUpdateStartingLane);
                            }
                            else
                                return TRUE;
                        }
                    }
                    //return resetFrog(&lane->frogsOnLane[j], startingLaneRow);
                }
            }
        }
    }
    return FALSE;
}

void removeFromLane(Lane *lane, Frog* frog){
	//Frog *frogAux = malloc(sizeof(Frog));
	for(int i = 0; i < lane->numOfFrogs; i++){
		if(frog->hNamedPipeMovement == lane->frogsOnLane[i].hNamedPipeMovement){
			removeFrog(&lane->frogsOnLane[i]);
			/*lane->frogsOnLane[i].x = 0;
			lane->frogsOnLane[i].y = 0;
			lane->frogsOnLane[i].hNamedPipeMovement = NULL;
			lane->frogsOnLane[i].hNamedPipeMap = NULL;
			lane->frogsOnLane[i].symbol = _T('S');
			lane->frogsOnLane[i].points = 0;
			lane->frogsOnLane[i].level = 0;
			lane->frogsOnLane[i].currentLifes = 0;
			lane->frogsOnLane[i].isDead = TRUE;*/
			if(lane->numOfFrogs == 2 && i == 0){
				lane->frogsOnLane[i] = lane->frogsOnLane[1];
				removeFrog(&lane->frogsOnLane[1]);
				lane->numOfFrogs--;
				//free(frogAux);
				return;
			}
		}
	}
	//free(frogAux);
	lane->numOfFrogs--;
}
