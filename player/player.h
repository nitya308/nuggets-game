/* 
 * player.h - header file for CS50 player module
 *
 * The player module stores a structure for a player in the game
 * 
 * Nitya Agarwala 2022
 */

#ifndef __player_H
#define __player_H

#include <stdio.h>
#include <stdbool.h>
#include "../grid/grid.h"
#include "../libcs50/counters.h"
#include "../libcs50/file.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/mem.h"
#include "../libcs50/set.h"

/**************** global types ****************/
typedef struct player player_t;  // opaque to users of the module

/**************** functions ****************/

/**************** FUNCTION ****************/
/**************** player_new ****************/
/* Create a new initialized player structure.
 *
 * We return:
 *   pointer to a new player_t; NULL if error. 
 * We guarantee:
 *   playeret is intialized with an ID, random open coordinate, gold at that coordinate and its name 
 * Caller is responsible for:
 *   later calling player_delete();
 */
player_t* player_new(char* name, grid_t* grid, hashtable_t* allPlayers, int* numGoldLeft, counters_t* gold, int numPlayers);

/**************** player_updateCoordinate ****************/
/* Update the coordinate of a player
 * 
 * Caller provides:
 *   valid pointer to player (not NULL)
 *   a new coordinate
 * We return:
 *   true if success
 *   false if it fails
 * We guarantee:
 *   the player's coordinate will be updated if success
 * We do:
 *   update the player's seenBefore set to add the updated coordinate
 */
bool player_updateCoordinate(player_t* player, hashtable_t* allPlayers, grid_t* grid, counters_t* gold, int newCoor);

/**************** player_moveRegular ****************/
/* Allow player to move once with lowercase key press
 *
 * Caller provides:
 *   valid pointer to player, valid lowercase character and pointer to game struct
 * We do:
 *   calulate new coordinate (left/right/up/down/diagonal)
 *   check if the spot in the grid is open
 *   call player_swap location to swap if another player exists
 *   call player_update_coordinate to update coordinate
 *   call player_collectGold to collect gold if there is any
 * We return:
 *   true if success
 *   false if any error or move was invalid
 */
bool player_moveRegular(player_t* player, char move, hashtable_t* allPlayers, grid_t* grid, counters_t* gold, int* numGoldLeft);

/**************** player_moveCapital ****************/
/* Allow player to move until possible once with uppercase key press
 *
 * Caller provides:
 *   valid pointer to player, valid lowercase character and pointer to game struct
 * We do:
 *   repeat until the move is possible:
 *   calulate new coordinate (left/right/up/down/diagonal)
 *   check if the spot in the grid is open
 *   call player_swap location to swap if another player exists
 *   call player_update_coordinate to update coordinate
 *   call player_collectGold to collect gold if there is any
 * We return:
 *   true if success
 *   false if any error or move was invalid
 */
bool player_moveCapital(player_t* player, char move, hashtable_t* allPlayers, grid_t* grid, counters_t* gold, int* numGoldLeft);

/**************** player_collectGold ****************/
/* Collect gold in new location of there is any
 *
 * Caller provides:
 *   valid pointer to player, 
 *   pointer to integer number of gold left
 *   pointer to counter with gold
 * We do:
 *   if gold exists in that location
 *   increment player's purse by number of gold
 *   decrement numGoldLeft in Game by number of gold
 *   set that counter to 251 (means no more gold left)
 * We return:
 *   true if we colelcted gold
 *   false if no gold was found at player's location
 */
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold);

/**************** player_swapLocations ****************/
/* if there is another player in that locatin, swap the location
 *
 * Caller provides:
 *   valid pointer to current player, 
 *   pointer to hashtable with all player
 *   int of new location player is trying to move to
 * We do:
 *   if another player exists in that location
 *   swap their coordinates with the given current player
 * We return:
 *   true if we swapped a player
 *   false if no other player was found in the new coordinate
 */
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor);

/**************** player_quit ****************/
/* Deletes a player when it quits and sets that item in hashtable to null
 *
 * Caller provides:
 *   a valid pointer to a player and hashtable with all players
 * We do:
 *  find player with given address
 *  call player_delete on the player
 *  set it to null
 * We return:
 *  true if player was found an deleted
 *  false if player was not found
 */
bool player_quit(const char* address, hashtable_t* allPlayers, counters_t* gold, int* numGoldLeft);


/**************** player_locations ****************/
/* Prepapres a set of (int player location, char player IDs)
 *
 * Caller provides:
 *   a valid pointer to hashtable with all players
 * We do:
 *   iterate over hashtable and add each players location and ID to a set
 * We return:
 *   the set of all player locations and IDs
 */
set_t* player_locations(hashtable_t* allPlayers);

/**************** player_locations ****************/
/* Prepares summary of all players and their gold
 *
 * Caller provides:
 *   a valid pointer to hashtable with all players
 * We do:
 *   iterate over hashtable and add each player's summary
 * We return:
 *   the character pointer to the summary
 */
char* player_summary(hashtable_t* allPlayers);

/**************** player_delete ****************/
/* Deletes a player struct and frees all associated memory
 *
 * Caller provides:
 *   a valid pointer to a player.
 * We do:
 *   we ignore NULL player.
 *   we free all memory we allocate for this player.
 */
void player_delete(player_t* player);

#endif // __player_H

// Getter method prototypes
int player_getCurrCoor(player_t* player);
char* player_getID(player_t* player);
int player_getpurse(player_t* player);
int player_getRecentGold(player_t* player);
set_t* player_getSeenBefore(player_t* player);
void player_setSeenBefore(player_t* player, set_t* set);
void player_print(player_t* player);