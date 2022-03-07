/*
 * grid.C file for grid module
 * performs all functions having to do with the game map
 * including displaying view, visibiilty
 *
 *Matthew Timofeev 2022
 */
#include "grid.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "counters.h"
#include "file.h"
#include "mem.h"
#include "set.h"
#include "grid.h"

#define RADIUS 1000

/**************** local types ****************/
typedef struct grid {
  char** map;
  int nrows;
  int ncols;
} grid_t;

/******************local functions**************/
/**************mergeHelper************************/
/* Merge the set this is iterated through into the argument set
 * Does:
 *  Takes set input as void* arg, converts back to set.
 *  As the functions is iterated through a seen-before locations
 *  set by grid_updateView, if a seen-before location does not
 *  appear in newly-visible locations set, inserts the location
 *  key with dummy item "g" into the newly-visible set.
 */
static void mergeHelper(void* arg, const char* key, void* item);

/**************isBlocked************************/
/* Verify whether a point is visible from a vantage point in grid
 * Does:
 *  Takes input vantage point, and point to be observed, both
 *  as two int coordinates. Calculates a line through the points
 *  and if any walls or corners fall on that line in the grid
 *  between vantage point and observation point, returns true
 *  (meaning point is blocked by a wall somewhere)
 *  if line is clear, then returns false. grid_visible calls this
 *  on each point in the grid, to determine whether to add to visible
 *  set.
 *  Passage spots block other spots. 
 *  All points directly adjacent to observer are by default not blocked.
 */
static bool isBlocked(grid_t* grid, int rowObsrvr, int colObsrvr, int rowp, int colp);

/******************global functions**************/

/******************grid_read**************/
/* see grid.h */
grid_t* grid_read(char* filename)
{
  FILE* file = fopen(filename, "r");
  if (file != NULL) {
    grid_t* grid = mem_malloc(sizeof(grid_t));

    if (grid == NULL) {
      fclose(file);
      fprintf(stderr, "Error allocating grid memory.\n");
      return NULL;
    }

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
    if (carr != NULL) {
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
      fclose(file);
      fprintf(stderr, "Error allocating char array memory.\n");
      return NULL;
    }
  }
  else {
    fprintf(stderr, "Invalid file for reading grid\n");
    return NULL;
  }
}

/******************grid_locationConvert**************/
/* see grid.h */
int* grid_locationConvert(grid_t* grid, int loc)
{
  int* coordinates = mem_malloc(2 * sizeof(int));
  if (coordinates != NULL) {
    if (grid != NULL) {
      if (loc >= 0 && loc < (grid->ncols) * (grid->nrows)) {
        coordinates[0] = loc / (grid->ncols);
        coordinates[1] = loc % (grid->ncols);
        return coordinates;
      }
      mem_free(coordinates);
    }
    else {
      mem_free(coordinates);
    }
  }
  return NULL;
}

/******************grid_isOpen**************/
/* see grid.h */
bool grid_isOpen(grid_t* grid, int loc)
{
  char roomSpot = '.';
  char passageSpot = '#';
  int* coordinates = grid_locationConvert(grid, loc);
  if (coordinates != NULL) {
    char** carr = grid->map;
    if (carr[coordinates[0]][coordinates[1]] != roomSpot &&
        carr[coordinates[0]][coordinates[1]] != passageSpot) {
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

/******************grid_isRoom**************/
/* see grid.h */
bool grid_isRoom(grid_t* grid, int loc)
{
  char roomSpot = '.';
  int* coordinates = grid_locationConvert(grid, loc);
  if (coordinates != NULL) {
    char** carr = grid->map;
    if (carr[coordinates[0]][coordinates[1]] != roomSpot) {
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
  if (grid_isOpen(grid, loc)) {
    // insert the @ symbol into center of visible set
    set_t* visible = set_new();
    char* intToStr = mem_malloc(11);

    if (intToStr != NULL) {
      sprintf(intToStr, "%d", loc);
    }

    set_insert(visible, intToStr, "@");

    // now begin testing visible locations
    int location;
    int* coordinates = grid_locationConvert(grid, loc);

    //loop through every location in grid to see if it is blocked (invisible)
    for (int r = 0; r < grid->nrows; r++) {
      for (int c = 0; c < grid->ncols; c++) {
        if (r != coordinates[0] || c != coordinates[1]) {

          if (!isBlocked(grid, coordinates[0], coordinates[1], r, c)) {
            if((coordinates[0]-r)*(coordinates[0]-r) + (coordinates[1]-c)*(coordinates[1]-c) 
                <= RADIUS*RADIUS ){
              //if distance is less than radius
              //if not blocked, print the location to string key
              location = r * (grid->ncols) + c;
              sprintf(intToStr, "%d", location);

              //insert appropriate symbol into set 
              //(either player symbol, gold symbol, or dummy "g")
              if (!grid_isOpen(grid, location)) {
                set_insert(visible, intToStr, "g");
              }
              else if (counters_get(gold, location) > 0 && counters_get(gold, location) != 251) {
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
    }
    mem_free(intToStr);
    mem_free(coordinates);
    return visible;
  }
  return NULL;
}

static bool isBlocked(grid_t* grid, int rowObsrvr, int colObsrvr, int rowp, int colp)
{
  char** carr = grid->map;
  double roundError = 0.000000000001;
  char roomSpot = '.';
  int unitVec;

  //if observer and point in same column, iterate from observer to point
  // if runs into any non-room spot, determines point as blocked
  if (colObsrvr == colp) {
    unitVec = (rowp - rowObsrvr)/abs(rowp - rowObsrvr);
    for (int r = rowObsrvr + unitVec; r != rowp; r += unitVec) {
      if (carr[r][colp] != roomSpot) {
        return true;
      }
    }
    return false;
  }
  //observer and point in same row
  if (rowObsrvr == rowp) {
    unitVec = (colp - colObsrvr)/abs(colp - colObsrvr);
    for (int c = colObsrvr + unitVec; c != colp; c += unitVec) {
      if (carr[rowp][c] != roomSpot) {
        return true;
      }
    }
    return false;
  }

  //observer and point are diagonally separated
  //define a slope between the points
  double slope = (double)(rowp - rowObsrvr) / (double)(colp - colObsrvr);

  //start at observer, iterate until point
  //declaring a signed unitVec simplifies two cases into one
  unitVec = (colp - colObsrvr)/abs(colp - colObsrvr);
  for (int c = colObsrvr + unitVec; c != colp; c += unitVec) {
    double row = (double)rowObsrvr + (double)(c - colObsrvr) * slope;
    int r1 = (int)row;
    int r2 = r1 + 1;
    //if line falls exactly on a point (within roundoff error)
    //and point isn't room spot, this is a blockage
    if ((r1 - row) * (r1 - row) < roundError) {
      if (carr[r1][c] != roomSpot) {
        return true;
      }
    }

    //if line is between two points, and both points aren't room spots,
    //this blocks the point
    if (carr[r1][c] != roomSpot && carr[r2][c] != roomSpot) {
      return true;
    }
  }

  //repeat this procedure for columns 
  //(in case the observer and point were only 1 row apart)
  unitVec = (rowp - rowObsrvr)/abs(rowp - rowObsrvr);
  for (int r = rowObsrvr + unitVec; r != rowp; r += unitVec) {
    double col = (double)colObsrvr + (double)(r - rowObsrvr) / slope;
    int c1 = (int)col;
    int c2 = c1 + 1;
    if ((c1 - col) * (c1 - col) < roundError) {
      if (carr[r][c1] != roomSpot) {
        return true;
      }
    }
    if (carr[r][c1] != roomSpot && carr[r][c2] != roomSpot) {
      return true;
    }
  }
  return false;
}

/******************grid_updateView**************/
/* see grid.h */
set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* visible = grid_isVisible(grid, newloc, playerLocations, gold);
    if (visible != NULL) {
      set_iterate(seenBefore, visible, mergeHelper);
      set_delete(seenBefore,NULL);
      return visible;
    }
  }
  return seenBefore;
}

static void mergeHelper(void* arg, const char* key, void* item)
{
  set_t* newlyVisible = arg;
  if (set_find(newlyVisible, key) == NULL) {
    set_insert(newlyVisible, key, "g");
  }
}

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
  if (grid != NULL) {
    return grid->ncols;
  }
  return 0;
}

int grid_getNumberRows(grid_t* grid)
{
  if (grid != NULL) {
    return grid->nrows;
  }
  return 0;
}

void grid_delete(grid_t* grid)
{
  if (grid != NULL) {
    char** map = grid->map;
    for (int i = 0; i < grid_getNumberRows(grid); i++) {
      mem_free(map[i]);
    }
    
    mem_free(grid->map);
    mem_free(grid);
  }
}

