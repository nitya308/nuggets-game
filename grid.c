// Grid module for nuggets 2022
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libcs50/counters.h"
#include "../libcs50/file.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/mem.h"
#include "../libcs50/set.h"

/**************** local types ****************/
typedef struct grid {
  char** map;
  int nrows;
  int ncols;
} grid_t;

grid_t* grid_read(char* filename)
{
  FILE* file = fopen(filename, "r");
  if (file != NULL) {
    grid_t* grid = mem_malloc(sizeof(grid_t));
    char* word = NULL;
    int numrows = 0;
    int numcols = 0;
    while (word = file_readLine(file)) {
      numrows++;               // count num rows
      numcols = strlen(word);  // assume all rows same length
      mem_free(word);
    }
    rewind(file);
    char* carr[numrows];
    for (int i = 0; i < numrows; i++) {  // fill the char array
      carr[i] = file_readLine(file);
    }
    grid->map = carr;
    grid->ncols = numcols;
    grid->nrows = numrows;
    return grid;
  }
  else {
    fprintf(stderr, "Invalid file for reading grid\n");
    return NULL;
  }
}

int* grid_locationConvert(grid_t* grid, int loc)
{
  int coordinates[2];
  if (grid != NULL) {
    if (loc >= 0 && loc < (grid->ncols) * (grid->nrows)) {
      coordinates[0] = loc / (grid->nrows);
      coordinates[1] = loc % (grid->nrows);
      return coordinates;
    }
  }
  return NULL;
}

bool grid_isOpen(grid_t* grid, int loc)
{
  int* coordinates = grid_locationConvert(grid, loc);
  char** carr = grid->map;
  if (strcmp(carr[coordinates[0]][coordinates[1]], ".") != 0) {
    return false;
  }
  else {
    return true;
  }
}

set_t* grid_isVisible(grid_t* grid, int loc)
{
}

/**************grid_updateView()*********************/
/*returns an expanded set, from the seenbefore set input. The newly visible
 *portion of the set could contain gold or player symbols if locations match
 */
set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* visible = grid_isVisible(grid, newloc);
    if (visible != NULL) {
      void* arg = seenBefore;
      void* plocations = playerLocations;
      void* goldlocations = gold;
      set_iterate(visible, gold, insertgold);
      set_iterate(visible, plocations, insertplayers);
      set_iterate(visible, arg, insertHelper);  // insert visible location
                                                // into seenBefore set
                                                // if not already there
    }
  }
}

static void insertgold(void* arg, const char* key, void* item)
{
  counters_t* gold = arg;
  int location;
  sscanf(key, "%d", &location);
  if (counters_get(gold, location) > 0) {  // if this location is in gold locations
                                           // insert gold symbol as this item
    item = mem_malloc(sizeof(char));
    strcpy(item, "*");
  }
}

static void insertplayers(void* arg, const char* key, void* item)
{
  set_t* plocations = arg;  // if this location is in player locations
                            // insert player symbol as this item
  item = set_find(plocations, key);
}

/****************gold_displaySpectator()*******************/
/* returns set of all locations in the grid, with gold symbols and player symbol
 *characters in approporatie locxations
 */
set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold)
{
  set_t* allLocations = set_new();
  int gridSize = (grid->ncols) * (grid->nrows);
  // get size of grid
  // convert the int location to string literal, to insert into set
  char* intToStr = mem_malloc(sizeof(char) * (int)log10(gridSize));
  for (int i = 0; i < gridSize; i++) {
    sprintf(intToStr, "%d", i);
    if (counters_get(gold, i) > 0) {
      set_insert(allLocations, intToStr, "*");
    }
    else {
      set_insert(allLocations, intToStr, set_find(playerLocations, intToStr));
    }
  }
}

static void insertHelper(void* arg, const char* key, void* item)
{
  set_t* seenBefore = arg;
  if (set_find(seenBefore, key) == NULL) {
    set_insert(seenBefore, key, item);
  }
}

char* grid_print(grid_t* grid, set_t* locations,
                 set_t* playerlocations, counters_t* gold)
{
  if (grid != NULL && locations != NULL && playerlocations != NULL && gold != NULL) {
    int gridSize = (grid->ncols) * (grid->nrows);
    char** carr = grid->map;
    int* coordinates;
    char* printString = mem_malloc(sizeof(char) * gridSize);

    char* intToStr = mem_malloc(sizeof(char) * (int)log10(gridSize));
    char symbol;
    for (int i = 0; i < gridSize; i++) {
      if (i % grid->ncols == 1) {
        strcat(printString, "\n");
      }
      sprintf(intToStr, "%d", i);
      if (set_find(locations, intToStr) != NULL) {
        symbol = set_find(locations, intToStr);
        strcat(printString, symbol);
      }
      else {
        coordinates = grid_locationConvert(grid, i);
        strcat(printString, carr[coordinates[0]][coordinates[1]]);
      }
    }
  }
}

int grid_getNumberCols(grid_t* grid)
{
  return grid->ncols;
}

int grid_getNumberRows(grid_t* grid)
{
  return grid->nrows;
}

static void grid_delete(grid_t* grid)
{
  mem_free(grid->map);
  mem_free(grid);
}
