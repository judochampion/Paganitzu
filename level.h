#ifndef LEVEL_H
#define LEVEL_H

/*
 *
 * level.h: Header file specifying structures and methods dealing with initial level data.
 *          these structures are fixed ones created. Variable state is not stored in them.
 *
 * IMPORTANT:
 * The only modifiactions allowed to this file/interface are: extending TileState and/or LevelState
 * */

#define MAX_LEVEL_WIDTH 16
#define MAX_LEVEL_HEIGHT 16

/* Possible content of a tile */
typedef enum tile_content_ { TILE_EMPTY=0 , TILE_EXIT , TILE_WALL , TILE_SPIDER , TILE_SNAKE , TILE_BOULDER , TILE_WATER , TILE_TELEPORT , TILE_DEBRIS , TILE_GEM , TILE_KEY , TILE_PLAYER , TILE_FIREBALL , TILE_WALL_BREAKABLE } tile_content;
#define TILE_CONTENT_COUNT (TILE_WALL_BREAKABLE+1)

/* 
 * char array, which links tile_content with chars stored in level files. The tile_content index is the index in the array.
 * For example: 
 *     TILE_BOULDER -> 5
 *     type_chars[5] == 'O'
 *
 * So 'O' is a boulder in the level files.
 * */
extern const char* type_chars;

/* Possible moves/directions. Used for spiders and players. */
typedef enum move_ { MOVE_NOT=0, MOVE_EAST, MOVE_NORTH, MOVE_WEST, MOVE_SOUTH } move;
#define MOVE_NONE MOVE_NOT

/* Translate a move enum to a (statically allocated) string. */
const char* move_to_s(move m);
/* Translate a string to a move enum. */
int parse_move(const char* m);


/* All Info info about a single tile */
typedef struct {
    tile_content type;
    unsigned short graphic_index;   /* index of graphics for this type (ex: different type of walls) */
    union {
        struct {
            /*teleport destination*/
            unsigned x : 4;
            unsigned y : 4;
			} teleport;
        struct {
            unsigned clockwise : 1;
            unsigned current_direction : 3;
			unsigned already_moved:1;
        } spider;
    } logic;
	unsigned WasTeleportTile:1;
} TileState;

/* All info about a level state, including all tile states. 
 *
 * You will need to extend this, if you need to store more info about a level.
 * */
typedef struct {
    char* name;
    int height;
    int width;
    int needed_key_count;
	int collected_key_count;
    int initial_gem_count;
	int collected_gem_count; 
	int player_x;
	int player_y;
	int lives;
	int player_killed_by_spider;
	int player_killed_by_snake; 
	int player_moved_last_time; 
	int score;
	unsigned finished:1;			//level moet worden afgesloten
	unsigned player_facing_right:1;
	int input;
	unsigned impossible_move:1;		//AI
    TileState** tiles;	
	int*** Zobrist_Random_Tabel;	//AI
} LevelState;

/* Fills in LevelState's needed_key_count and initial_gem_count based on tiles */
void init_counts(LevelState* level_state);

/* Read a level from file. */
LevelState* read_level(const char* filename);

/* Load a static example level. */
LevelState* load_example_level();

#endif

