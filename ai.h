#ifndef AI_H
#define AI_H

#include "game.h"
#include "level.h"
#include "gui.h"

/*
 * ai.h: the interface for the AI implementation.
 *
 * You may only edit the functions and datastructures here if it is required for your implementation,
 * and if so, keep the edits to a minimum.
 * It should be possible not to edit the functions or datastructures.
 *
 * You may also add your own functions to this interface.
 * */

/* Interval between AI reports to GUI. */
#define AI_REPORT_DELAY 500

/* maximum time AI will work (in ms) before giving up. */
#define AI_MAX_TIME 60000

/* Fixed size of hashtable. */
#define HASH_BITS 20
#define HASHTABLE_SIZE (1 << (HASH_BITS))

/* 
 * AI interface function: Use the AI to find a solution
 *
 *** This method is called when the AI button is pressed. ***
 *
 * returns true if solution was found, false otherwise.
 *
 * arguments:
 *   - moves: pointer to an array. If there is a solution, this 
 *            function allocates an array and puts the list of 
 *            moves needed to reach the solution in it.
 *   - moves_len: pointer to an int. If there is a solution, this
 *            function stores the number of moves needed to reach
 *            the solution in this int. Note that this is also the
 *            size of the "moves" array returned.
 *   - state: The LevelState from which the AI starts looking for 
 *            a solution.
 */
int find_solution(move** moves, int* moves_len, LevelState* state);

/* 
 * Returns the hash value for a LevelState. 
 *
 * Note: a requirement for any hash function implementation is: if equal_level then equal hashkey. 
 *
 * TODO implement this, using zobrist hashing, which is explained in the Project document part B.
 */
int hash(LevelState* level_state);



/*********************************************************************************
 * The next functions and definitions are for the datastructures used by the AI. * 
 *********************************************************************************/


/* forward declaration of AINode */
typedef struct AINode AINode;


/* struct to store AINodes in a tree. */
typedef struct AINodeTree AINodeTree;
struct AINodeTree
{
    AINode* ai_node;

    /* child nodes, indexed by (move - 1)  (because MOVE_NONE not used). */
    AINodeTree* children[4];

    /* parent node of possibly four children nodes */
    AINodeTree* parent;

    /* move made to go from parent to this child (only meaningful if parent != NULL) */
    move prev_player_move;
};

/* 
 * struct to store AINodes in a double linked list. 
 * 
 * see Project document part B.
 */
typedef struct AINodeList AINodeList; 
struct AINodeList 
{
    AINode* ai_node;
    AINodeList* next;
    AINodeList* prev;
};

/* 
 * An "AINode" is used in the AI algorithm. It stores a LevelState, and the list
 * and tree structures of certain list and trees it belongs to.
 */
struct AINode
{
    LevelState state;
    int depth;

    /* Note: no pointers. */
    AINodeTree tree_link;
    AINodeList todo_link;
    AINodeList all_link;
    AINodeList same_state_hashtable_link;
};

/* 
 * This function copies AINode and the LevelState. 
 *
 * The returned copy is NOT a member of any lists or tree, even if the original was. 
 * The copy is NOT automatically added to any list or tree by this function.
 *
 * All links must be initialised, point back to the copy, and have prev and next pointer
 * point to the link itself.
 * */
AINode* copy_ai_node(AINode* source_node);

/* 
 * Remove an AINode from a list, but only if it is a member.
 * This function must be safe to call if link is not a member of a list.
 */
void remove_ainode_from_list_if_member(AINodeList* link);

/* 
 * Remove an AINode from a tree, but only if it is a member.
 * This function must be safe to call if link is not a member of a tree.
 */
void remove_ainode_from_tree_if_member(AINodeTree* link);

/* 
 * Add an AINode to the end of a list. 
 * target_list: the list it has to be added to. 
 * added_link: the "AINodeList" link of the AINode to be added.
 */
void add_ainode_to_list(AINodeList* target_list, AINodeList* added_link);
/* 
 * Add an AINode to a tree.
 * parent_link: the parent in the tree.
 * m: the move to get from parent to child.
 * added_child: the "child" ainode that needs to be added.
 */
void add_ainode_to_tree(AINodeTree* parent_link, move m, AINode* added_child);


/* 
 * AINodeDB: a collection of lists, a hashtable and a tree. 
 *
 * all_size: at all time this contains the number of AINodes stored
 *
 * all_nodes: a list containing all stored AINodes
 * todo_nodes: a list containing all AINodes of which the "children" still need to be 
 *             checked and possibly added.
 * nodes_by_state_hashtable: a hashtable with all AINodes
 * tree_root: A pointer to a tree storing all AINodes. This is a pointer to the first 
 *            AINodes tree_link.
 *
 * hash_lookups: the number of hashtable lookups performed until now, i.e. the number of searches done.
 * hash_compares: the number of compares (=calls to equal_level) performed while doing hashtable lookups.
 *                i.e. the total number of compares needed for all searches. (a search can need multiple 
 *                compares or none at all)
 */
typedef struct {
    int all_size;

    AINodeList all_nodes;
    AINodeList todo_nodes;
    AINodeList nodes_by_state_hashtable[HASHTABLE_SIZE];
    AINodeTree* tree_root;

    /* statistical info */
    int hash_lookups;
    int hash_compares;
} AINodeDB;

/* 
 * Remove and free an ai_node from the AINodeDB: removes from all_nodes, todo_nodes (if member), hashtable and tree. 
 *
 * This function must work correctly if:
 *   - The node is in the db
 *   - The node is not in the db
 *   - db == NULL (in that case, assume the node is not in db) 
 * */
void free_ainode(AINode* ai_node, AINodeDB* db);


/* 
 * Marks node as expanded. 
 * This removes the node from the todo_nodes list in the db.
 * This must be safe: it should also works correctly if the node was already expanded.
 */
void mark_ainode_done(AINodeDB* db, AINode* ai_node);

/* 
 * Allocate and initialize an AINode. 
 *    - allocate an AINode
 *    - copy state LevelState into the AINode
 *    - initialise the list and tree "links" in AINode (empty lists)
 * returns NULL if not enough memory to allocate.
 * */
AINode* init_ai_node(LevelState* state);

/* 
 * Allocate and init a DB. 
 * Also add the root AINode to the DB.
 */
AINodeDB* init_db(AINode* root);

/* 
 * Free a DB and all its nodes.
 */
void free_db(AINodeDB* db);

/* 
 * Returns the size (in byte) of a db (including the size of all AINodes it stores)
 */
long int db_size_in_byte(AINodeDB* db);

/* 
 * copy_moves copies all moves needed to finish the game to the "moves" array.
 * It uses the tree_link from end_ai_node to go back in the tree to find these.
 *
 * end_ai_node is the AINode with a succesfull game exit.
 * moves_len is the number of moves needed to reach that state (given).
 * moves is initialised to store moves_len moves.
 */
void copy_moves(int moves_len, move* moves, AINode* end_ai_node);




/*************************************************************************
 * The next functions are an interface to the game logic implementation: *
 *   they are called from find_solution to use the "game logic" code.    * 
 *************************************************************************/


/* 
 * Returns whether a LevelState equals another LevelState
 * LevelStates are equal if all tiles are equal.
 */
int equal_level(LevelState* a, LevelState* b);

/* 
 * Access some information about game_over in LevelState.
 *
 * is_game_over returns true if the player died or exited the level
 * is_game_over_win returns true if the player exited the level
 * is_game_over_loss returns true if the player died
 *
 * */
int is_game_over(LevelState* level_state);
int is_game_over_win(LevelState* level_state);
int is_game_over_loss(LevelState* level_state);

/*
 * Run the game logic, moving player and updating everything (spiders, etc)
 *
 * return false if the player cannot make this move (blocked by wall, etc), true otherwise.
 * */
int calculate_state_after_playermove(LevelState* state, unsigned short player_move);
void show_solution(LevelState* level_state, move** moves_to_make, int number_of_moves);

#endif

