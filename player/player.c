/*
 * player.c
 *
 * see player.h for more information.
 *
 * Nitya Agarwala, Feb 2022
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../grid/grid.h"
#include "counters.h"
#include "file.h"
#include "hashtable.h"
#include "mem.h"
#include "set.h"

/**************** file-local global variables ****************/
/* none */
static const int MaxNameLength = 50;

/**************** global types ****************/
typedef struct player {
  char* pID;
  char* name;
  int purse;
  int recentGoldCollected;
  int currCoor;
  set_t* seenBefore;
} player_t;

/**************** local types ****************/
typedef struct playerSwap {
  player_t* player;
  int newCoor;
  bool swapped;
} player_swapstruct;

// function prototypes
player_t* player_new(char* name, grid_t* grid, hashtable_t* allPlayers,
  int* numGoldLeft, counters_t* gold, int numPlayers);
bool player_updateCoordinate(player_t* player, hashtable_t* allPlayers,
  grid_t* grid, counters_t* gold, int newCoor);
bool player_moveRegular(player_t* player, char move, hashtable_t* allPlayers,
  grid_t* grid, counters_t* gold, int* numGoldLeft);
bool player_moveCapital(player_t* player, char move, hashtable_t* allPlayers,
  grid_t* grid, counters_t* gold, int* numGoldLeft);
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold);
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor);
bool player_quit(const char* address, hashtable_t* allPlayers, 
  counters_t* gold,int* numGoldLeft);
void player_delete(player_t* player);
char* player_summary(hashtable_t* allPlayers);
set_t* player_locations(hashtable_t* allPlayers);
void player_print(player_t* player);
void itemPrint2(FILE* fp, const char* key, void* item);

// Getter method prototypes
int player_getCurrCoor(player_t* player);
char* player_getID(player_t* player);
int player_getpurse(player_t* player);
int player_getRecentGold(player_t* player);
set_t* player_getSeenBefore(player_t* player);
void player_setSeenBefore(player_t* player, set_t* set);

/**************** local functions ****************/
/* not visible outside this file */
static void swap_helper(void* arg, const char* key, void* item);
static void summary_helper(void* arg, const char* key, void* item);
static void location_helper(void* arg, const char* key, void* item);
static void stringfree(void* item);

/**************** player_new ****************/
/* see player.h for description */
player_t* player_new(char* name, grid_t* grid, hashtable_t* allPlayers, 
                            int* numGoldLeft, counters_t* gold, int numPlayers)
{
  mem_assert(name, "name provided was null");
  mem_assert(grid, "grid provided was null");
  mem_assert(gold, "gold provided was null");
  player_t* player = mem_malloc(sizeof(player_t));
  if (player == NULL) {
    return NULL;
  }
  int intID = 65 + numPlayers;
  char ID = (char)intID;
  char* pID = malloc(2);
  sprintf(pID, "%c", ID);
  player->pID = pID;

  // truncate an over-length real name to MaxNameLength characters
  if (strlen(name) > MaxNameLength) {
    name[MaxNameLength] = '\0';
  }

  // replacing with an underscore _ any character for which both isgraph() & isblank() are false
  int i = 0;
  while (name[i] != '\0') {
    if (!isgraph(name[i]) && !isblank(name[i])) {
      name[i] = '_';
    }
    i++;
  }

  player->name = mem_malloc(strlen(name) + 1);
  if (player->name == NULL) {
    // error allocating memory for name;
    // cleanup and return error
    mem_free(player);
    return NULL;
  }
  strcpy(player->name, name);

  // set to random coordinate within grid!!!
  int coor = rand() % (grid_getNumberRows(grid) * grid_getNumberRows(grid));
  while (!grid_isOpen(grid, coor)) {
    coor = rand() % (grid_getNumberRows(grid) * grid_getNumberRows(grid));
  }
  player->purse = 0;

  if (!player_swapLocations(player, allPlayers, coor)) {
    player->currCoor = coor;
    player_collectGold(player, numGoldLeft, gold);
    player->recentGoldCollected = player->purse;
  }

  player->seenBefore = set_new();
  if (player->seenBefore == NULL) {
    // error allocating memory for name;
    // cleanup and return error
    mem_free(player);
    return NULL;
  }
  return player;
}

/**************** player_updateCoordinate ****************/
/* see player.h for description */
bool player_updateCoordinate(player_t* player, hashtable_t* allPlayers, 
                                  grid_t* grid, counters_t* gold, int newCoor)
{
  player->currCoor = newCoor;
  set_t* playerLocations = player_locations(allPlayers);
  set_t* newSeenBefore = grid_updateView(grid, newCoor, player->seenBefore, playerLocations, gold);
  player->seenBefore = newSeenBefore;
  set_delete(playerLocations, stringfree);
  return true;
}

static void stringfree(void* item)
{
  free(item);
}

/**************** player_moveRegular ****************/
/* see player.h for description */
bool player_moveRegular(player_t* player, char move, hashtable_t* allPlayers, 
                              grid_t* grid, counters_t* gold, int* numGoldLeft)
{
  int newCoor;
  int cols = grid_getNumberCols(grid);
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
  if (grid_isOpen(grid, newCoor)) {
    if (player_swapLocations(player, allPlayers, newCoor)) {
      return true;
    }
    else {
      if (player_updateCoordinate(player, allPlayers, grid, gold, newCoor)) {
        if (!player_collectGold(player, numGoldLeft, gold)) {
          player->recentGoldCollected = 0;
        }
        return true;
      }
      else {
        return false;
      }
    }
  }
  return false;
}

/**************** player_moveCapital ****************/
/* see player.h for description */
bool player_moveCapital(player_t* player, char move, hashtable_t* allPlayers, 
                              grid_t* grid, counters_t* gold, int* numGoldLeft)
{
  int recentGold = 0;  // counts all the gold collected across multiple moves here
  while (true) {
    int newCoor;
    int cols = grid_getNumberCols(grid);
    switch (move) {
      case 'K':
        newCoor = player->currCoor - cols;
        break;
      case 'H':
        if (player->currCoor % cols == 0) {
          // cannot move in that direction
          return false;
        }
        newCoor = player->currCoor - 1;
        break;
      case 'L':
        if ((player->currCoor + 1) % cols == 0) {
          // cannot move in that direction
          return false;
        }
        newCoor = player->currCoor + 1;
        break;
      case 'J':
        newCoor = player->currCoor + cols;
        break;
      case 'Y':
        newCoor = player->currCoor - cols - 1;
        if ((newCoor + 1) % cols == 0) {
          // cannot move in that direction
          return false;
        }
        break;
      case 'U':
        newCoor = player->currCoor - cols + 1;
        if (newCoor % cols == 0) {
          // cannot move in that direction
          return false;
        }
        break;
      case 'B':
        newCoor = player->currCoor + cols - 1;
        if ((newCoor + 1) % cols == 0) {
          // cannot move in that direction
          return false;
        }
        break;
      case 'N':
        newCoor = player->currCoor + cols + 1;
        if (newCoor % cols == 0) {
          // cannot move in that direction
          return false;
        }
        break;
      default:
        return false;
    }
    if (!grid_isOpen(grid, newCoor)) {
      break;
    }
    if (!player_swapLocations(player, allPlayers, newCoor)) {
      if (player_updateCoordinate(player, allPlayers, grid, gold, newCoor)) {
        player_collectGold(player, numGoldLeft, gold);
        recentGold += player->recentGoldCollected;
      }
    }
  }
  player->recentGoldCollected = recentGold;
  return true;
}

/**************** player_collectGold ****************/
/* see player.h for description */
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold)
{
  int newGold = counters_get(gold, player->currCoor);
  if (newGold > 0 && newGold != 251) {
    player->purse = player->purse + newGold;
    *numGoldLeft -= newGold;
    player->recentGoldCollected = newGold;
    counters_set(gold, player->currCoor, 251);
    return true;
  }
  player->recentGoldCollected = 0;
  return false;
}

/**************** player_swapLocations ****************/
/* see player.h for description */
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor)
{
  struct playerSwap args = {currPlayer, newCoor, false};
  if (allPlayers != NULL) {
    hashtable_iterate(allPlayers, &args, swap_helper);
  }

  return args.swapped;
}

/**************** swap_helper ****************/
/* swaps players if one at the same location is found */
static void swap_helper(void* arg, const char* key, void* item)
{
  struct playerSwap* args = (struct playerSwap*)arg;
  player_t* currPlayer = args->player;
  int newCoor = args->newCoor;
  player_t* player = item;
  if (player->currCoor == newCoor) {
    player->currCoor = currPlayer->currCoor;
    currPlayer->currCoor = newCoor;
    args->swapped = true;
  }
}

/**************** player_quit ****************/
/* see player.h for description */
bool player_quit(const char* address, hashtable_t* allPlayers, counters_t* gold, int* numGoldLeft)
{
  player_t* player = hashtable_find(allPlayers, address);
  if (player == NULL) {
    return false;
  }
  int goldOnLocation = counters_get(gold, player->currCoor);
  if (goldOnLocation ==251){
    goldOnLocation = 0;
  }
  counters_set(gold, player->currCoor, goldOnLocation + player->purse);
  *numGoldLeft += player->purse;
  player->currCoor = -1;  // removes player from everyone's map
  return true;
}

/**************** player_delete ****************/
/* see player.h for description */
void player_delete(player_t* player)
{
  mem_free(player->pID);
  mem_free(player->name);
  set_delete(player->seenBefore, NULL);
  mem_free(player);
}

/**************** player_summary ****************/
/* see player.h for description */
char* player_summary(hashtable_t* allPlayers)
{
  char* summary = malloc(1000000);
  strcpy(summary, "");
  hashtable_iterate(allPlayers, &summary, summary_helper);
  return summary;
}

/**************** summary_helper ****************/
/* helps add the summary of each player */
static void summary_helper(void* arg, const char* key, void* item)
{
  player_t* player = item;
  if (player != NULL) {
    char* addition = malloc(15);
    strcpy(addition, player->pID);
    char* num = malloc(15);
    sprintf(num, "%5d", player->purse);
    strcat(addition, num);
    strcat(addition, " ");
    strcat(addition, player->name);
    strcat(addition, "\n");
    char** sum = arg;
    strcat(*sum, addition);
    free(addition);
    free(num);
  }
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
static void location_helper(void* arg, const char* key, void* item)
{
  set_t* locationSet = arg;
  player_t* player = item;
  if (player != NULL) {
    char* coorString = malloc(11);
    sprintf(coorString, "%d", player->currCoor);
    char* pIDString = malloc(2);
    // so you need a way to store things
    strcpy(pIDString, player->pID);
    set_insert(locationSet, coorString, pIDString);
    free(coorString);
  }
}

// Getter methods
int player_getCurrCoor(player_t* player)
{
  if (player == NULL) {
    return -1;
  }
  return player->currCoor;
}

char* player_getID(player_t* player)
{
  if (player == NULL) {
    return '\0';
  }
  return player->pID;
}

int player_getpurse(player_t* player)
{
  if (player == NULL) {
    return -1;
  }
  return player->purse;
}

int player_getRecentGold(player_t* player)
{
  if (player == NULL) {
    return -1;
  }
  return player->recentGoldCollected;
}

set_t* player_getSeenBefore(player_t* player)
{
  if (player == NULL) {
    return NULL;
  }
  return player->seenBefore;
}

void player_setSeenBefore(player_t* player, set_t* set)
{
  if (set != NULL) {
    player->seenBefore = set;
  }
}

void player_print(player_t* player)
{
  printf("Name: %s\n", player->name);
  printf("Coordinate: %d\n", player->currCoor);
  printf("ID: %s\n", player->pID);
  printf("Gold: %d\n", player->purse);
  printf("Recent gold: %d\n", player->recentGoldCollected);
}