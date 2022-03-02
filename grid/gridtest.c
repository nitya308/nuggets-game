//Matthew Timofeev 2022
//Testing file for grid module


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"

int main(const int argc, char* argv[])
{
  grid_t* grid;
  //test reading grid from invalid file (does not exist)
  //gives error
  printf("Reading grid from cats.txt (nonexistent)...\n");
  grid = grid_read("cats.txt");
  if(grid == NULL){
      fprintf(stderr,"Error reading grid\n");
  }  

  //read a small grid file (valid, and assume right format) into a grid structure
  printf("Reading grid from visdemo.txt...\n");
  grid = grid_read("visdemo.txt");
  if(grid == NULL){
    fprintf(stderr,"Error reading grid\n");
  }

  //print grid dimensions
  printf("Grid dimenstions are %d rows by %d cols\n", 
      grid_getNumberRows(grid), grid_getNumberCols(grid));


  //now make the spectator's view of the grid
  //first, make set of all locations in grid
  //(do not pass in gold or other player symbols for now)

  printf("Maxing set of locations...\n");
  set_t* allLocations = set_new();
  allLocations = grid_displaySpectator(grid, NULL,NULL);

  //print the set to a string

  printf("Printing the view to string...\n");
  char* printString = grid_print(grid, allLocations);
  printf("Spectator sees the following: \n%s\n",printString);
  set_delete(allLocations,NULL);

  set_t* visible = set_new();
  
  int* coordinates = grid_locationConvert(grid,38);
  printf("Coordinates are %d , %d\n", coordinates[0], coordinates[1]);
  //for(int i =0; i< grid_getNumberRows(grid)*grid_getNumberCols(grid); i++){
    
    visible = grid_updateView(grid,38,visible,NULL,NULL);
    
  // }
  char* printstring = grid_print(grid, visible);
  printf("%s", printstring);
  set_delete(visible, NULL);
  mem_free(printstring);
  
    return 0;
}