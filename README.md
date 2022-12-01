# Nuggets Game
## CS50: Software Design and Implementation at Dartmouth College 🌲

### Overview and Features
This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
* The rooms and passages are defined by a *map* loaded by the server at the start of the game.
* The gold nuggets are randomly distributed in *piles* within the rooms.
* Up to 26 players, and one spectator, may play a given game.
* Each player is randomly dropped into a room when joining the game.
* Players move about, collecting nuggets when they move onto a pile.
* Players can only see the part of the grid directly in their line of sight, i.e. not blocked diagonally or directly by room walls. This is calculated by the grid module.
* If a player moves into a spot occupied by another player, they are swapped.
* When all gold nuggets are collected, the game ends and a summary is printed.
* The winner is the player who collects the most gold.

## Client

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
 
 ## Server
The server takes in a map file as an input. 
<br/>
A map file is a `.txt` file that contains a grid of ascii characters representing a map of walls, empty spots, passageways, and solid rock. <br/>
The server also takes in an optional positive integer seed for the random-number generator.<br/>
It announces the port number in the terminal and sends messages back to the client.<br/>
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

### Important Functions

This function handles all messages passed from the client to the server based on protocol in [requirements spec](https://github.com/cs50winter2022/nuggets-info/blob/main/REQUIREMENTS.md#network-protocol).
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

This function initializes new player to game to the game, sending OK, GRID message to the player
```c
static bool playerJoin(char* name, const addr_t client);
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

This function is called by hashtable_iterate and sends GOLD message to player, telling them the gold they recently collected, the gold in their purse, and the remaining gold in game.
```c
static void sendGoldMessage(void* arg, const char* addr, void* item);
```

This function generates an array of random number of gold for each gold pile, summing up to GoldTotal
```c
static void generateGoldDistribution(int numGoldPiles, int* arr);
```
For a detailed list of prototypes and code please see [IMPLEMENTATION.md](https://github.com/nitya308/nuggets-game/blob/main/IMPLEMENTATION.md)

## Player Module
Handles each player in the game

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

### Important Functions

Updates the coordinates of the given player to the new coordinate give.
```c
bool player_updateCoordinate(player_t* player, int newCoor);
```

Handles a single move by the player and takes a lowecase character to specify move direction.
```c
bool player_moveRegular(player_t* player, char move, game_t* game);
```

Handles a single moving until it is no longer possible in that direction. It takes an uppercase character to specify move direction.
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

A function that deletes a player struct and frees all associated memory
```c
void player_delete(player_t* player);
```

A function that prepares and returns a string summary of all players and the gold they have when the game ends
```c
char* player_summary(hashtable_t* allPlayers);
```

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
### Important Functions

Takes int location input and grid structure.finds the location in the grid and states whether it is passage/room spot or not.
```c
bool grid_isOpen(grid_t* grid, int location);
```

Takes int location input and calculates a set of integer keys and character items, representing all the locations that are visible from the input location, according to requirements spec. Returns this set
```c
set_t* grid_isVisible(grid_t* grid, int location, set_t* playerLocations, counters_t* gold);
```
Modifies the player’s seen-before set of locations to include the newly visible portions of the map. Includes gold and other player symbols as items only in the newly visible portion.
```c
set_t* grid_updateView(grid_t* grid, int newlocation, set_t* seenBefore, set_t* playerLocations, counters_t* gold);
```

For a detailed list of all functions and code explanations please see [IMPLEMENTATION.md](https://github.com/nitya308/nuggets-game/blob/main/IMPLEMENTATION.md)
