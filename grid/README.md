# Grid Module
The grid module contains the functions to create a map from a map file, to calculate a player's visibility field of view, to calculate viewable gold and other player symbols, to create the spectator's view, to verify whether a point on the map is a room/passage spot or not. Also contains function to print a player or spectator view to a string for easy sending as a message.

## Contents
grid.c
grid.h 
gridtest.c
Makefile
.gitignore

## Compilation
To compile, type `make`. To test, type `make test`. For valgrind, `make valgrind` For cleaning, `make clean`

## Testing
Results of running `make test`, which calls gridtest.c, are printed to testing.out

## Assumptions
Assumes that map files are in valid format.

## Extra credit
Implements radius of visibility. To change radius, change defined value in grid.c
