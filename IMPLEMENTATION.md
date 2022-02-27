# CS50 Nuggets
## Implementation Spec
### add-drop Winter 2022

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes player, grid and message modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor

* Client: Ashna
* Server: Vico
* Player module: Nitya
* Grid module including visibility: Matthew

Each team member is responsible for testing and documentation of their module/program based on a consistent style defined by CS50 style guidelines.

## Player

### Data structures

The player side of the program does not use any data structures.

### Definition of function prototypes

> For function, provide a brief description and then a code block with its function prototype.
> For example:

A function to parse the command-line arguments, initialize the game struct, initialize the message module, and (BEYOND SPEC) initialize analytics module.

```c
static int parseArgs(const int argc, char* argv[]);
```
### Detailed pseudo code

> For each function write pseudocode indented by a tab, which in Markdown will cause it to be rendered in literal form (like a code block).
> Much easier than writing as a bulleted list!
> For example:

#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player

---

## Server

### Data structures

#### allPlayers
This is a hashtable (from libscs50 data structures) that stores all the players in the game. The key of the hashtable is a string representation of the player's address. The item is a player data type as defined above.

#### game
This holds all the information about the game:
typedef struct game {
  hashtable_t* allPlayers;
  hashtable_t* addresses;
  int numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  address_t* spectatorAddress;
} game_t;

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct and initialize the message module.

```c
static int parseArgs(const int argc, char* argv[]);

```

### Detailed pseudo code

#### `parseArgs`:

	validate commandline
	verify map file can be opened for reading
	if seed provided
		verify it is a valid seed number
		seed the random-number generator with that seed
	else
		seed the random-number generator with getpid()

#### `initializeGame`:

	pass seed if it exists or call srand(getpid())
	call grid_read to store map in a 2D array of characters
	drop random Gold piles in random rooms
	initialize the 'message' module
	initialize the network and announce the port number
	call message_loop(), to await clients

#### `spectatorJoin`:

	if spectator is NULL
		store address of spectator is spectator variable
	else
		Send QUIT message to current spectator address
		replace address in spectator variable with address of new spectator
    Send GRID message to spectator with grid size
		Send GOLD message to spectator with current status of gold
		Send DISPLAY message to spectator with entire map

#### `playerJoin`:

	If numPlayers< maxPlayers
    call players_new to create new player
		store the new player it the allPlayers hashtable with its address
		Send GRID message to player with grid size
		Send GOLD message to player with 0 gold
		call display module to get a display string
		Send DISPLAY message to player with display string

#### `handleMessage`:
Handle all messages passed from the client to the server based on protocol in requirements spec.

	If message first word is PLAY
		Call playerJoin with player name
	If message first word is SPECTATE
		Call spectatorJoin
	If message first word is QUIT
		Call player_quit in player module
	if message is a keystroke character
		if the character is lowercase
			call player_move_regular from player module and pass the character
		if the character is uppercase
			call player_move_capital from player module and pass the character
		if numGoldLeft becomes zero after move
			Prepare summary of all gold by calling player_summary
			send message QUIT with summary to all clients
			end message loop and exit
		otherwise, if there is still gold left
			For each player in the hashtable
				send them updated GOLD message
				Calculate new display for them using display module
				send them the updated DISPLAY message
			if a spectator exists
				send them updated GOLD message
				send them updated DISPLAY message

#### `handleInput`:
		return false as we don't expect input from stdin so this should never be called

---

## player module

### Data structures

#### player_t
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

#### playerSwap
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

### Detailed pseudo code

#### `player_new`:

	if name is not null
		malloc space for a new player struct
		if malloced memory returns null
			return NULL
		else
			initialize the player struct
			set ID to a new ID based on numPlayers
			set name to given name
      set purse to int get_gold for that currCoordinate (likely zero)
      set currentCoordinate to a random coordinate
      initialize seenBefore to an empty set
      add currentCoordinate to seenBefore
      return that player struct

#### `player_updateCoordinate`:

    set player->currCoor to newCoor
    add newCoor key to the set seenBefore with null item
    if set_insert succeeds
      return true
    if set_insert fails
      return false

#### `player_moveRegular`:

	use switch case on character move provided
		based on case calulate new coordinate (left/right/up/down/diagonal)
	call grid_isOpen to check if new coordinate in game grid is open
	if the spot in that grid is open
	  call player_swap location to swap if another player exists
    if player_swap swaps with a player
      return true
		else
			call player_update_coordinate to update coordinate
      if player_updateCoordinate is a success
			  call player_collectGold to collect gold if there is any
        return true
      else
        return false

  
#### `player_moveCapital`:

	use switch case on character move provided
  		based on case calulate new coordinate (left/right/up/down/diagonal)
    call grid_isOpen to check if new coordinate in grid is open
	while the spot in that grid is open
	  call player_swap location to swap if another player exists
    if player_swap does not swap with a player
			call player_update_coordinate to update coordinate
      if player_updateCoordinate fails
        return false
      else
			  call player_collectGold to collect gold if there is any
        calulate new coordinate (left/right/up/down/diagonal)
	return true

#### `player_collectGold`:

	if counters_get on newCoor does not return 0
		increment player's purse by number of gold
		decrement numGoldLeft in Game by number of gold
		counters_set the newCoor to 0

#### `player_swapLocations`:

	takes a currPlayer and int newCoor where currPlayer is trying to move
	for each player in allPlayers hashtable
		if any player’s current location matches newCoor
      store player's currCoor in an int variable temp
			set player's currCoor to currPlayer's currCoor
      set currPlayer's currCoor to temp
      return true
	return false

#### `player_quit`:

	call hashtable_find to find player with given address
	if player cannot be found
    return false
	else
    call player_delete on the player
    set it to null
    return true

### `player_delete`:

	free real name of player
	call set_delete on seenBefore set
	free the player struct memory

## Testing plan

### unit testing

> How will you test each unit (module) before integrating them with a main program (client or server)?

### integration testing

> How will you test the complete main programs: the server, and for teams of 4, the client?

### system testing

> For teams of 4: How will you test your client and server together?

---

## Limitations

> Bulleted list of any limitations of your implementation.
> This section may not be relevant when you first write your Implementation Plan, but could be relevant after completing the implementation.
