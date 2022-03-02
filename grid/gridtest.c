//Matthew Timofeev 2022
//Testing file for grid module


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"

int main(const int argc, char* argv[])
{
  
    

    grid_t* grid = grid_read("../visdemo.txt");
    if(grid == NULL){
      fprintf(stderr,"Error reading grid\n");
      return 1;
    }

    for(int i =0; i< grid_getNumberRows(grid)*grid_getNumberCols(grid); i++){
      set_t* visible = grid_updateView(grid,14,NULL,NULL,NULL);
      char* printstring = grid_print(grid, visible);
      printf("%s", printstring);
      set_delete(visible, NULL);
      mem_free(printstring);
    }
    
  
    return 0;
}