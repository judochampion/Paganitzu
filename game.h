#ifndef GAME_H
#define GAME_H

/*
 *
 * game.h: Header file specifying structures and methods dealing with gameplay.
 *
 * This file must be modified and extended. 
 *
 * */

#include "gui.h"
#include "level.h"

#define FRAME_DURATION_MS 250

void game_loop(LevelState *levels[], int level_count);
void run_level_loop(LevelState* level_state);
void update_level_graphics(LevelGraphics* graphic_output, LevelState* level_state);

/*KOPIEREN van het ingelezen level*/
void copy_level(LevelState* current, LevelState* level);

/*Geheugen ingenomen door tiles VRIJGEVEN*/
void free_tiles(LevelState* level);

/*Gewenste verplaatsing bepalen op basis van de input*/
void interpret_input(LevelState* level_state);

/*Bepalen ACTIE op basis van input en het eventuele obstakel waar de speler mee wil interageren*/
void take_action(LevelState* level_state, int obst_x, int obst_y);

/*Het eigenlijke VERPLAATSEN van de speler*/
void move_player(LevelState* level_state,int obst_x, int obst_y);

/*Voortduwen van een steen + interactie met water*/
void move_boulder(LevelState* level_state, int obst_x, int obst_y);

//SLANGmethodes:

/*Controleren of speler wordt gedood*/
void check_for_snake(LevelState* level_state, LevelGraphics* graphic_output);
int killed_by_snake(LevelState* level_state);		//AI

/*Animatie vuurbal*/
void snake_animation(LevelState* level_state, LevelGraphics* graphic_output, int snake_position);


#endif
