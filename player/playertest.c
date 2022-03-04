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

  // creating a simple grid for testing
  grid_t* grid = grid_read("testmap.txt");

  // creating a simple gold counter for testing
  int numPlayers = 0;
  int numGoldLeft = 20;
  counters_t* gold = counters_new();
  counters_set(gold, 6, 5);
  counters_set(gold, 8, 5);
  counters_set(gold, 13, 5);
  counters_set(gold, 17, 5);

  // Create a hashtable of players for testing
  hashtable_t* allPlayers = hashtable_new(10);

  // Testing player_new
  p1 = player_new("Alice", grid, &numGoldLeft, gold, numPlayers);
  numPlayers++;
  p2 = player_new("Bob", grid, &numGoldLeft, gold, numPlayers);
  numPlayers++;

  // Add both players to the hashtable
  hashtable_insert(allPlayers, " ", p1);
  hashtable_insert(allPlayers, " ", p2);

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
  player_moveCapital(p1, 'X', allPlayers, grid, gold, &numGoldLeft);
  printf("\n%s\n", "PLAYER 1 should not have changed:");
  player_print(p1);

  set_t* allLocations = grid_displaySpectator(grid, NULL, gold);
  char* printString = grid_print(grid, allLocations);
  printf("%s", printString);

  /*   // printing grid with initial players and their locations
    set_print(player_locations(allPlayers), stdout, NULL);
    allLocations = grid_displaySpectator(grid, player_locations(allPlayers), gold);
    printString = grid_print(grid, allLocations);
    printf("%s\n", printString);

    // Testing player_updateCoordinate
    int p1coor = player_getCurrCoor(p1);
    printf("%d\n", p1coor);

    player_updateCoordinate(p1, allPlayers, grid, gold, 6);

    p1coor = player_getCurrCoor(p1);
    printf("%d\n", p1coor);

    allLocations = grid_displaySpectator(grid, NULL, gold);
    printString = grid_print(grid, allLocations);
    printf("%s\n", printString);

    // deleting everything
    set_delete(allLocations, NULL);
    counters_delete(gold);
    grid_delete(grid);
    hashtable_delete(allPlayers, player_delete); */
  exit(0);
}

void itemPrint(FILE* fp, const char* key, void* item)
{
  fprintf(fp, "%s ", (char*)item);
}