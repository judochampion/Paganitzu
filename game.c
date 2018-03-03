#include "game.h"

#include "system.h"

#include "ai.h"

#include "gui.h"
#include "spider.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void game_loop(LevelState *levels[], int level_count)
{
	int count=0;	//deze variabele houdt bij welk level momenteel wordt uitgevoerd
	int score = 0; //dit is de globale score, behaald over alle levels
	int i,j,k;
	
	move** moves = (move**) malloc (sizeof(move*));
	int* movelen = (int*) calloc (sizeof(int),1);
	
	LevelState* current = (LevelState*) malloc (sizeof(LevelState));

	printf("Wij wensen u veel succes!\nAantal levels: %i \n\n",level_count);
	
	current->score=0;
	current->lives=3;

	assert(level_count > 0);
	
	/*We starten het eerste level*/
	copy_level(current, levels[count]);
	run_level_loop(current);

	/*In verdere iteraties starten we het volgende level als er nog 1 is en het vorige correct is uitgespeeld
	**OF KEY_SKIPLEVEL werd ingedrukt
	**In geval van skip level wordt de eventuele score behaald in dit level niet meegeteld */

	while(current->input!=KEY_EXIT){
		
		/*NAAR HET VOLGENDE LEVEL*/
		if((current->finished==1 || current->input==KEY_SKIPLEVEL)&&count<level_count-1){
			if (current->input!=KEY_SKIPLEVEL)
				score = current->score;
			count++;
			free_tiles(current);
			copy_level(current, levels[count]);
			//initialisatie van enkele parameters: 
			current->lives=3;
			current->score = score;	//score meenemen naar volgend level
			
			run_level_loop(current);
		}

		else if(current->lives==0){
			printf("\nGAME OVER\n");
			break;
		}

		/*HUIDIG LEVEL HERSTARTEN*/
		else if(current->player_killed_by_snake==1 || current->player_killed_by_spider == 1 || current->input==KEY_RESTART){
			/*RESTART: score teruggezet naar score voor huidig level
			**KILLED: score terug op 0*/
			if(current->input==KEY_RESTART)
				current->score=score;
			free_tiles(current);
			copy_level(current, levels[count]);
			run_level_loop(current);
		}
		
		/*AI STARTEN*/
		else if(current->input==KEY_AI){			
			if(find_solution(moves,movelen,current)){
				printf("OPLOSSING GEVONDEN!\naantal moves: %i\n",*movelen);
				show_solution(current,moves,*movelen);
				current->input=0;
			}
			else{
				printf("GEEN OPLOSSING GEVONDEN...\n");
				current->input=0;
				run_level_loop(current);
			}
		}

		else{
			printf("Geen hogere levels meer\n");
			break;
		}
	}

	score = current->score;
		
	//Vrijgeven gealloceerd geheugen

	
	free_tiles(current);
	free(current);

	for(i=0; i<level_count; i++){
		for(j=0; j<levels[i]->width;j++){
			free(levels[i]->tiles[j]);
			for(k=0; k< levels[i]->height; k++)
				free(levels[i]->Zobrist_Random_Tabel[j][k]);
			free(levels[i]->Zobrist_Random_Tabel[j]);
		}
		free(levels[i]->tiles);
		free(levels[i]->Zobrist_Random_Tabel);
		if(strcmp(levels[i]->name,"Static test level")!=0)
			free(levels[i]->name);
		free(levels[i]);
	}

	free(movelen);
	free(moves);

	printf("\nProficiat! Uw score bedraagt maar liefst %d punten!\n",score);
}

void copy_level(LevelState* current, LevelState* level){
	int x,y;

	//BREEDTE EN HOOGTE
	current->width=level->width;
	current->height=level->height;

	//GEHEUGEN ALLOCEREN EN TILES KOPIEREN
	current->tiles = (TileState**) malloc(sizeof(TileState*) * current->width);	
	for (x=0; x < current->width; x++){
        current->tiles[x] = (TileState*) malloc(sizeof(TileState) * current->height);
		for(y=0; y<current->height;y++)
			current->tiles[x][y]=level->tiles[x][y];
    }

	//ZOBRIST RANDOM TABEL	
	current->Zobrist_Random_Tabel=level->Zobrist_Random_Tabel;

	//INDICATORS EN TELLERS
	current->collected_gem_count=level->collected_gem_count;
	current->collected_key_count=level->collected_key_count;
	
	current->player_killed_by_spider=level->player_killed_by_spider;
	current->player_killed_by_snake=level->player_killed_by_snake;
	current->player_facing_right=level->player_facing_right;

	current->finished=level->finished;
	current->score=level->score;
	current->input=level->input;
		
	current->name=level->name;
	current->initial_gem_count=level->initial_gem_count;
	current->needed_key_count=level->needed_key_count;

	//LOCATIE PLAYER
	current->player_x=level->player_x;
	current->player_y=level->player_y;

}

void free_tiles(LevelState* level){
	int i;

	for(i=0; i<level->width;i++)
		free(level->tiles[i]);
	free(level->tiles);
}

void run_level_loop(LevelState* level_state)
{
	int x;

    LevelGraphics graphic_output;
    
	//geheugen vrijmaken voor graphics
    graphic_output.graphics = (TileGraphics**) malloc(sizeof(TileGraphics) * level_state->width); //hier en hieronder (...**) toegevoegd
    for (x = 0; x < level_state->width; x++)
        graphic_output.graphics[x] = (TileGraphics*) malloc(sizeof(TileGraphics) * level_state->height);

    while(level_state->input< KEY_AI && level_state->lives!=0 && level_state->finished==0 && level_state->player_killed_by_snake==0 && level_state->player_killed_by_spider ==0) { //hier stond oorspronkelijk 1
        update_level_graphics(&graphic_output, level_state);

        /* Update GUI level using JNI */
        update_level_screen(&graphic_output);

        /* Wait until next logic and graphic frame */
        sleep_millis(FRAME_DURATION_MS);

        /* Get input */
		level_state->input=get_key_bitmask();

        /* Move player if input. (and also do interaction with level) */
		
		//Controle of speler gedood wordt door slang (hier omdat anders speler verplaatst wordt na animatie slang)
		if(level_state->player_moved_last_time)
			check_for_snake(level_state,&graphic_output);
					
		//Actie ondernemen op basis van input
		level_state->player_moved_last_time=0;
		interpret_input(level_state);

			
        /* Update level (move spiders, see if snakes kill player, etc.) */
		//Slang: zie hierboven
		if(check_if_spider_is_next_player(level_state)){			
			level_state->player_killed_by_spider=1;
			level_state->lives--;
		}
		automatic_move_spider(level_state);
		if(check_if_spider_is_next_player(level_state)){			
			level_state->player_killed_by_spider=1;
			level_state->lives--;
		}

		
    }

	/*Vrijgeven gealloceerd geheugen*/
	for(x=0; x<level_state->width; x++)
		free(graphic_output.graphics[x]);
	free(graphic_output.graphics);
}

void update_level_graphics(LevelGraphics* graphic_output, LevelState* level_state)
{
    int x,y;
    /* Dynamische info wordt in overeenstemming gebracht met de actuele level_state */
    int player_pos_x= level_state->player_x; 
    int player_pos_y= level_state->player_y; 
    int player_facing_right = level_state->player_facing_right;
    int player_moved_last_time = level_state->player_moved_last_time;
	int player_killed_by_spider = level_state->player_killed_by_spider;
    int player_killed_by_snake = level_state->player_killed_by_snake;
	

    static int global_animation_index = 0;
    global_animation_index++;

    /* update graphic_output */
    graphic_output->lives = level_state->lives;
    graphic_output->score = level_state->score;
    graphic_output->height = level_state->height;
    graphic_output->width = level_state->width;
    graphic_output->name = level_state->name;
    graphic_output->needed_key_count = level_state->needed_key_count;
    graphic_output->collected_key_count = level_state->collected_key_count; 
    graphic_output->initial_gem_count = level_state->initial_gem_count;
    graphic_output->collected_gem_count = level_state->collected_gem_count; 

    for (x = 0; x < level_state->width; x++)
        for (y = 0; y < level_state->height; y++)
        {
           TileState* init = & level_state->tiles[x][y];
           TileGraphics* g_out = & graphic_output->graphics[x][y];
           
           g_out->type = init->type;
           switch (init->type)
           {
               case TILE_EXIT: 
                   {
                       g_out->graphic_index = 0;
                       if (graphic_output->collected_key_count < level_state->needed_key_count)
                           g_out->animation_index = 0;
                       else
                           g_out->animation_index = 1;
                       break;
                   }
               case TILE_SNAKE: 
                   {
                       int face_right = player_pos_x > x;
                       int open_mouth = global_animation_index % 2;
                       g_out->graphic_index = 0;
                       g_out->animation_index = face_right ? 1 : 0;
                       if (open_mouth)
                           g_out->animation_index += 2;
                       break;
                   }
               case TILE_SPIDER: 
                   {
                       g_out->graphic_index = 0;
                       switch(init->logic.spider.current_direction) {
                           case MOVE_EAST: g_out->animation_index = 3; break;
                           case MOVE_NORTH: g_out->animation_index = 2; break;
                           case MOVE_WEST: g_out->animation_index = 1; break;
                           case MOVE_SOUTH: g_out->animation_index = 0; break;
                           default: g_out->animation_index = 0; break;
                       }
                       break;
                   }
               case TILE_PLAYER:
                   {
                       g_out->graphic_index = 0;
                       g_out->animation_index = player_facing_right ? 1 : 0;
                       if (player_moved_last_time && global_animation_index % 2)
                           g_out->animation_index += 2;

                       if (player_killed_by_spider)
                       {
                           g_out->graphic_index = 2;
                           g_out->animation_index = 0;
                       }
                       if (player_killed_by_snake)
                       {
                           g_out->graphic_index = 1;
                           g_out->animation_index = global_animation_index % 2;
                       }
					   break;
                   }
               default : 
                   {
                       if (g_out->type == init->type)
                       {
                           g_out->graphic_index = init->graphic_index;
                           g_out->animation_index = 0;
                       }
                       else
                       {
                           g_out->graphic_index = 0;
                           g_out->animation_index = 0;
                       }
                       break;
                   }
           }
        }
}

void interpret_input(LevelState* level_state){
	/*Gewenste coordinaten input genereren als en slechts als ze zich nog binnen de dimensie van het level bevinden*/
	int input = level_state->input;
		
	if((input == KEY_UP) && (level_state->player_y < level_state->height-1))			//MOVE NORTH		
		take_action(level_state,level_state->player_x,level_state->player_y+1);
	else if((input == KEY_DOWN )&& (level_state->player_y>0))							//MOVE SOUTH
		take_action(level_state,level_state->player_x,level_state->player_y-1);
	else if((input == KEY_LEFT) && (level_state->player_x>0)){							//MOVE WEST
		level_state->player_facing_right=0;
		take_action(level_state,level_state->player_x-1,level_state->player_y);
	}
	else if((input == KEY_RIGHT) && (level_state->player_x<level_state->width-1)){		//MOVE EAST
		level_state->player_facing_right=1;
		take_action(level_state,level_state->player_x+1,level_state->player_y);
	}

	// in het geval van EXIT SKIPLEVEL RESTART OF AI moet er niets gebeuren en wordt bij de volgende iteratie uit de while-lus van de level_loop gegaan
}

void take_action(LevelState* level_state, int obst_x, int obst_y){
	level_state->impossible_move=0;
	switch(level_state->tiles[obst_x][obst_y].type)
	{	

		case TILE_EMPTY:
			{
				move_player(level_state, obst_x, obst_y);
				break;
			}

		case TILE_DEBRIS:
			{
				move_player(level_state, obst_x, obst_y);
				break;
			}

		case TILE_SPIDER:
			{
				level_state->lives--;
				level_state->player_killed_by_spider=1;
				break;
			}

		case TILE_GEM:
			{
				move_player(level_state, obst_x, obst_y);
				level_state->collected_gem_count++;
				level_state->score+=10;						//keuze: 10 punten per gem
				break;
			}

		case TILE_KEY:
			{
				move_player(level_state, obst_x, obst_y);
				level_state->collected_key_count++;
				break;
			}

		case TILE_BOULDER:
			{
				move_boulder(level_state, obst_x, obst_y);
				break;
			}

		case TILE_TELEPORT:
			{
				/*Speler eerst verzetten naar te betreden teleporttegel*/
				level_state->tiles[level_state->player_x][level_state->player_y].type=TILE_EMPTY;
				level_state->player_x=obst_x;
				level_state->player_y=obst_y;

				/*Als doelcoordinaat geven we de teleportdata van deze tegel mee*/
				move_player(level_state, level_state->tiles[obst_x][obst_y].logic.teleport.x, level_state->tiles[obst_x][obst_y].logic.teleport.y);
				
				break;
			}

		case TILE_EXIT:
			{
				if(level_state->needed_key_count==level_state->collected_key_count)
					level_state->finished=1;
				else {
					level_state->impossible_move = 1;
				}
				break;
			}

		default: 
			{
				level_state->impossible_move = 1;
				break;
			}
			//in alle andere gevallen wordt de speler niet verplaatst (muur, water,slang...) en gebeurt er verder ook niets 
		
				
	}
}

void move_player(LevelState* level_state, int obst_x, int obst_y){
	
	//herstellen verlaten tegel
	if(level_state->tiles[level_state->player_x][level_state->player_y].WasTeleportTile==1){
		level_state->tiles[level_state->player_x][level_state->player_y].type=TILE_TELEPORT;
	}
	else
		level_state->tiles[level_state->player_x][level_state->player_y].type=TILE_EMPTY;
	
	//speler verzetten
	level_state->tiles[obst_x][obst_y].type=TILE_PLAYER;
	level_state->tiles[obst_x][obst_y].graphic_index=level_state->tiles[level_state->player_x][level_state->player_y].graphic_index;
	level_state->player_x=obst_x;
	level_state->player_y=obst_y;
	level_state->player_moved_last_time=1;
}

void move_boulder(LevelState* level_state, int obst_x, int obst_y){
	//Bepalen coordinaten locatie waar steen !eventueel! naartoe wordt geduwd
	int boulder_x = obst_x, boulder_y=obst_y;

	if(obst_x>level_state->player_x && obst_x < level_state->width-1){	//steen rechts
		boulder_x = obst_x+1;
	}
	else if(obst_x < level_state->player_x && obst_x>0){					//steen links
		boulder_x = obst_x-1;
	}
	else if(obst_y > level_state->player_y && obst_y<level_state->height-1){ //steen boven
		boulder_y = obst_y+1;
	}
	else if(obst_y < level_state->player_y && obst_y>0){						//steen onder
		boulder_y = obst_y-1;
	}

	if(level_state->tiles[boulder_x][boulder_y].type == TILE_WATER){		//Water op toekomstige locatie?
		level_state->tiles[boulder_x][boulder_y].type = TILE_EMPTY;
		move_player(level_state, obst_x, obst_y);
	}

	else if(level_state->tiles[boulder_x][boulder_y].type == TILE_EMPTY){		//Toekomstige locatie leeg?
		level_state->tiles[boulder_x][boulder_y].type = TILE_BOULDER;
		move_player(level_state, obst_x, obst_y);
	}

	//in het andere geval worden noch de steen noch de player verplaatst
}



void check_for_snake(LevelState* level_state, LevelGraphics* graphic_output){
	/*Deze functie controleert of de speler gedood wordt door een slang en voert de bijhorende animatie uit*/

	int snake_position = killed_by_snake(level_state);
	if(snake_position!=-1)	
		snake_animation(level_state,graphic_output, snake_position);
}

int killed_by_snake(LevelState* level_state){
	/*Deze functie zoekt zowel links als rechts van de speler naar slangen en doodt de speler indien nodig*/
	/*Deze functie geeft de kolomcoordinaat van de slang terug*/

	int i;
	
	//Links zoeken
	for(i=level_state->player_x-1; i>=0;i--){

		//obstakel gevonden, verder zoeken is overbodig
		if(level_state->tiles[i][level_state->player_y].type!=TILE_EMPTY && level_state->tiles[i][level_state->player_y].type!=TILE_SNAKE)
			break;

		//slang op zelfde hoogte, we zijn er zeker van dat er enkel lege ruimte tussen de speler en de slang zit
		if(level_state->tiles[i][level_state->player_y].type == TILE_SNAKE){
			level_state->player_killed_by_snake=1;
			level_state->lives--;
			return i;
		}
	}

	//Rechts zoeken, enkel indien links geen dodelijke slang gevonden
	if(level_state->player_killed_by_snake==0){
		for(i=level_state->player_x+1; i<level_state->width; i++){
			if(level_state->tiles[i][level_state->player_y].type!=TILE_EMPTY && level_state->tiles[i][level_state->player_y].type!=TILE_SNAKE)
				break;
			
			if(level_state->tiles[i][level_state->player_y].type == TILE_SNAKE){	
				level_state->player_killed_by_snake=1;
				level_state->lives--;
				return i;
			}
		}
	}

	return -1;
}
void snake_animation(LevelState* level_state, LevelGraphics* graphic_output, int snake_position){
	/*Deze functie voert de animatie uit die hoort bij het gedood worden door een slang*/

	int i;
	
	if(snake_position < level_state->player_x)
		for(i=snake_position+1; i<level_state->player_x; i++){
			level_state->tiles[i][level_state->player_y].type=TILE_FIREBALL;
			update_level_graphics(graphic_output,level_state);
			update_level_screen(graphic_output);
			sleep_millis(FRAME_DURATION_MS);
			level_state->tiles[i][level_state->player_y].type=TILE_EMPTY;
		
		}

	else
		for(i=snake_position-1; i>level_state->player_x; i--){
			level_state->tiles[i][level_state->player_y].type=TILE_FIREBALL;
			update_level_graphics(graphic_output,level_state);
			update_level_screen(graphic_output);
			sleep_millis(FRAME_DURATION_MS);
			level_state->tiles[i][level_state->player_y].type=TILE_EMPTY;
		}
}
