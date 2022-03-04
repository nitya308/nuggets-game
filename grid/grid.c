// Grid module for nuggets 2022
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "counters.h"
#include "mem.h"
#include "file.h"
#include "set.h"
#include "grid.h"

#define PI 3.14159265

/**************** local types ****************/
typedef struct grid {
  char** map;
  int nrows;
  int ncols;
} grid_t;

/******************local functions**************/
// static void insertPlayers(void* arg, const char* key, void* item);
// static void insertGold(void* arg, const char* key, void* item);
static void mergeHelper(void* arg, const char* key, void* item);

grid_t* grid_read(char* filename)
{
  FILE* file = fopen(filename, "r");
  if (file != NULL) {
    grid_t* grid = mem_malloc(sizeof(grid_t));
    char* word = NULL;
    int numrows = 0;
    int numcols = 0;
    while ((word = file_readLine(file))) {
      // count num rows
      // assume all rows same length
      numrows++;
      numcols = strlen(word);
      mem_free(word);
    }
    rewind(file);
    char** carr = mem_malloc(sizeof(char*) * numrows);

    // fill the char array
    for (int i = 0; i < numrows; i++) {
      carr[i] = file_readLine(file);
    }

    grid->map = carr;
    grid->ncols = numcols;
    grid->nrows = numrows;
    fclose(file);
    return grid;
  }
  else {
    fprintf(stderr, "Invalid file for reading grid\n");
    return NULL;
  }
}

int* grid_locationConvert(grid_t* grid, int loc)
{
  int* coordinates = mem_malloc(2 * sizeof(int));
  if (grid != NULL) {
    if (loc >= 0 && loc < (grid->ncols) * (grid->nrows)) {
      coordinates[0] = loc / (grid->ncols);
      coordinates[1] = loc % (grid->ncols);
      return coordinates;
    }
    mem_free(coordinates);
  }
  else{
   mem_free(coordinates); 
  }
  
  return NULL;
}

bool grid_isOpen(grid_t* grid, int loc)
{
  int* coordinates = grid_locationConvert(grid, loc);
  if (coordinates != NULL) {
    char** carr = grid->map;
    if (carr[coordinates[0]][coordinates[1]] != '.' &&
        carr[coordinates[0]][coordinates[1]] != '#') {
      mem_free(coordinates);
      return false;
    }
    else {
      mem_free(coordinates);
      return true;
    }
  }
  return false;
}

set_t* grid_isVisible(grid_t* grid, int loc, set_t* playerLocations, counters_t* gold)
{
  // returns set of locations (string literal key for location,
  // and char* "g" as dummy items.)
  // needs to contain a dummy item, else later one effectively cannot use set_find to
  // distinguish between a location being or not being in the set

  if (grid_isOpen(grid, loc)) {
    // insert the @ symbol into center of visible set
    set_t* visible = set_new();
    char* intToStr = mem_malloc(11);
    sprintf(intToStr, "%d", loc);

    set_insert(visible, intToStr, "@");

    int location;
    char** carr = grid->map;
    int* coordinates = grid_locationConvert(grid, loc);
    bool onPassageEnd = false;

    // for passageways, survey only adjacent points
    if (carr[coordinates[0]][coordinates[1]] == '#') {
      printf("Passage\n");
      for (int i = coordinates[0] - 1; i <= coordinates[0] + 1; i++) {
        for (int j = coordinates[1] - 1; j <= coordinates[1] + 1; j++) {
          if (carr[coordinates[0]][coordinates[1]] != ' ') {
            // convert location back to integer
            location = i * (grid->ncols) + j;
            sprintf(intToStr, "%d", location);
            if(carr[i][j]=='.'){
              onPassageEnd = true;
            }

            if (counters_get(gold, location) > 0 && counters_get(gold, location) != 251) {
              set_insert(visible, intToStr, "*");
            }
            else if (set_find(playerLocations, intToStr) != NULL) {
              set_insert(visible, intToStr, set_find(playerLocations, intToStr));
            }
            else {
              set_insert(visible, intToStr, "g");
            }
          }
        }
      }
    }
    if(carr[coordinates[0]][coordinates[1]]!='#' || onPassageEnd){
      printf("scanning\n");
      // location is in room spot
      int maxr = (grid->ncols) + (grid->nrows);
      double row = 0;
      double col = 0;
      bool oncorner = false;
      double tolerance = 0.1;
      //"look" in all directions
      for (double theta = 0; theta < 2 * PI; theta += PI / 360) {
        for (double radius = 1; radius < maxr; radius += 0.1) {
          row = coordinates[0] + radius * sin(theta);
          col = coordinates[1] + radius * cos(theta);

          //prevent array-out-of-bounds errors
          if((int)row >= grid->nrows || (int)col >= grid->ncols){
            break;
          }
          

          // if close to wall location, stop increasing radius, add to visible set
          if ((carr[(int)row][(int)col] == '|') || (carr[(int)row][(int)col] == '-') || (carr[(int)row][(int)col] == '#') ) {
            // printf("Close to wall\n");
            location = (int)row * (grid->ncols) + (int)col;
            sprintf(intToStr, "%d", location);
            // printf("%d\n", location);

            if (counters_get(gold, location) > 0 && counters_get(gold, location) != 251) {
              set_insert(visible, intToStr, "*");
            }
            else if (set_find(playerLocations, intToStr) != NULL) {
              set_insert(visible, intToStr, set_find(playerLocations, intToStr));
            }
            else {
              set_insert(visible, intToStr, "g");
            }
            // printf("%c\n",carr[(int)row][(int)col]);
            oncorner = false;
            // stop increasing radius
            break;
          }
          // if exactly on some location
          else if (((int)col - col) * ((int)col - col) + ((int)row - row) * ((int)row - row) < tolerance) {
            // if exactly on room location
             //printf("Exactly on: %d %d %c\n",(int)row, (int)col, carr[(int)row][(int)col]);
            if (carr[(int)row][(int)col] == '.') {
              // printf("room\n");
              // add the location, dummy character
              location = (int)row * (grid->ncols) + (int)col;
              sprintf(intToStr, "%d", location);

              if (counters_get(gold, location) > 0 && counters_get(gold, location) != 251) {
                set_insert(visible, intToStr, "*");
              }
              else if (set_find(playerLocations, intToStr) != NULL) {
                set_insert(visible, intToStr, set_find(playerLocations, intToStr));
              }
              else {
                set_insert(visible, intToStr, "g");
              }
              // printf("%c\n",carr[(int)row][(int)col]);
            }
            // if exactly on corner
            // add the location, dummy character

            if (carr[(int)row][(int)col] == '+') {
              // printf("corner\n");
              location = (int)row * (grid->ncols) + (int)col;
              sprintf(intToStr, "%d", location);

              if (counters_get(gold, location) > 0 && (counters_get(gold, location) != 251)) {
                set_insert(visible, intToStr, "*");
              }
              else if (set_find(playerLocations, intToStr) != NULL) {
                set_insert(visible, intToStr, set_find(playerLocations, intToStr));
              }
              else {
                set_insert(visible, intToStr, "g");
              }

              //  printf("%c\n",carr[(int)row][(int)col]);
              oncorner = true;
              // stop increasing radius
              break;
            }
          }
          else {
            //  printf("Continuing\n");
            continue;
          }
          // if just recently encountered corner
          // treat it like a wall (immediately stop increasing radius)
          if (oncorner) {
            // printf("oncorner\n");
            if (carr[(int)row][(int)col] == '+') {
              //   printf("broke\n");
              break;
            }
          }
        }
      }
    }
    mem_free(intToStr);
    mem_free(coordinates);
    return visible;
  }
  return NULL;
}

/**************grid_updateView()*********************/
/*returns an expanded set, from the seenbefore set input. The newly visible
 *portion of the set could contain gold or player symbols if locations match
 */
set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* visible = grid_isVisible(grid, newloc, playerLocations, gold);
    if (visible != NULL) {
      char* intToStr = mem_malloc(11);
      sprintf(intToStr, "%d", newloc);
      mem_free(intToStr);

      // insert gold symbols into visible portion
      // set_iterate(visible, gold, insertGold);
      // insert players symbols into visible portion
      // set_iterate(visible, playerLocations, insertPlayers);
      // extend visible using seenbefore locations
      set_iterate(seenBefore, visible, mergeHelper);
      return visible;
    }
    else {
      set_delete(visible, NULL);
    }
  }
  return NULL;
}

/* static void insertGold(void* arg, const char* key, void* item)
{
  counters_t* gold = arg;
  int location;
  sscanf(key, "%d", &location);
  if (counters_get(gold, location) > 0 && counters_get(gold, location) != 251) {  // if this location is in gold locations
                                                                                  // insert gold symbol as this item
    item = mem_malloc(sizeof(char));
    sprintf(item, "*");
  }
} */

/* static void insertPlayers(void* arg, const char* key, void* item)
{
  // if this location is in player location
  // insert player symbol as this item
  set_t* plocations = arg;
  item = set_find(plocations, key);
} */

/****************grid_displaySpectator()*******************/
/* returns set of all locations in the grid, with gold symbols and player symbol
 *characters in approporatie locxations
 */
set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* allLocations = set_new();
    int gridSize = (grid->ncols) * (grid->nrows);
    // get size of grid
    // convert the int location to string literal, to insert into set
    char* intToStr = malloc(11);
    char* symbol;
    for (int i = 0; i < gridSize; i++) {
      sprintf(intToStr, "%d", i);
      if (!grid_isOpen(grid, i)) {
        set_insert(allLocations, intToStr, "g");
      }
      else {
        if (counters_get(gold, i) > 0 && counters_get(gold, i) != 251) {
          set_insert(allLocations, intToStr, "*");
        }
        else {
          symbol = set_find(playerLocations, intToStr);
          if (symbol != NULL) {
            set_insert(allLocations, intToStr, symbol);
          }
          else {
            set_insert(allLocations, intToStr, "g");
          }
        }
      }
    }
    free(intToStr);
    return allLocations;
  }
  return NULL;
}

static void mergeHelper(void* arg, const char* key, void* item)
{
  set_t* newlyVisible = arg;
  if (set_find(newlyVisible, key) == NULL) {
    set_insert(newlyVisible, key, NULL);
  }
}

char* grid_print(grid_t* grid, set_t* locations)
{
  if (grid != NULL && locations != NULL) {
    int gridSize = (grid->ncols) * (grid->nrows);
    char** carr = grid->map;
    char* printString = mem_malloc((sizeof(char) * gridSize) + grid->nrows + 1);
    char* intToStr = mem_malloc(11);
    char* symbol;
    strcpy(printString, "");

    // run through all grid locations
    for (int i = 0; i < grid->nrows; i++) {
      strcat(printString, "\n");
      for (int j = 0; j < grid->ncols; j++) {
        sprintf(intToStr, "%d", i * (grid->ncols) + j);

        // if location is in set
        if (set_find(locations, intToStr) != NULL) {
          // if not dummy character (means gold /player), print the symbol
          symbol = set_find(locations, intToStr);
          if (strcmp(symbol, "g") != 0) {
            sprintf(printString + i * ((grid->ncols) + 1) + j + 1, "%s", symbol);
            printf("Added a symbol %s\n", symbol);
          }
          else {
            // print the grid character corresponding to the location
            sprintf(printString + i * ((grid->ncols) + 1) + j + 1, "%c", carr[i][j]);
          }
        }
        // if location not in set, print space (indicates not visible)
        else {
          strcat(printString, " ");
        }
      }
    }
    mem_free(intToStr);
    return printString;
  }
  return NULL;
}

int grid_getNumberCols(grid_t* grid)
{
  return grid->ncols;
}

int grid_getNumberRows(grid_t* grid)
{
  return grid->nrows;
}

void grid_delete(grid_t* grid)
{
  char** map = grid->map;
  for (int i = 0; i < grid_getNumberRows(grid); i++) {
    mem_free(map[i]);
  }
  mem_free(grid->map);
  mem_free(grid);
}