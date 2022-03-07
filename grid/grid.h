/*
 * grid.h file for grid module
 * performs all functions having to do with the game map
 * including displaying view, visibiilty
 *
 *Matthew Timofeev 2022
 */ 

#ifndef __GRID_H
#define __GRID_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "counters.h"
#include "file.h"
#include "mem.h"
#include "set.h"


/**************** global types ****************/
typedef struct grid grid_t;


/**************** functions ****************/


/**************** grid_read ****************/
/* Create a grid structure from a mapfile name.
 * 
 * Caller provides:
 *  string filename
 * We return:
 *  a pointer to a grid_t structure containing a 2-d char array,
 *  int nrows, int ncols, if file can be opened
 *  Caller must later call grid_delete
 * 
 *  (null) if cannot open file or memory error
 * We do:
 *  Read the file and copy all the characters to a two-dimensional
 *  array in the grid_t structure. Initialize the number of rows 
 *  and number of columns integers to number of rows and columns in 
 *  the grid.
 * Assumption:
 *  File is in valid map format if it can be opened.
 */
grid_t* grid_read(char* filename);


/**************** grid_locationConvert ****************/
/* Converts a 1-dimensional integer to coordinates array.
 * 
 * Caller provides:
 *  pointer to grid_t struct and an integer location
 * We return:
 *  an array of 2 numbers: row coordinate, column coordinate
 *  caller must later free this array
 * 
 *  (null) if invalid grid, integer location < 0, or if
 *  location >= (grid's number of columns * grid's num rows)
 * 
 * We do:
 *  If location,grid valid, do location / number of rows for row coordinate
 *  location % number of rows for column coordinate
 */
int* grid_locationConvert(grid_t* grid, int loc);

/**************** grid_isOpen ****************/
/* Verify whether a location on a grid is room/passage spot
 * 
 * Caller provides:
 *  pointer to grid_t struct and an integer location
 * We return:
 *  true if location points to room or passage spot '.' or '#' on grid
 *  false if points to wall, corner, space
 *  false if location or grid invalid
 * We do:
 *  Call grid_locationConvert on location and grid
 *  Compare against the character stored at that location
 *  in the grid character array
 */
bool grid_isOpen(grid_t* grid, int loc);

/**************** grid_isRoom ****************/
/* Verify whether a location on a grid is room spot
 * 
 * Caller provides:
 *  pointer to grid_t struct and an integer location
 * We return:
 *  true if location points to room spot '.' on grid
 *  false if points to anything else
 *  false if location or grid invalid
 * We do:
 *  Call grid_locationConvert on location and grid
 *  Compare against the character stored at that location
 *  in the grid character array
 */
bool grid_isRoom(grid_t* grid, int loc);


/**************** grid_visible ****************/
/* Give set of visible locations, gold, players, from a 
 *  vantage point in grid.
 * 
 * Caller provides:
 *  pointer to grid_t struct, integer location, 
 *  counters_t* of gold locations, set_t* of player locations
 * We return:
 *  a pointer to set_t of location keys, dummy character items
 *  or gold symbol "*" or other player ID symbols "A", "B", etc..
 *  if gold or other players occupy a location. "@" symbol in the vantage 
 *  point location represents the user.
 *  This is representing set of all locations, gold, and players
 *  visible from the vantage point in the grid.
 * 
 *  NULL if grid or location in grid are invalid
 * 
 * We do:
 *  Call grid_locationConvert on location and grid
 *  loop through all other locations in the grid
 *  if line of sight to the location is not blocked by
 *  wall or corner, and location is less than defined radius
 *  away from the observer, print the integer location to a string
 *  literal, and add it to the set.
 *  For the item, insert "*" if counters_find on the gold
 *  counter returns > 0 for that location, or insert a 
 *  player ID symbol if set_find on the playerlocations set
 *  gives not NULL.
 *  If no players or gold at the location, insert dummy "g" item.
 * 
 *  To determine which locations are blocked or not, REQUIREMENTS spec 
 *  is followed: i.e. only adjacent points visible in passages,
 *  all direct line of sight points visible in rooms.
 */
set_t* grid_isVisible(grid_t* grid, int loc, set_t*playerLocations, counters_t* gold);


/**************** grid_updateView ****************/
/* Give set of "known" and "seen" locations combined, with 
 * gold and player symbols only in "seen" portion.
 * 
 * Caller provides:
 *  pointer to grid_t struct, integer location, 
 *  counters_t* of gold locations, set_t* of player locations
 *  set_t* of seen before locations
 * 
 * We return:
 *  a pointer to set_t of location keys, dummy character items
 *  or gold symbol "*" or other player ID symbols "A", "B", etc..
 *  if gold or players occupy a location.
 *  This is representing set of all known and newly seen locations
 *  with gold and players only in the newly seen portion.
 * 
 *  Caller must free this set
 * 
 *  NULL if grid or location in grid are invalid
 * 
 * We do:
 *  call grid_visible on the given grid, location, players set and 
 *  gold counters to make a visible set. call set_iterate on the 
 *  seenBefore set, and for location keys in there but not in visible,
 *  insert them into visible with dummy item "g", thus erasing
 *  gold and player symbols from no-longer-visible locations.
 *  Then, set_delete the seen before set.
 */
set_t* grid_updateView(grid_t* grid, int newloc,
                       set_t* seenBefore, set_t* playerLocations, counters_t* gold);

/**************** grid_displaySpectator ****************/
/* Give set of all locations in grid with 
 * gold and player symbols in their proper locations.
 * 
 * Caller provides:
 *  pointer to grid_t struct,
 *  counters_t* of gold locations, set_t* of player locations
 *  
 * 
 * We return:
 *  a pointer to set_t of location keys, dummy character items
 *  or gold symbol "*" or other player ID symbols "A", "B", etc..
 *  if gold or players occupy a location.
 *  This is representing a god's eye view of grid and all gold
 *  and players.
 *  
 *  caller must free this set
 * 
 *  NULL if grid or location in grid are invalid
 * 
 * We do:
 *  loop through every location in the grid, printing the
 *  integer to string literal and inserting string as key into 
 *  the set of locations.
 *  For the item, insert "*" if counters_find on the gold
 *  counter returns > 0 for that location, or insert a 
 *  player ID symbol if set_find on the playerlocations set
 *  gives not NULL.
 *  If no players or gold at the location, insert dummy "g" item.
 */
set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold);


/**************** grid_print ****************/
/* Give string representation of set of locations from grid
 * 
 * Caller provides:
 *  pointer to grid_t struct, set_t* of locations in grid
 *  
 * 
 * We return:
 *  a char* string representation of the set of locations
 *  in the grid
 *  
 *  caller must free this string
 * 
 *  NULL if grid or set_t* are null.
 * 
 * We do:
 *  Allocate memory for printstring.
 *  loop through every location in the grid. If set_find for the 
 *  inputted set on that location gives NULL, append a space to
 *  the printstring. If set_find gives dummy item "g", append the 
 *  character from grid char array to the string.
 *  If set_find gives gold symbol or player symbol, append that symbol.
 */
char* grid_print(grid_t* grid, set_t* locations);


/**************** grid_getNumberCols ****************/
/* Give number of columns in the grid
 * 
 * Caller provides:
 *  pointer to grid_t struct
 *
 * We return:
 *  int number of columns stored in the grid_t structure
 *  0 if invalid grid pointer
 */
int grid_getNumberCols(grid_t* grid);

/**************** grid_getNumberRows ****************/
/* Give number of columns in the grid
 * 
 * Caller provides:
 *  pointer to grid_t struct
 *
 * We return:
 *  int number of rows stored in the grid_t structure
 *  0 if invalid grid pointer
 */
int grid_getNumberRows(grid_t* grid);

/**************** grid_delete ****************/
/* free all memory associated with the grid
 * 
 * Caller provides:
 *  pointer to grid_t struct
 *
 * We return:
 *  nothing. 
 * 
 * We do:
 *  Free the memory associated with grid.
 */
void grid_delete(grid_t* grid);

#endif // __GRID_H