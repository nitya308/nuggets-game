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

/* **************************************** */
int main()
{
  player_t* p1 = NULL;
  player_t* p2 = NULL;
  grid_t* grid = grid_read("testmap.txt");
  int numGoldLeft = 20;
  counters_t* gold = counters_new();
  counters_set(gold, 2, 5);
  counters_set(gold, 4, 5);
  counters_set(gold, 6, 5);
  counters_set(gold, 8, 5);
  hashtable_t* allPlayers = hashtable_new(10);

  // Testing player_new
  p1 = player_new("Alice", grid, &numGoldLeft, gold);

  hashtable_insert(allPlayers, " ", p1);
  hashtable_insert(allPlayers, " ", p2);

  // Testing player_updateCoordinate
  int p1coor = player_getCurrCoor(p1);
  printf("%d\n", p1coor);

  player_updateCoordinate(p1, allPlayers, grid, gold, 6);

  p1coor = player_getCurrCoor(p1);
  printf("%d\n", p1coor);

  set_t* allLocations = grid_displaySpectator(grid, NULL, gold);
  char* printString = grid_print(grid, allLocations);
  printf("%s\n", printString);

  // deleting everything
  set_delete(allLocations, NULL);
  counters_delete(gold);
  grid_delete(grid);
  hashtable_delete(allPlayers, player_delete);
  exit(0);
}