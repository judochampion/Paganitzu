#include "ai.h"
#include "level.h"

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <stdlib.h>
#include "spider.h"

/*
 * ai.c: implementation of the AI
 *
 * The implementation of find_solution is given below. You need to add the implementation of the 
 * functions for the datastructures, and the functions interfacing with the game logic code.
 *
 * You should not need to change anything in "find_solution" or "add_node_unless_match_exists".
 * If you really need to change something, keep the change to a minimum, add comments to the code,
 * and explain in your report.
 *
 * You are allowed to add helper functions to this file.
 * */

/* 
 * Helper function for find_solution: 
 *    Add to db, unless worse than existing. 
 *    Assumes new_node is todo. */                                                                               
int add_node_unless_match_exists(AINodeDB* db, AINode* new_node, AINode* parent_node, move m)
{
    LevelState* level_state = & new_node->state;
	
    /* Check hashtable collisions */
    
	AINodeList* same_state_list = &db->nodes_by_state_hashtable[hash(level_state)];
	
    AINodeList* cur_same_state = (same_state_list->next);

	//printf("\n%d",hash(level_state));

    db->hash_lookups++;

	//printf("%d\n",hash(level_state));

    while (cur_same_state != same_state_list)
    {
        AINode* same_state_ainode = cur_same_state->ai_node;
        cur_same_state = cur_same_state->next;
		
		//printf("+\n");

        db->hash_compares++;

        /* Check if collision is due to equal level, or not. */
        if (equal_level(&same_state_ainode->state, level_state))
        {
			//printf("equal\n");
            return 0; /* existing node: do not add */
        }
    }
	//printf("lookup\n");

    /* add this node */
    add_ainode_to_tree(&parent_node->tree_link, m, new_node);
    add_ainode_to_list(&db->all_nodes, &new_node->all_link);
    add_ainode_to_list(&db->todo_nodes, &new_node->todo_link);
    add_ainode_to_list(same_state_list, &new_node->same_state_hashtable_link);
    db->all_size += 1;
	
    return 1;
}

/* see Project document part B */
int find_solution(move** moves, int* moves_len, LevelState* level_state){
	move all_moves[4] = { MOVE_EAST, MOVE_NORTH, MOVE_WEST, MOVE_SOUTH };
    AINode* start_node;
    AINodeDB* db;
    int depth = 1;
    int todo_size = 1;
    int stop = 0;
    
    system_time_t search_start = current_time_millis();
    system_time_t next_report = search_start;
    system_time_t ai_deadline = search_start + AI_MAX_TIME;
    start_node = init_ai_node(level_state);

	
	if (start_node == NULL)
        return 0;
    start_node->depth = 0;
	db = init_db(start_node);
	if (db == NULL)
    {

        free_ainode(start_node, NULL);
        return 0;
    }
    while(!stop)
    {

        int added_count = 0;
        int not_added_count = 0;
        int not_possible_count = 0;

        AINodeList* todo_nodes = db->todo_nodes.next;


        while (!stop && todo_nodes != &db->todo_nodes)
        {
			
            AINode* base_ainode = todo_nodes->ai_node;
            system_time_t now = current_time_millis();
            int all_moves_index;
			
            assert(base_ainode != NULL);

            if (next_report <= now)
            {
                update_ai_screen(now - search_start, db_size_in_byte(db), depth, db->all_size, todo_size, db->hash_lookups, db->hash_compares, HASHTABLE_SIZE);
                next_report = next_report + AI_REPORT_DELAY;

                if (now >= ai_deadline)
                {
                    printf("Maximum search time reached\n");
                    stop = 1;
                }
            }


            if (!is_game_over(&base_ainode->state))
				
                for (all_moves_index=0; all_moves_index<4; all_moves_index++)
                {
                    move player_move = all_moves[all_moves_index];
                    int must_add_to_db = 0;
                    int legal_player_move;

                    AINode* next_node = copy_ai_node(base_ainode);
                    if (next_node == NULL)
                    {
                        printf("Stop: out of memory\n");

                        update_ai_screen(current_time_millis() - search_start, db_size_in_byte(db), depth, db->all_size, todo_size, db->hash_lookups, db->hash_compares, HASHTABLE_SIZE);

                        free_db(db);
                        return 0;
                    }
                    next_node->depth = base_ainode->depth + 1;

                    legal_player_move = calculate_state_after_playermove(&next_node->state, player_move);
					
                    if (legal_player_move)
                    {
                        if (is_game_over(&next_node->state))
                        {
                            if (is_game_over_win(&next_node->state))
                            {
                                /* Game over, win. */
                                printf("Stop AI: solution found at depth %d.\n", depth);
                                add_ainode_to_tree(&base_ainode->tree_link, player_move, next_node);

                                /* Copy moves and return solution. */
                                *moves_len = next_node->depth;
                                *moves = (move*) malloc(*moves_len * sizeof(move));
                                copy_moves(next_node->depth, *moves, next_node);

                                update_ai_screen(current_time_millis() - search_start, db_size_in_byte(db), depth, db->all_size, todo_size, db->hash_lookups, db->hash_compares, HASHTABLE_SIZE);

                                free_ainode(next_node, NULL);
                                free_db(db);
                                return 1;
                            }
                            else{
                                must_add_to_db = 0; /* Game over, killed.*/
							}
							
                        } else
                            must_add_to_db = 1; /* Game not over */
                    } else{
                        must_add_to_db = 0; /* illegal move */
					}
					

                    if (must_add_to_db)
                    {
                        /* added in front of todo, so causes no problem with cur_item */
                        int has_been_added = add_node_unless_match_exists(db, next_node, base_ainode, player_move);
                        if (!has_been_added)
                        {
                            free_ainode(next_node, NULL);
                            not_added_count++;
                        }
                        else
                        {
                            added_count++;
                            todo_size++;
                        }
                    } else
                    {
                        /* Game over, killed or illegal move. */
                        free_ainode(next_node, NULL);
                        not_possible_count++;
                    }
                }

            todo_nodes = todo_nodes->next;

            mark_ainode_done(db, base_ainode);
            todo_size--;
        }
		
		
        depth++;

        if (db->todo_nodes.next == &db->todo_nodes)
        {
            /* No more nodes in todo list */
            printf("Stop AI: No possible actions.\n");
            stop = 1;
        }
    }
	
    update_ai_screen(current_time_millis() - search_start, db_size_in_byte(db), depth, db->all_size, todo_size, db->hash_lookups, db->hash_compares, HASHTABLE_SIZE);

    free_db(db);

    return 0;
}




/*********************************************************************************
 *  Code to be added. Empty functions are provided, these need to be implemented!*
 *********************************************************************************/

long int db_size_in_byte(AINodeDB* db)

{   /*In deze code wordt de grootte van al het dynamisch gealloceerd geheugen berekend*/
	/*De breedte en hoogte van het level kunnen enkel geïnitialiseerd worden als er in de db al tenminste één knoop in de todo_nodes-lijst zit*/
	int i = sizeof(db);
	int breedte= (db->todo_nodes.ai_node!=NULL)?db->todo_nodes.ai_node->state.width:0;
	int hoogte = (db->todo_nodes.ai_node!=NULL)?db->todo_nodes.ai_node->state.height:0;
	i+= db->all_size*(sizeof(AINode)+(breedte*sizeof(TileState)+hoogte*sizeof(TileState*)));  // Al het geheugen gealloceerd door een AINODE
	i+=(db->todo_nodes.ai_node!=NULL)?(sizeof(db->todo_nodes.ai_node->state.Zobrist_Random_Tabel)):0;
    return i;
   
   
}

int hash(LevelState* level_state)
{   /*In de hash-functie wordt handig gebruik gemaakt van de Zobrist_Random_Tabel, die in de read-levelfunctie
	als onderdeel van de LevelState werd gealloceerd*/
	int x,y,result=0;
	for(x=0;x<level_state->width;x++){
		for(y=0;y<level_state->height;y++){
			result=result^(level_state->Zobrist_Random_Tabel[x][y][level_state->tiles[x][y].type]);
		}
	}
	return result%(HASHTABLE_SIZE);
}


int equal_level(LevelState* a, LevelState* b)
{
	/* We gaan er meteen van uit dat de hoogte en de breedte van de twee LevelStates a en b overeenkomen, aangezien het LevelStates zijn
	die allemaal voortkomen uit de ene originele LevelState waarbij de AI werd opgeroepen*/
	int x,y;

	/* We controleren enkel de tegeltypes. Het is belangrijk dat deze functie 'licht' is, daarom gaan we 
	niet de richting van de spin ook controleren. Als we hier bijvoorbeeld ook de spin-richting controleren, gaat onze AI aanzienlijk 
	trager werken. Bovendien zal deze richting steeds dezelfde zijn op een welbepaalde tegel, hetzelfde geldt voor de andere velden*/
	for(x=0; x<a->width; x++)
		for(y=0; y<a->height; y++)
			if(a->tiles[x][y].type != b->tiles[x][y].type)
				return 0;
		
    return 1;
}


AINode* init_ai_node(LevelState* state)
{	//Alle velden van de knoop worden hier meteen geïnitaliseerd. 
    int i;

	/*GEHEUGEN ALLOCEREN
	********************/
	AINode* node = (AINode*) malloc (sizeof(AINode));
	node->state=*state;
	copy_level(&node->state,state);

	/*ALL LINK
	**********/
	node->all_link.ai_node=node;
	node->all_link.next=&node->all_link;
	node->all_link.prev=&node->all_link;

	/*TODO LINK
	************/
	node->todo_link.ai_node=node;
	node->todo_link.next=&node->todo_link;
	node->todo_link.prev=&node->todo_link;

	/*TREE LINK
	************/
	node->tree_link.ai_node=node;
	node->tree_link.parent=&node->tree_link;
	for(i=0; i<4; i++)
		node->tree_link.children[i]=&node->tree_link;

	/*SAME STATE HASHTABLE LINK
	*****************************/
	node->same_state_hashtable_link.ai_node=node;
	node->same_state_hashtable_link.next=&node->same_state_hashtable_link;
	node->same_state_hashtable_link.prev=&node->same_state_hashtable_link;

	return node;
}

AINode* copy_ai_node(AINode* source_node)
{
  	int i ; 
	/*GEHEUGEN ALLOCEREN
	*********************/
	AINode* copy = (AINode*) malloc(sizeof(AINode));

	/*LEVEL STATE KOPIEREN
	***********************/
	copy_level(&copy->state,&source_node->state);

	copy->depth=source_node->depth;

	/*TREE LINK
	************/
	copy->tree_link.ai_node=copy;
	copy->tree_link.parent=&copy->tree_link;
	for(i=0; i<4; i++)
		copy->tree_link.children[i]=&copy->tree_link;

	/*TODO LINK
	***********/
	copy->todo_link.ai_node=copy;
	copy->todo_link.next=(&copy->todo_link);
	copy->todo_link.prev=(&copy->todo_link);

	/*ALL LINK
	***********/
	copy->all_link.ai_node=copy;
	copy->all_link.next=(&copy->all_link);
	copy->all_link.prev=(&copy->all_link);

	/*SAME STATE HASHTABLE LINK
	****************************/
	copy->same_state_hashtable_link.ai_node=copy;	
	copy->same_state_hashtable_link.next=(&copy->same_state_hashtable_link);
	copy->same_state_hashtable_link.prev=(&copy->same_state_hashtable_link);

	return copy;

}

void free_ainode(AINode* ai_node, AINodeDB* db)
{
	/* Het checken of de ai_node in de db zit gebeurt bij het oproepen van de gepaste remove-functies, indien dit niet het geval is 
	 zal er simpelweg niets gebeuren*/
	int i;

	if(db!=NULL){
		db->all_size--;
		remove_ainode_from_list_if_member((&ai_node->all_link));
		remove_ainode_from_list_if_member((&ai_node->todo_link));
		remove_ainode_from_tree_if_member((&ai_node->tree_link));
		remove_ainode_from_list_if_member((&ai_node->same_state_hashtable_link));
	}

	//VRIJGEVEN TEGELS:
	for( i = 0 ; i < ai_node->state.width ; i++){
		free(ai_node->state.tiles[i]);
	}
	free(ai_node->state.tiles);
	
	//VRIJGEVEN KNOOP ZELF
	free(ai_node);

}

void remove_ainode_from_list_if_member(AINodeList* link)
{	//Als de knoop in de lijst zit, wordt de knoop gevonden en uit de lijst gehaald:
	if(link->next!=NULL && link->prev!=NULL && link->ai_node!=NULL){
		link->prev->next=link->next; 
		link->next->prev=link->prev;
	}

}

void remove_ainode_from_tree_if_member(AINodeTree* link)
{	int child;
	
	//De correcte kindpointer van de parent op nul plaatsen
	//Alle pointers van de verwijderde link zelf op nul plaatsen 
	//De ouderpointer van de bestaande kinderen op nul plaatsen 

	if(link->ai_node!=NULL && link->parent !=NULL)
		for(child=0; child<4; child++){
			if(link->parent->children[child] == link)
				link->parent->children[child]=NULL;
			link->children[child]->parent=NULL;
			link->children[child]=NULL;
		}
	link->ai_node=NULL;
	link->parent=NULL;

}

void add_ainode_to_list(AINodeList* target_list, AINodeList* added_link)
{
	
	added_link->next=target_list->next;
	// de targetschakel krijgt een opvolger...
	target_list->next=added_link;
	// de nieuwe toegevoegde schakel krijgt een voorganger...
	added_link->prev=target_list;
	added_link->next->prev=added_link;
	
}

void add_ainode_to_tree(AINodeTree* parent_link, move m, AINode* added_child)
{
	added_child->tree_link.parent=parent_link;
	added_child->tree_link.prev_player_move=m;
	parent_link->children[m-1]=(&added_child->tree_link);
}

void free_db(AINodeDB* db)
{
  
	int i;
	AINodeList* tijdelijk;
	AINodeList* cursor = db->all_nodes.next;
	while(cursor!=&db->all_nodes){
		tijdelijk=cursor;
		cursor = cursor->next;
		for( i = 0 ; i < tijdelijk->ai_node->state.width ; i++){
			free(tijdelijk->ai_node->state.tiles[i]);
		}
		free(tijdelijk->ai_node->state.tiles);
		free(tijdelijk->ai_node);
	}

	free(db);

}

void copy_moves(int moves_len, move* moves, AINode* end_ai_node)
{	//count overloopt alle te kopieren moves:
	//to_copy bevat telkens de huidige knoop en wordt na het kopieren verzet naar de parent van deze knoop
	int count;
	AINode* to_copy = end_ai_node;

	for(count=moves_len-1; count>=0; count--){
		moves[count]=to_copy->tree_link.prev_player_move;
		to_copy = to_copy->tree_link.parent->ai_node;
	}
}

AINodeDB* init_db(AINode* root)
{
	int i;
	AINodeDB* db = (AINodeDB*) malloc (sizeof(AINodeDB));
	
	/*TREE ROOT
	***********/
	db->tree_root=&root->tree_link;
	
	/*ALL NODES
	***********/
	db->all_nodes.ai_node=NULL;
	db->all_nodes.next=&root->all_link;
	db->all_nodes.prev=&root->all_link;

	/*TODO NODES
	*************/
	db->todo_nodes.ai_node=NULL;
	db->todo_nodes.next=&root->todo_link;
	db->todo_nodes.prev=&root->todo_link;

	root->all_link.next=&db->all_nodes;
	root->all_link.prev=&db->all_nodes;
	root->all_link.ai_node=root;
	root->todo_link.next=&db->todo_nodes;
	root->todo_link.ai_node=root;
	root->todo_link.prev=&db->todo_nodes;
		
	/*HASHTABEL : INITIALISATIE ALLE RIJEN
	***************************************/

	for(i=0; i < HASHTABLE_SIZE; i++){
		db->nodes_by_state_hashtable[i].ai_node=NULL;
		db->nodes_by_state_hashtable[i].next=&db->nodes_by_state_hashtable[i];
		db->nodes_by_state_hashtable[i].prev=&db->nodes_by_state_hashtable[i];
	}

	/*PLAATSEN EERSTE KNOOP IN HASHTABEL
	**************************************/
	db->nodes_by_state_hashtable[hash(&root->state)].ai_node=NULL;
	db->nodes_by_state_hashtable[hash(&root->state)].next=&root->same_state_hashtable_link;
	db->nodes_by_state_hashtable[hash(&root->state)].next->ai_node=root; //dit kan overbodig worden dankzij init_ainode
	db->nodes_by_state_hashtable[hash(&root->state)].prev=&root->same_state_hashtable_link;
	db->nodes_by_state_hashtable[hash(&root->state)].prev->ai_node=root;//same here

	root->same_state_hashtable_link.next=&db->nodes_by_state_hashtable[hash(&root->state)];
	root->same_state_hashtable_link.prev=&db->nodes_by_state_hashtable[hash(&root->state)];

	db->hash_compares=0;
	db->hash_lookups=0;
	db->all_size=1;
	
    return db;
}

void mark_ainode_done(AINodeDB* db, AINode* ai_node){
	//Hierbij maken we gebruik van de todo_link:	
	ai_node->todo_link.prev->next=ai_node->todo_link.next;
	ai_node->todo_link.next->prev=ai_node->todo_link.prev;
	
}




int is_game_over(LevelState* level_state)
{  
	if(is_game_over_win(level_state) || is_game_over_loss(level_state))
		return 1;

    return 0;
}

int is_game_over_win(LevelState* level_state)
{
	if(level_state->finished!=0)
		return 1;

    return 0;
}

int is_game_over_loss(LevelState* level_state)
{
	if(level_state->player_killed_by_snake != 0 || level_state->player_killed_by_spider != 0)
		return 1;
    return 0;
}

int calculate_state_after_playermove(LevelState* level_state, unsigned short player_move)
{
    /* player_move omzetten naar correcte inputwaarde, dit om zoveel mogelijk code te hergebruiken
	************************************************************************************************/

	if(player_move == MOVE_NORTH)
		level_state->input=KEY_UP;
	if(player_move == MOVE_SOUTH)
		level_state->input=KEY_DOWN;
	if(player_move == MOVE_WEST)
		level_state->input=KEY_LEFT;
	if(player_move == MOVE_EAST)
		level_state->input=KEY_RIGHT;

	/* HERGEBRUIK METHODES DEEL A
	******************************/

	//Eventueel verzetten speler
	interpret_input(level_state);
	
	//ingrijpen slang indien nodig:
	killed_by_snake(level_state); 

	//bewegen spin:
	if(check_if_spider_is_next_player(level_state)){			
			level_state->player_killed_by_spider=1;
		}
		automatic_move_spider(level_state);
		if(check_if_spider_is_next_player(level_state)){			
			level_state->player_killed_by_spider=1;
		}
	
	if(level_state->impossible_move==1)
		return 0;
	else
		return 1;
}

void show_solution(LevelState* level_state, move** moves_to_make, int number_of_moves){
	int i;
	LevelGraphics graphic_output;
    
	//geheugen vrijmaken voor graphics
    graphic_output.graphics = (TileGraphics**) malloc(sizeof(TileGraphics) * level_state->width); 

    for (i = 0; i < level_state->width; i++)
        graphic_output.graphics[i] = (TileGraphics*) malloc(sizeof(TileGraphics) * level_state->height);

	for(i=0; i<number_of_moves;i++){
		calculate_state_after_playermove(level_state, (*moves_to_make)[i]);
		update_level_graphics(&graphic_output, level_state);

        /* Update GUI level using JNI */
        update_level_screen(&graphic_output);

        /* Wait until next logic and graphic frame */
        sleep_millis(FRAME_DURATION_MS);
	}

		for (i = 0; i < level_state->width; i++)
        free(graphic_output.graphics[i]);
	free(graphic_output.graphics);

	free(*moves_to_make);
}


