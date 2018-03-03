#include "level.h"
#include "ai.h"
#include "spider.h"

/*
 *
 * level.c: implementation of the level interface.
 *
 * Task: implement the method:
 *       LevelState* read_level(const char* filename)
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#pragma warning( disable : 4996 )

#ifdef WIN32
/* Disable visual studio warnings for fopen and sscanf */
#define _CRT_SECURE_NO_WARNINGS
#endif


const char* type_chars = " E#._OWt|gkPX=";
const char* movenames[5] = { "NO MOVE", "EAST", "NORTH", "WEST", "SOUTH"};
const char* move_to_s(move m) { return movenames[m]; }

LevelState* load_example_level()
{
	int i,j,k;
    tile_content fixed[13][11] = {
        {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL},
        {TILE_WALL, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_PLAYER, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL_BREAKABLE, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL, TILE_SPIDER, TILE_DEBRIS, TILE_EMPTY, TILE_SNAKE, TILE_WALL},
        {TILE_WALL, TILE_WALL, TILE_WALL_BREAKABLE, TILE_EMPTY, TILE_EMPTY, TILE_GEM, TILE_EMPTY, TILE_KEY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_KEY, TILE_DEBRIS, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_WALL, TILE_WALL_BREAKABLE, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_BOULDER, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_BOULDER, TILE_WATER, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_SNAKE, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_GEM, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_EMPTY, TILE_WALL},
        {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_EXIT, TILE_WALL},
        {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL},
        {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL}};
    int x,y;
    LevelState* res = (LevelState*) malloc(sizeof(LevelState)); //ook hier cast aangepast
    res->name = "Static test level";
    res->height = 11;
    res->width = 13;
    res->tiles = (TileState**) malloc(sizeof(TileState*) * res->width);
    for (x=0; x < res->width; x++)
    {
        res->tiles[x] = (TileState*) malloc(sizeof(TileState) * res->height);
    }
    for (x=0; x < res->width; x++)
        for (y=0; y < res->height; y++)
        {
            memset(& res->tiles[x][y], 0, sizeof(TileState));
            switch(fixed[x][y])
            {
                case TILE_SPIDER: 
                    { 
                        res->tiles[x][y].logic.spider.current_direction = MOVE_SOUTH;
                        res->tiles[x][y].logic.spider.clockwise = 1;
                        break;
                    }
                default: break;
            }
            res->tiles[x][y].type = fixed[x][y];
            res->tiles[x][y].graphic_index = 0;
        }

    init_counts(res);

	/*INITIALISATIE VELDEN LEVELSTATE
	**********************************/

	res->collected_gem_count=0;
	res->collected_key_count=0;
	
	res->player_killed_by_spider=0;
	res->player_killed_by_snake=0;
	res->player_facing_right=0;
	res->score=0;
	res->finished=0;

	res->input=0;

	//Bepalen locatie player bij starten level
	//Opsporen teleport_tiles

	for(i=0; i<res->width; i++)
		for(j=0; j<res->height; j++){
			if(res->tiles[i][j].type == TILE_PLAYER){
				res->player_x=i;
				res->player_y=j;
			}
			res->tiles[i][j].WasTeleportTile=(res->tiles[i][j].type==TILE_TELEPORT)?1:0;
		}


	//Initialiseren Zobrist random tabel
	
	//De tabel heeft als dimensies [level_state->width, level_state->height, TILE_CONTENT_COUNT]
	
	res->Zobrist_Random_Tabel=(int***) calloc(res->width,sizeof(int**));

	for(i=0;i<res->width;i++){
		res->Zobrist_Random_Tabel[i]=(int**) calloc(res->height,sizeof(int*));
		for(j=0;j<res->height;j++) {
			res->Zobrist_Random_Tabel[i][j]=(int*) calloc(TILE_CONTENT_COUNT,sizeof(int));
			for(k=0;k < TILE_CONTENT_COUNT; k++)
				res->Zobrist_Random_Tabel[i][j][k]=(rand() ^ (rand() << 15))%HASHTABLE_SIZE;
		}
	}

    return res;
}

LevelState* read_level(const char* filename)
{
	int i,j,k;
	tile_content type;
	FILE* file;
	char* a;
	int* b;
	char t;
	int read_teleport=0;

	LevelState* result = (LevelState*) malloc(sizeof(LevelState));
	

	a = (char*) malloc(sizeof(char));
	b = (int*) malloc(sizeof(int));
	
	file = fopen(filename,"r");
	
	/*INLEZEN BREEDTE EN HOOGTE:
	*****************************/
	
	fscanf(file,"%d %d",&result->width);
	fscanf(file,"%c");	//x overslaan
	fscanf(file,"%d",&result->height);
	
	result->tiles = (TileState**) malloc(sizeof(TileState*) * result->width);
    for (i=0; i < result->width; i++){
        result->tiles[i] = (TileState*) malloc(sizeof(TileState) * result->height);
    }

	/*INLEZEN NAAM: 
	****************/
	result->name=(char*)malloc(sizeof(char));			
	t=fgetc(file);
	i=1;
	while(t!='\n'){
		result->name=(char*) realloc(result->name,i*sizeof(char));
		result->name[i-1]=t;
		i++;
		t=fgetc(file);
	}
	result->name=(char*) realloc(result->name,i*sizeof(char));
	result->name[i-1]=0;
	
	/*INLEZEN TEGELTYPES:
	***********************/
	for(j=result->height-1; j>=0; j--){		
		for(i=0; i<result->width; i++){
			fscanf(file,"%c",a);
			for(k=0; k<14; k++){
				if(*a==type_chars[k]){
					type = (tile_content) k;
					result->tiles[i][j].type = type;
					break;
				}
			}
		}
		fscanf(file,"%c");				//newline wordt genegeerd
	}	

	/*INLEZEN GRAFISCHE DATA: 
	**************************/

	for(j=result->height-1; j>=0; j--){			
		for(i=0; i<result->width; i++){
			
			fscanf(file,"%1d",b);
			result->tiles[i][j].graphic_index = *b;
		}
		
		fscanf(file,"%c");
						
	}


	/*INLEZEN LOGICAL DATA:
	************************/

	//er zijn evenveel logic data als er spinnen en teleports zijn
	for(j=result->height-1; j>=0; j--){	//we volgen de volgorde van de file en gaan rij per rij alle elementen af
		for(i=0; i<result->width; i++){
			if(result->tiles[i][j].type==TILE_TELEPORT){
				if(read_teleport!=0){	
					t=fgetc(file); //newline
				}
				fscanf(file,"%c",a); //(
				fscanf(file,"%d",b);
				result->tiles[i][j].logic.teleport.x = *b;
				fscanf(file,"%c"); //,
				fscanf(file,"%d",b);
				result->tiles[i][j].logic.teleport.y = *b;
				fscanf(file,"%c"); //)
				read_teleport=1;
			}

			if(result->tiles[i][j].type==TILE_SPIDER){
				t=fgetc(file); 
				if (fgetc(file) == 'O'){
					result->tiles[i][j].logic.spider.clockwise=0;
					for (k =0;k<14;k++)
						fgetc(file);
				}
				else {
					result->tiles[i][j].logic.spider.clockwise=1;
					for (k =0;k<7;k++)
						fgetc(file);
				}
				fgetc(file);	
				t=fgetc(file);
				
				switch (t){
					case 'U': result->tiles[i][j].logic.spider.current_direction=MOVE_NORTH;break;
					case 'D': result->tiles[i][j].logic.spider.current_direction=MOVE_SOUTH;break;
					case 'L': result->tiles[i][j].logic.spider.current_direction=MOVE_WEST;break;
					case 'R': result->tiles[i][j].logic.spider.current_direction=MOVE_EAST;break;
				}
			
				while(t!='\n') 
					t=fgetc(file);
			}
		}
	}
		
	fclose(file);
	init_counts(result);

	/*INITIALISATIE VELDEN LEVELSTATE
	**********************************/

	result->collected_gem_count=0;
	result->collected_key_count=0;
	
	result->player_killed_by_spider=0;
	result->player_killed_by_snake=0;
	result->player_facing_right=0;
	result->score=0;
	result->finished=0;

	result->input=0;

	//Bepalen locatie player bij starten level
	//Opsporen teleport_tiles

	for(i=0; i<result->width; i++)
		for(j=0; j<result->height; j++){
			if(result->tiles[i][j].type == TILE_PLAYER){
				result->player_x=i;
				result->player_y=j;
			}
			result->tiles[i][j].WasTeleportTile=(result->tiles[i][j].type==TILE_TELEPORT)?1:0;
		}


	//Initialiseren Zobrist random tabel
	
	//De tabel heeft als dimensies [level_state->width, level_state->height, TILE_CONTENT_COUNT]
	
	result->Zobrist_Random_Tabel=(int***) calloc(result->width,sizeof(int**));

	for(i=0;i<result->width;i++){
		result->Zobrist_Random_Tabel[i]=(int**) calloc(result->height,sizeof(int*));
		for(j=0;j<result->height;j++) {
			result->Zobrist_Random_Tabel[i][j]=(int*) calloc(TILE_CONTENT_COUNT,sizeof(int));
			for(k=0;k < TILE_CONTENT_COUNT; k++)
				result->Zobrist_Random_Tabel[i][j][k]=(rand() ^ (rand() << 15))%HASHTABLE_SIZE;
		}
	}

	/*VRIJGEVEN GEHEUGEN
	**********************/
	
	free (a);
	free (b);
    return result;
}


void init_counts(LevelState* level_state){
    int x,y;
    level_state->needed_key_count = 0;
    level_state->initial_gem_count = 0;
    for (x=0; x < level_state->width; x++)
        for (y=0; y < level_state->height; y++)
        {
            switch(level_state->tiles[x][y].type)
            {
                case TILE_GEM: { level_state->initial_gem_count++; break; }
                case TILE_KEY: { level_state->needed_key_count++; break; }
                default: break;
            }
        }
}

