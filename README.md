# Nuggets Game
## CS50: Software Design and Implementation at Dartmouth College 🌲

### Overview
This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.
The winner is the player who collects the most gold.
Our map file is located in the `maps` folder, named `add-drop.txt`.

## Features: Client

The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.
 
### User interface
Players will be able to move around the map using the following keystrokes:
 
 * `Q` quit the game.
 * `h` move left, if possible
 * `l` move right, if possible
 * `j` move down, if possible
 * `k` move up, if possible
 * `y` move diagonally up and left, if possible
 * `u` move diagonally up and right, if possible
 * `b` move diagonally down and left, if possible
 * `n` move diagonally down and right, if possible
 
 ## Features: Server
The server takes in a map file as an input. A map file is a `.txt` file that contains a grid of ascii characters representing a map of walls, empty spots, passageways, and solid rock. The server also takes in an optional positive integer seed for the random-number generator.
It announces the port number in the terminal and sends messages back to the client.
Any errors are logged to our log file which we keep as stderr

### Run Instructions
In order to make the client and server, simply type in `make all` into the command line, 
as instructed in the Makefile. To test, simply type `./server 2>server.log maps/mapfile.txt`
with the name of some map as the mapfile. Once the server announces the port number, in another
window run `./client 2>player.log hostname port playerName` or just `./client 2>spectator.log hostname port`
depending on if you want to be a player or spectator. To run valgrind, type `valgrind` or `myvalgrind`
followed by the appropriate test arguments.

## Stucture and modules

## Server

### Data structures

#### `allPlayers`:
This is a hashtable (from libscs50 data structures) that stores all the players in the game. The key of the hashtable is a string representation of the player's address. The item is a player_t struct as defined in the player module.

#### `addrID`:
This is a hashtable (from libscs50 data structures) that maps (char* address, address index in `addresses`), where the address index is the index at which `addresses` store the actual addr_t of the address.

#### `addresses`:
This is an array of size MaxPlayers + 1 which stores all the addr_t of clients that have joined the game.
Note: The last slot of `addresses` addresses[MaxPlayers] is a slot reserved for spectator's address.

#### `grid`:
This is the grid_t struct. Refer to `grid.h` for more information.

#### `gold`:
This is a counters (from libcs50 data structures) that maps (int location, gold at that location). It stores the locations of all the gold pile and the gold value in that pile.

#### `game`:
This holds all the information about the game:

struct game {
  hashtable_t* allPlayers;
  hashtable_t* addrID;
  addr_t* addresses;
  int* numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  int spectatorAddressID;
  int port;
}

### Definition of function prototypes

This function validates the command-line arguments, printing to stderr if any errors are encountered, and sends a message to the server depending on whether the client is a player or spectator.
```c
static int parseArgs(const int argc, char* argv[]);
```

This function performs error checks, reads from stdin, and calls message_send with the appropriate messages.
```c
static bool handleInput(void* arg);
```

This function handles all messages passed from the client to the server based on protocol in [requirements spec](https://github.com/cs50winter2022/nuggets-info/blob/main/REQUIREMENTS.md#network-protocol).
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

This is a function to check if the given file path name is readable.
```c
static bool isReadable(char* pathName);
```

This function initializes new player to game to the game, sending OK, GRID message to the player
```c
static bool playerJoin(char* name, const addr_t client);
```

This function checks if a given string is only filled with spaces
```c
static bool isEmpty(const char* name);
```

This function adds a spectator to the server. If there is already an existing spectator, QUIT the existing spectator and update spectator address. Send GRID, GOLD, DISPLAY message to new spectator.
```c
static void spectatorJoin(const addr_t* address);
```

This function loads the map.txt given by the caller into a grid_t and stores it
```c
static void buildGrid(grid_t* grid, char** argv);
```

This function ends the game and send GOLD, DISPLAY, and QUIT messages to all clients.
```c
static void endGame();
```

This function is called in hashtable_delete for game->allPlayers and deletes the player, freeing up memory.
```c
static void deletePlayer(void* item);
```

This function deletes the item in the set
```c
static void itemDelete(void* item);
```

This function is called in hashtable_iterate, sending DISPLAY message with the grid to players.
```c
static void sendDisplayMessage(void* arg, const char* addr, void* item);
```

This function is called by hashtable_iterate and sends GOLD message to player, telling them the gold they recently collected, the gold in their purse, and the remaining gold in game.
```c
static void sendGoldMessage(void* arg, const char* addr, void* item);
```

This function is called by hashtable_iterate and sends QUIT GAME OVER message to clients
```c
static void sendEndMessage(void* arg, const char* addr, void* item);
```

This function updates the spectator's display if the spectator exists.
```c
static void updateSpectatorDisplay();
```

This function initializes the game, allocating memory for the game struct and initialize the variables in it.
```c
static void initializeGame(char** argv);
```

This function generates a random number of gold piles and a random number of gold in each pile for the game.
```c
static void initializeGoldPiles();
```

This function generates an array of random locations on the grid to put the gold piles.
```c
static void generateRandomLocations(int numGoldPiles, int* arr);
```

This function generates an array of random number of gold for each gold pile, summing up to GoldTotal
```c
static void generateGoldDistribution(int numGoldPiles, int* arr);
```

## player module

### Data structures

#### `player_t`
This data structure stores information for each player in the game. It has the player ID, the player name, an integer purse, an integer representation of the current coordinate, and a set of all coordinates seenBefore.
```c
typedef struct player {
  char pID;
  char* name;
  int purse;
  int currCoor;
  set_t* seenBefore;
} player_t;
```

#### `playerSwap`
This structure stores the current player, the new coordinate it is trying to move to and a boolean variable to show if it was swapped. This struct is passed to hashtable_iterate as it checks if any of the players have the same coordinate as the new coordinate and swap them.
```c
struct playerSwap {
  player_t* player;
  int newCoor;
  bool swapped;
};
```

### Definition of function prototypes

A function that returns a pointer to a new initialized player struct.
```c
player_t* player_new(char* name, grid_t* grid);
```

A function that updates the coordinates of the given player to the new coordinate give.
```c
bool player_updateCoordinate(player_t* player, int newCoor);
```

A function that handles a single move by the player and takes a lowecase character to specify move direction.
```c
bool player_moveRegular(player_t* player, char move, game_t* game);
```

A function that handles a single moving until it is no longer possible in that direction. It takes an uppercase character to specify move direction.
```c
bool player_moveCapital(player_t* player, char move, game_t* game);
```

A function that collects gold if there is any in a grid location.
```c
bool player_collectGold(player_t* player, int* numGoldLeft, counters_t* gold);
```

A function that checks if another player is in a new location and swaps with the current player if there is.
```c
bool player_swapLocations(player_t* currPlayer, hashtable_t* allPlayers, int newCoor);
```

A function that deletes a player when it quits and sets that item in hashtable to null
```c
bool player_quit(char* address, hashtable_t* allPlayers);
```

A function that deletes a player struct and frees all associated memory
```c
void player_delete(player_t* player);
```

A function that prepares and returns a string summary of all players and the gold they have when the game ends
```c
char* player_summary(hashtable_t* allPlayers);
```

A function that returns a set of (int player locations and char player IDs)
```c
set_t* player_locations(hashtable_t* allPlayers)


## Grid module
Handles all functionality to do with the grid and calculates visibility
### Data structures
Grid structure stores:
  2D array of chars, representing the grid
  Integer height
  Integer width
  
```c
typedef struct grid{
char** map;
int nrows;
int ncols;
} grid_t;
```
### Function prototypes 
Reads from text file stores each char in a 2D array of characters stores the 2D array in Game data structure.
```c
grid_t* grid_read(char* filename);
```
Takes int location input and grid structure.finds the location in the grid and states whether it is passage/room spot or not.
```c
bool grid_isOpen(grid_t* grid, int location);
```
Takes int location input and grid structure.finds the location in the grid and states whether it is room spot.
```c
bool grid_isRoom(grid_t* grid, int location);
```
Takes int location input and calculates a set of integer keys and character items, representing all the locations that are visible from the input location, according to requirements spec. Returns this set
```c
set_t* grid_isVisible(grid_t* grid, int location, set_t* playerLocations, counters_t* gold);
```
Modifies the player’s seen-before set of locations to include the newly visible portions of the map. Includes gold and other player symbols as items only in the newly visible portion.
```c
set_t* grid_updateView(grid_t* grid, int newlocation, set_t* seenBefore, set_t* playerLocations, counters_t* gold);
```
Creates a set of locations and characters to display at that location to represent other players and gold (the whole map is visible)
```c
set_t* grid_displaySpectator(grid_t* grid, set_t* playerLocations, counters_t* gold)
```
A function which takes an integer input, grid number of columns, grid number of rows. Returns 2D location coordinate
```c
int* grid_locationConvert(grid_t*, int location);
```
creates a string of visible locations from a set returned by grid_updateView or grid_displaySpectator  
```c
char* grid_print(grid_t* grid, set_t* locations);
```
Gives number of rows in grid
```c
int grid_getNumberRows(grid_t* grid);
```
Gives number of cols in grid
```c
int  grid_getNumberCols(grid_t* grid);
```
Deletes the given grid
```c
static void grid_delete(grid_t* grid);
```
Helper function to merge two sets (taking the union of locations seen before, and adding in any newly visible locations).
```c
static void mergeHelper(void* arg, const char* key, void* item)
```
Helper function to determine whether a given input point is visible or not from a given vantage point in the grid. 
```c
static bool isBlocked(grid_t* grid, int rowObsrvr, int colObsrvr, int rowp, int colp);


