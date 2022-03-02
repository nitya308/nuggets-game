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
      // count num rows
      // assume all rows same length
      numrows++;               
      numcols = strlen(word);  
      mem_free(word);
    }
    rewind(file);
    char* carr[numrows];

    // fill the char array
    for (int i = 0; i < numrows; i++) {  
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
  if (coordinates!=NULL){
    char** carr = grid->map;
    if (carr[coordinates[0]][coordinates[1]]!= '.' && 
      carr[coordinates[0]][coordinates[1]]!= '#'){
      return false;
    }
    else {
      return true;
    }
  }
  return false; 
}

set_t* grid_isVisible(grid_t* grid, int loc)
{

  //returns set of locations (string literal key for location, 
  //and char* "g" as dummy items.)
  //needs to contain a dummy item, else later one effectively cannot use set_find to
  //distinguish between a location being or not being in the set 


  int* coordinates = grid_locationConvert(grid, loc);
  if (coordinates!=NULL){
    set_t* visible = set_new();
    int gridSize = (grid->ncols) * (grid->nrows);
    char** carr = grid->map;
    char* intToStr = mem_malloc(sizeof(char)*(int)log10(gridSize));
    


    if (carr[coordinates[0]][coordinates[1]]=='#'){
      for(int i = coordinates[0] -1; i <= coordinates[0] + 1; i++){
        for(int j = coordinates[1] -1; j <= coordinates[1] + 1; j++){
          if (carr[coordinates[0]][coordinates[1]]!=' '){
            sprintf(intToStr, "%d", loc);
            set_insert(visible,intToStr, "g");
          }
        }
      }
    }
    
  }
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
      void* arg = visible;
      void* plocations = playerLocations;
      void* goldlocations = gold;

      //insert gold symbols into visible portion
      set_iterate(visible, gold, insertGold);  
      //insert players symbols into visible portion
      set_iterate(visible, plocations, insertPlayers);
      //extend visible using seenbefore locations
      set_iterate(seenBefore, arg, mergeHelper);      
      return visible;
    }
  }
  return NULL;
}

static void insertGold(void* arg, const char* key, void* item)
{
  counters_t* gold = arg;
  int location;
  sscanf(key, "%d", &location);
  if (counters_get(gold, location) > 0) {  // if this location is in gold locations
                                           // insert gold symbol as this item
    item = mem_malloc(sizeof(char));
    sprintf(item, "%c", "*");
  }
}

static void insertPlayers(void* arg, const char* key, void* item)
{
  // if this location is in player location
  // insert player symbol as this item
  set_t* plocations = arg;  
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
  return NULL;
}

static void mergeHelper(void* arg, const char* key, void* item)
{
  set_t* newlyVisible = arg;
  if (set_find(newlyVisible, key) == NULL) {
    set_insert(newlyVisible, key, "g");
  }
}

char* grid_print(grid_t* grid, set_t* locations)
{
  if (grid != NULL && locations != NULL) {
    int gridSize = (grid->ncols) * (grid->nrows);
    char** carr = grid->map;
    int* coordinates;
    char* printString = mem_malloc(sizeof(char) * gridSize);

    char* intToStr = mem_malloc(sizeof(char) * (int)log10(gridSize));
    char symbol;

    //run through all grid locations
    for (int i = 0; i < gridSize; i++) {           
      sprintf(intToStr, "%d", i);

    //add newline chars
      if (i % grid->ncols == 1) {                   
        strcat(printString, "\n");
      }
      //if location is in set
      if (set_find(locations, intToStr) != NULL) {

        //if not dummy character (means gold /player), print the symbol
        symbol = set_find(locations, intToStr);
        if(strcmp(symbol, "g")!=0){                 
           strcat(printString, symbol);            
        }
        else {
          //print the grid character corresponding to the location
          coordinates = grid_locationConvert(grid, i);                           
          strcat(printString, carr[coordinates[0]][coordinates[1]]);
        }
      }
      //if location not in set, print space (indicates not visible)
      else {
        strcat(printString, " ");
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
