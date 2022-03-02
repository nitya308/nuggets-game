/*
 * player.c
 *
 * see player.h for more information.
 *
 * Nitya Agarwala, Feb 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libcs50/bag.h"
#include "../libcs50/counters.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/mem.h"
#include "../libcs50/set.h"
#include "grid.h"
#include "server.h"

/**************** file-local global variables ****************/
/* none */

/**************** local types ****************/
struct playerSwap {
  player_t* player;
  int newCoor;
  bool swapped;
};

/**************** global types ****************/
typedef struct player {
  char pID;
  char* name;
  int purse;
  int currCoor;
  set_t* seenBefore;
} player_t;

// function prototypes
player_t* player_new(char* name, grid_t* grid);
bool player_updateCoordinate(player_t* player, hashtable_t allPlayers, grid_t* grid, counters_t* gold, int newCoor);
bool player_moveRegular(player_t* player, char move, game_t* game);
bool player_moveCapital(player_t* player, char move, game_t* game);
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold);
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor);
bool player_quit(char* address, hashtable_t* allPlayers);
void player_delete(player_t* player);
char* player_summary(hashtable_t* allPlayers);

/**************** local functions ****************/
/* not visible outside this file */
static counternode_t* counternode_new(const int key);
static void swap_helper(void* arg, const char* key, void* item);
static void summary_helper(void* arg, char* key, void* item);

/**************** player_new ****************/
/* see player.h for description */
player_t* player_new(char* name, grid_t* grid)
{
  mem_assert(name, "name provided was null");
  player_t* player = mem_malloc(sizeof(player_t));
  if (player == NULL) {
    return NULL;
  }
  char ID = 'A';
  player->pID = ID;
  player->name = mem_malloc_asser(strlen(name) + 1);
  if (player->name == NULL) {
    // error allocating memory for name;
    // cleanup and return error
    mem_free(player);
    return NULL;
  }
  strcpy(player->name, name);
  // set to random coordinate within grid!!!
  int coor = rand();
  while (!grid_isOpen(coor, grid)) {
    coor = rand();
  }
  player->currCoor = coor;
  player->purse = grid_getGold(player->currCoor);
  player->seenBefore = set_new();
  if (player->seenBefore == NULL) {
    // error allocating memory for name;
    // cleanup and return error
    mem_free(player);
    return NULL;
  }
  set_insert(player->seenBefore, player->currCoor, NULL);
  return player;
}

/**************** player_updateCoordinate ****************/
/* see player.h for description */
bool player_updateCoordinate(player_t* player, hashtable_t allPlayers, grid_t* grid, counters_t* gold, int newCoor)
{
  player->currCoor = newCoor;
  set_t playerLocations = player_locations(allPlayers);
  grid_updateView(grid, newCoor, player->seenBefore, playerLocations, gold);
}

/**************** player_moveRegular ****************/
/* see player.h for description */
bool player_moveRegular(player_t* player, char move, game_t* game)
{
  int newCoor;
  int cols = grid_getNumberCols(game->grid);
  switch (move) {
    case 'k':
      newCoor = player->currCoor - cols;
      break;
    case 'h':
      if (player->currCoor % cols == 0) {
        // cannot move in that direction
        return false;
      }
      newCoor = player->currCoor - 1;
      break;
    case 'l':
      if ((player->currCoor + 1) % cols == 0) {
        // cannot move in that direction
        return false;
      }
      newCoor = player->currCoor + 1;
      break;
    case 'j':
      newCoor = player->currCoor + cols;
      break;
    case 'y':
      newCoor = player->currCoor - cols - 1;
      if ((newCoor + 1) % cols == 0) {
        // cannot move in that direction
        return false;
      }
      break;
    case 'u':
      newCoor = player->currCoor - cols + 1;
      if (newCoor % cols == 0) {
        // cannot move in that direction
        return false;
      }
      break;
    case 'b':
      newCoor = player->currCoor + cols - 1;
      if ((newCoor + 1) % cols == 0) {
        // cannot move in that direction
        return false;
      }
      break;
    case 'n':
      newCoor = player->currCoor + cols + 1;
      if (newCoor % cols == 0) {
        // cannot move in that direction
        return false;
      }
      break;
    default:
      return false;
  }
  if (grid_isOpen(game->grid, newCoor)) {
    if (player_swapLocations(player, game->allPlayers, newCoor)) {
      return true;
    }
    else {
      if (player_updateCoordinate(player, game->allPlayers, game->grid, game->gold, newCoor)) {
        player_collectGold(player, game->numGoldLeft, game->gold);
        return true;
      }
      else {
        return false;
      }
    }
  }
}

/**************** player_moveCapital ****************/
/* see player.h for description */
bool player_moveCapital(player_t* player, char move, game_t* game)
{
  int newCoor;
  int move;
  switch (move) {
    case 'K':
      newCoor = player->currCoor - grid_getNumberCols(game->grid);
      break;
    case 'H':
      newCoor = player->currCoor - 1;
      break;
    case 'L':
      newCoor = player->currCoor + 1;
      break;
    case 'J':
      newCoor = player->currCoor + grid_getNumberCols(game->grid);
      break;
    case 'Y':
      newCoor = player->currCoor - grid_getNumberCols(game->grid) - 1;
      break;
    case 'U':
      newCoor = player->currCoor - grid_getNumberCols(game->grid) + 1;
      break;
    case 'B':
      newCoor = player->currCoor + grid_getNumberCols(game->grid) - 1;
      break;
    case 'N':
      newCoor = player->currCoor + grid_getNumberCols(game->grid) + 1;
      break;
    default:
      return false;
  }
  while (grid_isOpen(game->grid, newCoor)) {
    if (player_swapLocations(player, game->allPlayers, newCoor)) {
      return true;
    }
    else {
      if (player_updateCoordinate(player, game->allPlayers, game->grid, game->gold, newCoor)) {
        player_collectGold(player, game->numGoldLeft, game->gold);
        return true;
      }
      else {
        return false;
      }
    }
    newCoor = newCoor + move;
  }
}

/**************** player_collectGold ****************/
/* see player.h for description */
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold)
{
  int newGold = counters_get(gold, player->currCoor);
  if (newGold > 0 && newGold != 251) {
    player->purse = player->purse + newGold;
    *numGoldLeft -= newGold;
    return counters_set(gold, player->currCoor, 251);
  }
  return false;
}

/**************** player_swapLocations ****************/
/* see player.h for description */
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor)
{
  struct playerSwap args = {currPlayer, newCoor, false};
  hashtable_iterate(allPlayers, &args, swapHelper);
  return args.swapped;
}

/**************** swap_helper ****************/
/* swaps players if one at the same location is found */
static void swap_helper(void* arg, const char* key, void* item)
{
  struct playerSwap* args = (struct playerSwap*)arg;
  int newCoor = args->newCoor;
  player_t* player = item;
  if (player->currCoor == newCoor) {
    int temp = player->currCoor;
    player->currCoor = newCoor;
    args->player->currCoor = temp;
    args->swapped = true;
  }
}

/**************** player_quit ****************/
/* see player.h for description */
bool player_quit(char* address, hashtable_t* allPlayers)
{
  player_t* player = hashtable_find(allPlayers, address);
  if (player == NULL) {
    return false;
  }
  player_delete(player);
  player = NULL;
  return true;
}

/**************** player_delete ****************/
/* see player.h for description */
void player_delete(player_t* player)
{
  mem_free(player->name);
  set_delete(player->seenBefore, NULL);
  mem_free(player);
}

/**************** player_summary ****************/
/* see player.h for description */
char* player_summary(hashtable_t* allPlayers)
{
  char* summary;
  hashtable_iterate(allPlayers, &summary, summary_helper);
  return summary;
}

/**************** summary_helper ****************/
/* helps add the summary of each player */
static void summary_helper(void* arg, char* key, void* item)
{
  player_t* player = item;
  char* addition = player->pID;
  char num[4];
  sprintf(num, "%5d ", player->purse);
  strcat(addition, num);
  strcat(addition, player->name);
  strcat(addition, "\n");
  char** summary = arg;
  strcat(*summary, addition);
}

/**************** player_locations ****************/
/* see player.h for description */
set_t* player_locations(hashtable_t* allPlayers)
{
  set_t* locationSet = set_new();
  hashtable_iterate(allPlayers, locationSet, location_helper);
  return locationSet;
}

/**************** location_Helper ****************/
/* helps add the location of each player to the locationSet */
static void location_helper(void* arg, char* key, void* item)
{
  set_t* locationSet = arg;
  player_t* player = item;
  if (player != NULL) {
    set_insert(locationSet, player->currCoor, player->pID);
  }
}