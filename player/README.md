# Player Module
Player module contains functions to create new players, quit players, move player locations/update their information, collect gold, swap player locations.

## Contents
player.c
player.h 
playertest.c
Makefile
.gitignore

## Compilation
To compile, type `make`. To test, type `make test`. For valgrind, `make valgrind` For cleaning, `make clean`

## Testing
Results of running `make test`, which calls gridtest.c, are printed to testing.out

## Extra credit
When player quits, drops gold in their last location. 