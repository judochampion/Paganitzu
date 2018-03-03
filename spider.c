#include "game.h"

#include "system.h"

#include "ai.h"

#include "gui.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "spider.h"


int check_if_spider_is_next_player(LevelState* level_state){
int result=0;
//hier wordt code wat ingewikkelder omdat we moeten controleren of player niet op rand van veld staat

if(level_state->player_x<level_state->width) result=(level_state->tiles[level_state->player_x+1][level_state->player_y].type==TILE_SPIDER);
if(level_state->player_y<level_state->height) result=(result || (level_state->tiles[level_state->player_x][level_state->player_y+1].type==TILE_SPIDER));
if(level_state->player_x>0) result=(result || (level_state->tiles[level_state->player_x-1][level_state->player_y].type==TILE_SPIDER));
if(level_state->player_y>0) result=(result || (level_state->tiles[level_state->player_x][level_state->player_y-1].type==TILE_SPIDER));

return result;
}




void automatic_move_spider(LevelState* level_state){
	// we overlopen alle tegels
	int x,y;

	int dir[10]={0,0,0,0,0,0,0,0,0,0};
 
    for (x = 0; x < level_state->width; x++){
        for (y = 0; y < level_state->height; y++){
			if(level_state->tiles[x][y].type == TILE_SPIDER){
				level_state->tiles[x][y].logic.spider.already_moved=0;

			}
		}
	}

    for (x = 0; x < level_state->width; x++){
        for (y = 0; y < level_state->height; y++){
			
			if(level_state->tiles[x][y].type == TILE_SPIDER){

					if(!level_state->tiles[x][y].logic.spider.already_moved){
					if(is_spin_blocked(level_state,x,y)){
					spider_dies(level_state,x,y);
					
					} else{
						if(is_spider_out_of_bounds(level_state,x,y)){move_spider_backward(level_state,x,y);//printf("backwardgeval\n");
						}else{
						
						get_spider_fields(level_state,x,y,dir);
						if(spin_only_backward(level_state,dir)){
						//spin draait 180graden
							
							actually_move_spider(level_state,x,y,dir[6],dir[7]);
							
						}else{//printf("1\n");
							if(level_state->tiles[dir[2]][dir[3]].type==TILE_EMPTY&&level_state->tiles[dir[8]][dir[9]].type!=TILE_EMPTY){
								actually_move_spider(level_state,x,y,dir[2],dir[3]);
								}else{
									if(level_state->tiles[dir[0]][dir[1]].type==TILE_EMPTY){
										actually_move_spider(level_state,x,y,dir[0],dir[1]);
									}else{
										if(level_state->tiles[dir[2]][dir[3]].type==TILE_EMPTY){
											actually_move_spider(level_state,x,y,dir[2],dir[3]);
										}else{
										actually_move_spider(level_state,x,y,dir[4],dir[5]);

										}
									}

							}
						}
						}}
				}// accolade van already_moved-controle
			}//accolade van spider-controle
			// als de tegel geen spin is, hoeven we niets te doen...

		}
	}
}

void  spider_dies(LevelState* level_state, int x, int y){
	/*Deze functie laat de spin sterven (die op coördinaten[x,y] zit) sterven en controleert of er in de omgeving gems moeten worden bijgemaakt*/
	level_state->tiles[x][y].type=TILE_EMPTY;
			if(level_state->tiles[x+1][y].type == TILE_WALL_BREAKABLE || level_state->tiles[x+1][y].type ==TILE_DEBRIS||level_state->tiles[x+1][y].type ==TILE_BOULDER){
			level_state->tiles[x+1][y].type =TILE_GEM;level_state->initial_gem_count++;

					
			}
			if(level_state->tiles[x-1][y].type == TILE_WALL_BREAKABLE || level_state->tiles[x-1][y].type ==TILE_DEBRIS||level_state->tiles[x-1][y].type ==TILE_BOULDER){
			level_state->tiles[x-1][y].type =TILE_GEM;level_state->initial_gem_count++;
					
			}
			if(level_state->tiles[x][y+1].type == TILE_WALL_BREAKABLE || level_state->tiles[x][y+1].type ==TILE_DEBRIS||level_state->tiles[x][y+1].type ==TILE_BOULDER){
			level_state->tiles[x][y+1].type =TILE_GEM;level_state->initial_gem_count++;
					
			}
			if(level_state->tiles[x][y-1].type == TILE_WALL_BREAKABLE || level_state->tiles[x][y-1].type ==TILE_DEBRIS||level_state->tiles[x][y-1].type ==TILE_BOULDER){
			level_state->tiles[x][y-1].type =TILE_GEM;level_state->initial_gem_count++;
					
			}



}

int give_direction(int old_x,int old_y,int new_x,int new_y){
	/* Deze functie geeft volgens de enum move_ een int terug die zegt welke de nieuwe direction is als je het oude en nieuwe veld van de spin/persoon weet*/

	int diff_x=new_x-old_x;
	int diff_y=new_y-old_y;
	int result=0; 
	 
	if(diff_x==1&&diff_y==0) result=1; // oosten
	if(diff_x==0&&diff_y==1) result=2;//noorden
	if(diff_x==-1&&diff_y==0) result=3;//westen
	if(diff_x==0&&diff_y==-1) result=4;//zuiden
	return result;
}

void  get_spider_fields(LevelState* level_state, int spider_x, int spider_y, int* dir){
/* Deze functie geeft de coördinaten van de kandidaatvelden waarnaar de spin wil gaan in een rij terug. 
We willen hier drie velden waar de spin misschien naar toe wil. Veld 1 is het veld recht voor de spin. Veld 2 is het veld waar de spin, 
afhankelijk van zijn driection en het al dan niet clockwise zijn, het liefst naar toe zou draaien. veld 3 is het veld waar de spin het minst 
graag naar toe zou draaien. veld 4 is het veld achter de spin. veld 5 is het kandidaat binnenkantmuurtje waarrond gedraaid zou worden*/
	/* op plaats 0 zit coördinaat x van veld 1, op plaats 1 coördinaat y van veld 1,
	op plaats 2 zit coördinaat x van veld 2, op plaats 3 coördinaat y van veld 2,
	op plaats 4 zit coördinaat x van veld 3, op plaats 5 coördinaat y van veld 3,
	op plaats 6 zit coordinaat x van veld 4, op plaats 7 zit coordinaat y van veld 4
	op plaats 8 zit coordinaat x van veld 5 , op plaats 9 zit coordinaat y van veld 5*/
	
	if(level_state->tiles[spider_x][spider_y].logic.spider.clockwise){
		switch(level_state->tiles[spider_x][spider_y].logic.spider.current_direction){
			case (MOVE_SOUTH): {
				dir[0]=spider_x;
				dir[1]=spider_y-1;
				dir[2]=spider_x-1;
				dir[3]=spider_y;
				dir[4]=spider_x+1;
				dir[5]=spider_y;
				dir[6]=spider_x;
				dir[7]=spider_y+1;
				dir[8]=spider_x-1;
				dir[9]=spider_y+1;
				break;
			}
			case (MOVE_EAST): {
				dir[0]=spider_x+1;
				dir[1]=spider_y;
				dir[2]=spider_x;
				dir[3]=spider_y-1;
				dir[4]=spider_x;
				dir[5]=spider_y+1;
				dir[6]=spider_x-1;
				dir[7]=spider_y;
				dir[8]=spider_x-1;
				dir[9]=spider_y-1;
				break;
			}
			case (MOVE_NORTH): {
				dir[0]=spider_x;
				dir[1]=spider_y+1;
				dir[2]=spider_x+1;
				dir[3]=spider_y;
				dir[4]=spider_x-1;
				dir[5]=spider_y;
				dir[6]=spider_x;
				dir[7]=spider_y-1;
				dir[8]=spider_x+1;
				dir[9]=spider_y-1;
				break;
			}
			case (MOVE_WEST): {
				dir[0]=spider_x-1;
				dir[1]=spider_y;
				dir[2]=spider_x;
				dir[3]=spider_y+1;
				dir[4]=spider_x;
				dir[5]=spider_y-1;
				dir[6]=spider_x+1;
				dir[7]=spider_y;
				dir[8]=spider_x+1;
				dir[9]=spider_y+1;
				break;
			}
			default:break;
		}
	}

	else{
	
		switch(level_state->tiles[spider_x][spider_y].logic.spider.current_direction){
			case (MOVE_SOUTH): {
				dir[0]=spider_x;
				dir[1]=spider_y-1;
				dir[2]=spider_x+1;
				dir[3]=spider_y;
				dir[4]=spider_x-1;
				dir[5]=spider_y;
				dir[6]=spider_x;
				dir[7]=spider_y+1;
				dir[8]=spider_x+1;
				dir[9]=spider_y+1;
				break;
			}
			case (MOVE_EAST): {
				dir[0]=spider_x+1;
				dir[1]=spider_y;
				dir[2]=spider_x;
				dir[3]=spider_y+1;
				dir[4]=spider_x;
				dir[5]=spider_y-1;
				dir[6]=spider_x-1;
				dir[7]=spider_y;
				dir[8]=spider_x-1;
				dir[9]=spider_y+1;
				break;
			}
			case (MOVE_NORTH): {
				dir[0]=spider_x;
				dir[1]=spider_y+1;
				dir[2]=spider_x-1;
				dir[3]=spider_y;
				dir[4]=spider_x+1;
				dir[5]=spider_y;
				dir[6]=spider_x;
				dir[7]=spider_y-1;
				dir[8]=spider_x-1;
				dir[9]=spider_y-1;
				break;
			}
			case (MOVE_WEST): {
				dir[0]=spider_x-1;
				dir[1]=spider_y;
				dir[2]=spider_x;
				dir[3]=spider_y-1;
				dir[4]=spider_x;
				dir[5]=spider_y+1;
				dir[6]=spider_x+1;
				dir[7]=spider_y;
				dir[8]=spider_x+1;
				dir[9]=spider_y-1;
				break;
			}
		}
	}

}

int is_spider_out_of_bounds(LevelState* level_state, int x,int y){
	return x==0||y==0||x==level_state->width-1||y==level_state->height-1;
}


void move_spider_backward(LevelState* level_state,int old_x,int old_y){
	int new_x,new_y;

	switch(level_state->tiles[old_x][old_y].logic.spider.current_direction){
		case MOVE_SOUTH: {new_x=old_x;new_y=old_y+1;break;}
		case MOVE_NORTH: {new_x=old_x;new_y=old_y-1;break;}
		case MOVE_WEST: {new_x=old_x+1;new_y=old_y;break;}
		case MOVE_EAST: {new_x=old_x-1;new_y=old_y;break;}
	}

	actually_move_spider(level_state,old_x,old_y,new_x,new_y);
}

int is_spin_blocked(LevelState* level_state, int x, int y){
	int result=0;

	//we moeten controleren of spin niet op rand van veld staat
	if(x<level_state->width-1) result=(level_state->tiles[x+1][y].type!=TILE_EMPTY);
	if(y<level_state->height-1) result=(result && (level_state->tiles[x][y+1].type!=TILE_EMPTY));
	if(x>0) result=(result && (level_state->tiles[x-1][y].type!=TILE_EMPTY));
	if(y>0) result=(result && (level_state->tiles[x][y-1].type!=TILE_EMPTY));

	return result;	
}

int spin_only_backward(LevelState* level_state,int* dir){
return ((level_state->tiles[dir[0]][dir[1]].type != TILE_EMPTY)
	&&(level_state->tiles[dir[2]][dir[3]].type != TILE_EMPTY)
	&&(level_state->tiles[dir[4]][dir[5]].type != TILE_EMPTY)
	&&(level_state->tiles[dir[6]][dir[7]].type == TILE_EMPTY))?1:0;

}

void actually_move_spider(LevelState* level_state,int old_x,int old_y,int new_x,int new_y){

	level_state->tiles[new_x][new_y].type=TILE_SPIDER;
	level_state->tiles[new_x][new_y].logic.spider.clockwise=level_state->tiles[old_x][old_y].logic.spider.clockwise;
	level_state->tiles[new_x][new_y].logic.spider.current_direction=give_direction(old_x,old_y,new_x,new_y);
	level_state->tiles[new_x][new_y].logic.spider.already_moved=1;							
	level_state->tiles[old_x][old_y].type=TILE_EMPTY;
}