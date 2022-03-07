/*
 * playertest.c - test program for players module
 *
 * Nitya Agarwala, CS50, Jan 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libcs50/file.h"
#include "player.h"

// static function prototypes
void itemPrint(FILE* fp, const char* key, void* item);
void freeString(void* item);
void deletePlayer(void* item);

/* **************************************** */
int main()
{
  player_t* p1 = NULL;
  player_t* p2 = NULL;

  // creating a simple grid for testing
  grid_t* grid = grid_read("../maps/testmap.txt");

  // creating a simple gold counter for testing
  int numPlayers = 0;
  int numGoldLeft = 20;
  counters_t* gold = counters_new();
  counters_set(gold, 6, 5);
  counters_set(gold, 8, 5);
  counters_set(gold, 13, 5);
  counters_set(gold, 17, 5);

  set_t* allLocations = grid_displaySpectator(grid, NULL, gold);
  char* printString = grid_print(grid, allLocations);
  printf("\n%s", printString);
  set_delete(allLocations, NULL);
  mem_free(printString);

  // Create a hashtable of players for testing
  hashtable_t* allPlayers = hashtable_new(10);

  // Testing player_new
  p1 = player_new("Alice", grid, allPlayers, &numGoldLeft, gold, numPlayers);
  numPlayers++;
  p2 = player_new("Bob", grid, allPlayers, &numGoldLeft, gold, numPlayers);
  numPlayers++;

  // Add both players to the hashtable
  hashtable_insert(allPlayers, "1", p1);
  hashtable_insert(allPlayers, "2", p2);

  // Print player 1
  printf("%s\n", "PLAYER 1:");
  player_print(p1);
  printf("\n%s\n", "PLAYER 2:");
  // Print player 2
  player_print(p2);

  // Testing player_moveRegular
  player_moveRegular(p1, 'h', allPlayers, grid, gold, &numGoldLeft);
  // Print player 1
  printf("\n%s\n", "PLAYER 1 moved to the left:");
  player_print(p1);

  // Testing player_moveCapital
  player_moveCapital(p1, 'H', allPlayers, grid, gold, &numGoldLeft);
  // Print player 1
  printf("\n%s\n", "PLAYER 1 moved to the left till border:");
  player_print(p1);

  // Testing an invalid key
  player_moveRegular(p1, 'x', allPlayers, grid, gold, &numGoldLeft);
  printf("\n%s\n", "PLAYER 1 should not have changed:");
  player_print(p1);

  // Testing an invalid key
  player_moveCapital(p1, 'X', allPlayers, grid, gold, &numGoldLeft);
  printf("\n%s\n", "PLAYER 1 should not have changed:");
  player_print(p1);

  // Testing player_swapLocations
  player_moveRegular(p2, 'k', allPlayers, grid, gold, &numGoldLeft);
  printf("\n%s\n", "PLAYER 2 should have swapped with PLAYER 1:");
  printf("%s\n", "PLAYER 1:");
  player_print(p1);
  printf("\n%s\n", "PLAYER 2:");
  player_print(p2);

  // Testing player_locations
  set_t* locations = player_locations(allPlayers);
  printf("\n%s\n", "LOCATIONS SET:");
  set_print(locations, stdout, itemPrint);

  // Testing player_summary
  char* sum = player_summary(allPlayers);
  printf("\n\nSUMMARY: \n%s\n", sum);
  free(sum);

  // printing the grid
  allLocations = grid_displaySpectator(grid, NULL, gold);
  printString = grid_print(grid, allLocations);
  printf("\n%s", printString);

  // Testing player_delete
  mem_free(printString);
  set_delete(locations, freeString);
  set_delete(allLocations, NULL);
  counters_delete(gold);
  grid_delete(grid);
  hashtable_delete(allPlayers, deletePlayer);
  exit(0);
}

/******** delete_player *********/
/* deletes each player in a hashtable */
void deletePlayer(void* item)
{
  player_t* player = item;
  player_delete(player);
}

/******** itemPrint *********/
/* prints each location and player in a set */
void itemPrint(FILE* fp, const char* key, void* item)
{
  fprintf(fp, " location:%s player:%s ", key, (char*)item);
}

/******** freeString *********/
/* free character pointer item in a set */
void freeString(void* item){
  if (item!=NULL) {
    free(item);
  }
}