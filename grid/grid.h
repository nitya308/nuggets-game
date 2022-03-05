// Grid module for nuggets 2022

#ifndef __GRID_H
#define __GRID_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libcs50/counters.h"
#include "../libcs50/file.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/mem.h"
#include "../libcs50/set.h"

typedef struct grid grid_t;

grid_t* grid_read(char* filename);

int* grid_locationConvert(grid_t* grid, int loc);

bool grid_isOpen(grid_t* grid, int loc);

set_t* grid_isVisible(grid_t* grid, int loc, set_t*playerLocations, counters_t* gold);

set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold);

set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold);

char* grid_print(grid_t* grid, set_t* locations);

int grid_getNumberCols(grid_t* grid);

int grid_getNumberRows(grid_t* grid);

void grid_delete(grid_t* grid);

#endif // __GRID_H
