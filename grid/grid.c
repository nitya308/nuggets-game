//Matthew Timofeev
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
  char** map;       //2-d array stores grid chars
  int nrows;        //number of rows in the array
  int ncols;        //number of columns in the array
} grid_t;

/******************local functions**************/

static void mergeHelper(void* arg, const char* key, void* item);
static bool isBlocked(grid_t* grid, int rowObsrvr, int colObsrvr, int rowp, int colp);


/******************functions**************/
grid_t* grid_read(char* filename)
{
  FILE* file = fopen(filename, "r");
  if (file != NULL) {
    grid_t* grid = mem_malloc(sizeof(grid_t));

    if (grid==NULL){
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
    if (carr!=NULL){
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
    else{
      fclose(file);
      fprintf(stderr, "Error allocating char array memory.\n");
      return NULL;
    }

  }
  else {
    fprintf(stderr, "Invalid file for reading grid.\n");
    return NULL;
  }
}

int* grid_locationConvert(grid_t* grid, int loc)
{
  int* coordinates = mem_malloc(2 * sizeof(int));
  if (coordinates!=NULL){
    if (grid != NULL) {
      if (loc >= 0 && loc < (grid->ncols) * (grid->nrows)) {
        coordinates[0] = loc / (grid->ncols);
        coordinates[1] = loc % (grid->ncols);
        return coordinates;
      }  
    }
    mem_free(coordinates);
  }
  return NULL;
}

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

set_t* grid_visible(grid_t* grid, int loc, set_t* playerLocations, counters_t* gold)
{
  if (grid_isOpen(grid, loc)) {

    // insert the @ symbol into center of visible set
    set_t* visible = set_new();
    char* intToStr = mem_malloc(11);
    if (intToStr!=NULL){
      sprintf(intToStr, "%d", loc);
    }
    set_insert(visible, intToStr, "@");

    //now begin testing visible locations
    int location;
    int* coordinates = grid_locationConvert(grid, loc);
    printf("player at %d ,%d \n", coordinates[1],coordinates[0]);
    for(int r =0; r< grid->nrows; r++){
      for(int c = 0; c< grid->ncols; c++){
        if(r != coordinates[0] || c != coordinates[1]){
          if(!isBlocked(grid,coordinates[0],coordinates[1], r, c)){
            //printf("visible: %d , %d\n", c,r);
            location = r* (grid->ncols) + c;
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
  //printf("\ntesting for blockage %d , %d\n", colp, rowp);
  char** carr = grid->map;
  double roundError = 0.000000000001;
  char roomSpot = '.';
  char passageSpot = '#';
  char corner = '+';
  char wallh = '-';
  char wallv = '|';

  if(colObsrvr == colp){
    for(int r= rowObsrvr; r< rowp; r++){
      if (carr[r][colp]!=roomSpot){
        return true;
      }
    }
    for(int r= rowObsrvr; r> rowp; r--){
      if (carr[r][colp]!=roomSpot){
        return true;
      }
    }
    return false;
  }
  
  if(rowObsrvr == rowp){
    for(int c= colObsrvr; c< colp; c++){
      if (carr[rowp][c]!=roomSpot){
        return true;
      }
    }
    for(int c= colObsrvr; c>colp; c--){
      if (carr[rowp][c]!=roomSpot){
        return true;
      }
    }
    return false;
  }

    double slope = (double)(rowp-rowObsrvr)/(double)(colp-colObsrvr);
    //printf("slope: %f\n",slope);
    for(int c = colObsrvr + (colp-colObsrvr)/abs(colp-colObsrvr); c!= colp; c+=(colp-colObsrvr)/abs(colp-colObsrvr)){
      double row = (double)rowObsrvr + (double)(c-colObsrvr)*slope;
      int r1 = (int)row;
      int r2 = r1 +1;
      if ((r1-row)*(r1-row) < roundError){
        if (carr[r1][c]!=roomSpot){
          return true;
        }
      }
      if(carr[r1][c]!=roomSpot && carr[r2][c]!=roomSpot){
        return true;
      }
    }
    for(int r = rowObsrvr +(rowp-rowObsrvr)/abs(rowp-rowObsrvr); r!= rowp; r+=(rowp-rowObsrvr)/abs(rowp-rowObsrvr)){
      double col = (double)colObsrvr + (double)(r-rowObsrvr)/slope;
      int c1 = (int)col;
      int c2 = c1 +1;
      if ((c1-col)*(c1-col) < roundError){
        if (carr[r][c1]!=roomSpot){
          return true;
        }
      }
      if(carr[r][c1]!=roomSpot && carr[r][c2]!=roomSpot){
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
    if (intToStr!=NULL){
      sprintf(intToStr, "%d", loc);
    }
    
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


set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* visible = grid_isVisible(grid, newloc, playerLocations, gold);
    set_iterate(seenBefore, visible, mergeHelper);
    return visible;
  }
  return NULL;
}




set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold)
{
  if (grid != NULL) {
    set_t* allLocations = set_new();

    if (allLocations!=NULL){
      int gridSize = (grid->ncols) * (grid->nrows);
      // get size of grid
      // convert the int location to string literal, to insert into set
      char* intToStr = malloc(11);
      char* symbol;
      for (int i = 0; i < gridSize; i++) {
        if (intToStr!=NULL){
           sprintf(intToStr, "%d", i);
        }
       
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
  if (grid!=NULL){
    return grid->ncols;
  }
  return 0;
}

int grid_getNumberRows(grid_t* grid)
{
  if (grid!=NULL){
    return grid->nrows;
  }
  return 0;
}

void grid_delete(grid_t* grid)
{
  if (grid!=NULL){
    char** map = grid->map;
    for (int i = 0; i < grid_getNumberRows(grid); i++) {
      mem_free(map[i]);
    }
    mem_free(grid->map);
    mem_free(grid);
  }
}