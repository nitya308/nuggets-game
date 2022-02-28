//Grid module for nuggets 2022
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libcs50/set.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"
#include "../libcs50/mem.h"
#include "../libcs50/file.h"


/**************** local types ****************/
typedef struct grid {
  char** map;
  int nrows;
  int ncols;     
} grid_t;


grid_t* grid_read(FILE* file)
{
  if(file!=NULL){
    grid_t* grid = mem_malloc(sizeof(grid_t));
      char* word = NULL;
      int numrows = 0;
      int numcols = 0;
      while(word = file_readLine(file)){
        numrows++;                      // count num rows
        numcols = strlen(word);         //assume all rows same length
        mem_free(word);        
      }
      rewind(file);
      char* carr[numrows];
      for (int i = 0; i < numrows; i++){// fill the char array
        carr[i] = file_readLine(file);
      }
      grid->map = carr;
      grid->ncols = numcols;
      grid->nrows = numrows;
      return grid;
    }
    else{
      fprintf(stderr,"Invalid file for reading grid\n");
      return NULL;
    }
}


int* grid_locationConvert(grid_t* grid, int loc)
{
  int coordinates[2];
  if (grid!=NULL){
    if (loc>=0 && loc<(grid->ncols)*(grid->nrows)){
      coordinates[0] = loc/(grid->nrows);
      coordinates[1] = loc%(grid->nrows);
      return coordinates;
    }
  }
  return NULL;
}

bool grid_isOpen(grid_t* grid, int loc)
{
  int* coordinates = grid_locationConvert(grid, loc);
  char** carr = grid->map;
  if (strcmp(carr[coordinates[0]][coordinates[1]],".")!=0){
    return false;
  }
  else{
    return true;
  }
}

set_t* grid_isVisible(grid_t* grid, int loc)
{
}

set_t* grid_updateView(grid_t* grid, int loc, 
    set_t* seenBefore, counters_t* playerLocations, counters_t* gold )
{
  if (grid!=NULL && playerLocations!=NULL && gold !=NULL){
      set_t* visible = grid_isVisible(grid,loc);
      if (visible!=NULL){
        void* arg = visible;
        counters_iterate(playerLocations, arg, matchplayer);
      }
  }
}

set_t* grid_displaySpectator(grid_t* grid, counters_t* playerLocations, counters_t* gold)
{
  
}

static void matchplayer(void* arg, const int key, const int count)
{

}

char* grid_print(grid_t* grid, set_t* playerlocations, counters_t* gold)
{

}

