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

static grid_t* buildSimpleGrid();

/* **************************************** */
int main()
{
  player_t* p1 = NULL;
  player_t* p2 = NULL;
  grid_t* grid = buildSimpleGrid();
  int numGoldLeft = 20;
  counters_t* gold = counters_new();
  counters_set(gold, 2, 5);
  counters_set(gold, 4, 5);
  counters_set(gold, 6, 5);
  counters_set(gold, 8, 5);
  printf("%s\n", "HERE1");
  fflush(stdout);
  p1 = player_new("Alice", grid, &numGoldLeft, gold);
  printf("%s\n", "HERE2");
  fflush(stdout);
  exit(0);
}

static grid_t* buildSimpleGrid()
{
  char* row1 = "+---+";
  char* row2 = "+...+";
  char* row3 = "#...#";
  char* row4 = "+...+";
  char* row5 = "+---+";
  char** map[5];
  map[0] = row1;
  map[1] = row2;
  map[2] = row3;
  map[3] = row4;
  map[4] = row5;
  grid_t* grid = {map, 5, 5};
  return grid;
}