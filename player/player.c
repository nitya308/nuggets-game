/*
 * bag.c - CS50 'bag' module
 *
 * see bag.h for more information.
 *
 * David Kotz, April 2016, 2017, 2019, 2021
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

// function prototypes
player_t* player_new(char* name, grid_t* grid);
bool player_updateCoordinate(player_t* player, int newCoor);
bool player_moveRegular(player_t* player, char move, game_t* game);
bool player_moveCapital(player_t* player, char move, game_t* game);
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold);
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor);
bool player_quit(char* address, hashtable_t* allPlayers);
void player_delete(player_t* player);
char* player_summary(hashtable_t* allPlayers);

static void swapHelper(void* arg, const char* key, void* item);
static void summaryHelper(void* arg, char* key, void* item);

typedef struct player {
  char pID;
  char* name;
  int purse;
  int currCoor;
  set_t* seenBefore;
} player_t;

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

bool player_updateCoordinate(player_t* player, int newCoor)
{
  player->currCoor = newCoor;
  return set_insert(player->seenBefore, newCoor, NULL);
}

bool player_moveRegular(player_t* player, char move, game_t* game)
{
  int newCoor;
  switch (move) {
    case 'k':
      newCoor = player->currCoor - grid_getNumberCols(game->grid);
      break;
    case 'h':
      newCoor = player->currCoor - 1;
      break;
    case 'l':
      newCoor = player->currCoor + 1;
      break;
    case 'j':
      newCoor = player->currCoor + grid_getNumberCols(game->grid);
      break;
    case 'y':
      newCoor = player->currCoor - grid_getNumberCols(game->grid) - 1;
      break;
    case 'u':
      newCoor = player->currCoor - grid_getNumberCols(game->grid) + 1;
      break;
    case 'b':
      newCoor = player->currCoor + grid_getNumberCols(game->grid) - 1;
      break;
    case 'n':
      newCoor = player->currCoor + grid_getNumberCols(game->grid) + 1;
      break;
    default:
      return false;
  }
  if (grid_isOpen(game->grid, newCoor)) {
    if (player_swapLocations(player, game->allPlayers, newCoor)) {
      return true;
    }
    else {
      if (player_updateCoordinate(player, newCoor)) {
        player_collectGold(player, game->numGoldLeft, game->gold);
        return true;
      }
      else {
        return false;
      }
    }
  }
}

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
      if (player_updateCoordinate(player, newCoor)) {
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

struct playerSwap {
  player_t* player;
  int newCoor;
  bool swapped;
};

bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor)
{
  struct playerSwap args = {currPlayer, newCoor, false};
  hashtable_iterate(allPlayers, &args, swapHelper);
  return args.swapped;
}

static void swapHelper(void* arg, const char* key, void* item)
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

void player_delete(player_t* player)
{
  mem_free(player->name);
  set_delete(player->seenBefore, NULL);
  mem_free(player);
}

char* player_summary(hashtable_t* allPlayers)
{
  char* summary;
  hashtable_iterate(allPlayers, &summary, summaryHelper);
  return summary;
}

static void summaryHelper(void* arg, char* key, void* item)
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