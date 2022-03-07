//Matthew Timofeev 2022
//Testing file for grid module


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"


int main(const int argc, char* argv[])
{
  grid_t* grid = NULL;
  char* printString = NULL;
  set_t* allLocations = NULL;
  set_t* visible = NULL;
  set_t* playerLoc = NULL;
  counters_t* gold = NULL;
  set_t* seenbefore = NULL;


  //test reading grid from invalid file (does not exist)
  //gives error
  printf("Reading grid from cats.txt (nonexistent)...\n");
  grid = grid_read("cats.txt");
  if(grid == NULL){
      fprintf(stderr,"Error reading grid\n");
  }  

  //read a small grid file (valid, and assume right format) into a grid structure
  printf("Reading grid from hole.txt...\n");
  grid = grid_read("../maps/hole.txt");
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
  allLocations = grid_displaySpectator(grid, NULL,NULL);

  //print the set to a string
  printf("Printing the view to string...\n");
  printString = grid_print(grid, allLocations);
  printf("Spectator sees the following: \n%s\n",printString);
  set_delete(allLocations,NULL);
  mem_free(printString);

  
  //now test if locations are open or not
  //test with invalid locations <0 or > num columns*num rows -1 (not open)
  printf("Test negative locations and locations > grid size...\n"
          "Should give false...\n");

  if (!grid_isOpen(grid, -14)){
    fprintf(stderr,"-14 does not correspond to open location.\n");
  }
  if (!grid_isOpen(grid, grid_getNumberCols(grid)*grid_getNumberRows(grid))){
    fprintf(stderr,"%d does not correspond to open location.\n", 
      grid_getNumberCols(grid)*grid_getNumberRows(grid));
  }

  

  //location conversion also gives error on these invalid locations
  printf("Testing location convert on invalid locations. Should give errors...\n");
  int* coordinates = grid_locationConvert(grid, -14);
  if (coordinates==NULL){
    fprintf(stderr,"-14 is invalid location.\n");
  }
  else{
    mem_free(coordinates);
  }

  coordinates = grid_locationConvert(grid,grid_getNumberCols(grid)*grid_getNumberRows(grid));
  if (coordinates==NULL){
    fprintf(stderr,"%d is invalid location.\n", grid_getNumberCols(grid)*grid_getNumberRows(grid));
  }
  else{
    mem_free(coordinates);
  }
  
  //Test with valid locations
  printf("Testing with valid locations from 0 to grid size...\n");
  for(int i =0; i< grid_getNumberCols(grid)*grid_getNumberRows(grid); i++){
    coordinates = grid_locationConvert(grid,i);
    if(grid_isOpen(grid, i)){
      if(grid_isRoom(grid,i)){
        printf("The point at row %d , col %d is room spot\n", coordinates[0], coordinates[1]);
      }
      else{
        printf("The point at row %d , col %d is passage spot\n", coordinates[0], coordinates[1]);
      }
    }
    else{
      printf("The point at row %d , col %d is not open\n", coordinates[0], coordinates[1]);
    }
    mem_free(coordinates);
  }
  


  //Now test with player locations and gold symbols
  //Will not display symbols in not-open locations
  //Should not see player "C"

  playerLoc = set_new();
  set_insert(playerLoc, "1507","A");
  set_insert(playerLoc, "1538","B");
  set_insert(playerLoc, "1056","C");
  set_insert(playerLoc, "1084","D");

  gold = counters_new();
  for(int i =0; i< grid_getNumberCols(grid)*grid_getNumberRows(grid); i+=17){
    counters_add(gold,i);
  }

  //display spectator's view

  printf("Maxing set of locations, players and gold on grid...\n");
  allLocations =  grid_displaySpectator(grid, playerLoc,gold);
  printString = grid_print(grid, allLocations);
  //print the set to a string
  printf("Printing the view to string...\n");
  printf("Specator sees the populated grid: \n%s\n", printString);
  set_delete(allLocations,NULL);
  mem_free(printString);


  //testing the visiblity feature
  printf("Testing visibility (radius defined 5)\n");
  printf("calculating player A's view\n");
  visible = grid_isVisible(grid,1507,playerLoc,gold);
  printString = grid_print(grid, visible);
  printf("Player A sees the following: \n%s\n",printString);
  mem_free(printString);
  set_delete(visible,NULL);

  printf("calculating player B's view\n");
  visible = grid_isVisible(grid,1538,playerLoc,gold);
  printString = grid_print(grid, visible);
  printf("Player B sees the following: \n%s\n",printString);
  mem_free(printString);
  set_delete(visible,NULL);

  printf("calculating player C's view\n");
  visible = grid_isVisible(grid,1056,playerLoc,gold);
  printString = grid_print(grid, visible);
  printf("Player C sees the following: \n%s\n",printString);
  mem_free(printString);
  set_delete(visible,NULL);

  printf("calculating player D's view\n");
  visible = grid_isVisible(grid,1084,playerLoc,gold);
  printString = grid_print(grid, visible);
  printf("Player D sees the following: \n%s\n",printString);
  mem_free(printString);
  set_delete(visible,NULL);


  //now, iterate player s location through the whole map,
  //updating its view each time (expanding their seen-before set).
  //print the growing set each time.
  //they will not see gold/players in rooms they have left

  //by this testing loop setup, player  will pass through
  //invalid locations, but its view will not be updated for those

  seenbefore = grid_isVisible(grid, 1507, playerLoc, gold);
  for(int i =1507; i >= 0; i--){
     seenbefore = grid_updateView(grid,i,seenbefore,playerLoc,gold);
     printString = grid_print(grid, seenbefore);
     printf("New player's cumulative view: \n%s\n",printString);
     mem_free(printString);
   }
  set_delete(seenbefore,NULL);
  set_delete(playerLoc,NULL);
  counters_delete(gold);
  grid_delete(grid);
  return 0;
}