#ifndef GUI_H
#define GUI_H

/*
 *
 * gui.h: header file of the interface with the java GUI.
 *
 * IMPORTANT:
 * No modification to this file is allowed!
 *
 * */


#include "level.h"

/* Everything the GUI shows about one tile. */
typedef struct {
    tile_content type;
    unsigned short graphic_index;   /* index of graphics for this type (ex: different type of walls) (ignored for certain types) */
    unsigned short animation_index; /* index of animation loop for this graphic. */
} TileGraphics;

/* Everything the GUI shows on the screen. */
typedef struct {
    const char* name;
    int lives;
    int score;
    int needed_key_count;
    int collected_key_count;
    int initial_gem_count;
    int collected_gem_count;

    int height;
    int width;
    TileGraphics** graphics;
} LevelGraphics;

/* Update what is shown by the GUI. (Also disables AI view) */
void update_level_screen(LevelGraphics* graphics);

/* Retreive input from GUI (keys and buttons) */
int get_key_bitmask();
#define KEY_UP (1 << 0)
#define KEY_DOWN (1 << 1)
#define KEY_LEFT (1 << 2)
#define KEY_RIGHT (1 << 3)
#define KEY_AI (1 << 4)
#define KEY_RESTART (1 << 5)
#define KEY_EXIT (1 << 6)
#define KEY_SKIPLEVEL (1 << 7)

/* Reporting AI progress. (Also enables AI view) */
void update_ai_screen(int searchtime_ms, int memsize, int current_depth, int total_states, int states_unexpand, int total_hashtable_lookups, int total_hashtable_collisions, int hashtable_size);

#endif

