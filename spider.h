#ifndef SPIDER_H
#define SPIDER_H

#include "game.h"
#include "level.h"
#include "gui.h"

//SPINmethodes:

int check_if_spider_is_next_player(LevelState* level_state);
void automatic_move_spider(LevelState* level_state);
void spider_dies(LevelState* level_state, int spider_x, int spider_y);
int give_direction(int old_x,int old_y,int new_x,int new_y);
void get_spider_fields(LevelState* level_state,  int spider_x, int spider_y, int* dir);
int is_spider_out_of_bounds(LevelState* level_state, int x,int y);
void move_spider_backward(LevelState* level_state,int old_x,int old_y);
int is_spin_blocked(LevelState* level_state, int x, int y);
int spin_only_backward(LevelState* level_state,int* dir);
void actually_move_spider(LevelState* level_state,int old_x,int old_y,int new_x,int new_y);

#endif

