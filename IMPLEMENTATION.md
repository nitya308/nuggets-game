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
* Grid module with visibility: Matthew

Each team member is responsible for testing and documentation of their module/program based on a consistent style defined by CS50 style guidelines.

## Client

### Data structures

We use a struct called playerAttributes which stores information about the player/spectator that the client program needs to reference as the game progresses.

typedef struct playerAttributes {
  char playerID;
  int purse;
  bool isPlayer;
  int numGoldLeft;
  int goldCollected;
  char* display;
} playerAttributes_t;


### Definition of function prototypes

A main function, a function to parse the command-line arguments, and a function to receive messages and handle messages.
The main function calls parseArgs and handles calling the functions of the message module.
```c
static int main(const int argc, char* argv[]);
```

This function validates the command-line arguments, printing to stderr if any errors are encountered, and sends a message to the server depending on whether the client is a player or spectator.
```c
static void parseArgs(const int argc, char* argv[]);
```

This function performs error checks, reads from stdin, and calls message_send with the appropriate messages.
```c
static bool handleInput(void* arg);
```

This function handles the bulk of the logic when receiving messages such as ‘QUIT’, ‘GOLD’, ‘OK’, ‘DISPLAY’,  ‘GRID’, and ‘ERROR’.

```c
static bool receiveMessage(void* arg, const addr_t from, const char* message);
```

This function uses the ncurses module to check that the user’s display is sufficiently large enough for the grid.
```c
static void checkDisplay(int nrow, int ncol);
```

### Detailed pseudo code

#### `main`:
	call parseArgs on argc and argv
	if message_init(NULL) is 0
	exit w/ non-zero int
	initialize address type for server
	call message_setAddr using argv[1] and argv[2] and address, check if returns false
		print to stderr and exit w/ non-zero status
	set bool variable to what calling message_loop returns
	call message_done()
	call endwin()
	return 0

#### `parseArgs`:
	if argc is < 3 or argc > 4
		print to stderr that there are an invalid number of args and exit
	if argv[1] or argv[2] is NULL
		print to stderr that the hostname or port is invalid and exit
	if argc is 4 and argv[3] is not NULL
		set playerAttributes➞isPlayer to true
		call message_send to server for `PLAY argv[3]`
	else
		set playerAttributes➞isPlayer to false
	call message_send to server for `SPECTATE`
	

#### `handleInput`
	set server address type to arg from param
	check if address pointer is NULL
		print to stderr
		return true
	if message_isAddr() returns false (address invalid)
		print to stderr
		return true
	get character c from stdin
	if c is EOF or EOT
		call message_send with quit message
	else
		call message_send with the inputted character
	return false

#### `receiveMessage`:
	if first word of message is QUIT 
		endwin()
	print explanation line
		return true
	if first word of message is GOLD
		scan message for n p r
		set playerAttributes➞goldCollected to n
		set playerAttributes➞purse to p
		set playerAttributes➞numGoldLeft to r
	if first word of message is GRID
		scan nrows and ncols from input
		call checkDisplay(nrows, ncols)
	if first word of message is OK
		set playerAttributes➞playerID to input
	if first word of message is DISPLAY
		set playerAttributes➞display to input
		call clear() from ncurses
		if playerAttributes➞isPlayer
			if playerAttributes➞goldCollected is 0
				print appropriate message if gold picked up
			else
				print appropriate message
		else
			print appropriate message for spectator
		print playerAttributes➞display
		call refresh() from ncurses
	if first word of message is ERROR
		print message to stderr
		clear()
		if playerAttributes➞isPlayer
			print appropriate message
		else
			print appropriate message
		print playerAttributes➞display
		call refresh() from ncurses
	else
		print to stderr that message has bad format
	return false

#### `checkDisplay`:
	call initscr()
	call cbreak()
	call noecho()
	initialize row and col variables
	set playerAttributes➞display to mem_malloc_assert(65507)
	call getmaxyx(stdscr, row, col) from ncurses
	while row < nrow + 1 or col < ncol + 1
		printw prompting user to increase window size and click enter
		call getch() to check for new line character
		call getmaxyx(stdscr, row, col)

---

## Server

### Data structures

#### `allPlayers`:
This is a hashtable (from libscs50 data structures) that stores all the players in the game. The key of the hashtable is a string representation of the player's address. The item is a player_t struct as defined in the player module.

#### `addresses`:
This is a hashtable (from libscs50 data structures) that stores the string representation of the player's address as the key and the actual address as the item.

#### `game`:
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

Do the initial setup for the game, pass seed if it exists, call function to build the grid, drop random Gold piles in random rooms, initialize the 'message' module, initialize the network and announce the port number and call message_loop(), to await clients
```c
static void initializeGame(char* argv);
```

Handles all messages from client by comparing first word of message to known types of messages and then calling the appropriate modules to handle the message. Deals with bad messages by printing error. 
```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```

Allows a new player to join if number of players is less than maxPlayers. Calls player module to create a new player and saves the player in the hashtable of all players.
```c
static void playerJoin(char* name, hashtable_t* allPlayers, hashtable_t* addresses, addr_t* client, grid_t* grid, int* numPlayers)
```

Allows a new spectator to join. If an existing spectator exists, kicks it out and replaces it with the new one.
```c
static void spectatorJoin(addr_t* address);
```

Function to free all memory and delete each player in the game along with both hashtables used by server and the gold counter.
```c
static void gameDelete();
```

Helper function to iterate over hashtable and send a quit message to each player
```c
static void sendQuit(void* arg, const char* addr, void* item);
```

### Detailed pseudo code

#### `main`:
  call parseArgs
  initialize the 'message' module
	initialize the network and announce the port number
	call message_loop(), to await clients
  exit with 0 code

#### `parseArgs`:
	validate commandline
	verify map file can be opened for reading
	if seed provided
		verify it is a valid seed number
		seed the random-number generator with that seed
	else
		seed the random-number generator with getpid()

#### `initializeGame`:
  allocate memory for game struct
  initialize the game struct
	call grid_read to store map in a 2D array of characters
	drop random Gold piles in random rooms
	

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
    increment numPlayers
		Send GRID message to player with grid size
		Send GOLD message to player with 0 gold
		call display module to get a display string
		Send DISPLAY message to player with display string

#### `handleMessage`:
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

#### `gameDelete`: 
	delete the allPlayers hashtable and all the players inside it
	delete the gold counter
	delete the grid
	free the spectator address
	free the game struct

#### `sendQuit`:
	create a message with "QUIT GAME 	OVER:\n summary"
	send the message to provided address

---

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
		all player_update_coordinate to update coordinate
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
        calculate new coordinate (left/right/up/down/diagonal)
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
			set player's currCoor to currPlayer's currCoorset currPlayer's currCoor to temp
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

### `player_summary`:
  create a summary string
  iterate over hashtable and add each player's summary
  return the summary

### `player_locations(hashtable_t* allPlayers)`:
  create a new set
  iterate over all players hashtable
    add each player's location and ID to set
  return the set
---

## grid module
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
```

### Pseudo code for logic/algorithmic flow

#### `grid_read`
	if file can be opened (assumes valid format for map)
		allocate memory for a grid_t*
			
		Read a line of the file. 
		initialize grid's num columns as length of the string returned
		Read lines until EOF, store num reads as integer height (add 1)
		initialize grid's num rows as this height

		Initialize empty 2-d array of chars dimensions width, height
		if NULL
			print to stderr
			return NULL
		while not end of file
			read file line into a row in the 2-d array.
			Increment current array row
	else
		print to stderr
		return null

#### `grid_locationConvert`
	if grid not null, location >=0, location < grid's num columns * num rows
		row number is  location/width
		column number is location%width
		return 2-element array of these two numbers
	else
		return NULL


#### `grid_isOpen`
	if coordinates given by locationConvert on location are null
		return false
	if the coordinates given point to a room or passage spot in 2-d grid char array
		return true
	else
		return false

#### `grid_isRoom`
	if coordinates given by locationConvert on location are null
		return false
	if the coordinates given point to a room spot in 2-d grid char array
		return true
	else
		return false

#### `grid_isVisible`
	if grid_isOpen on this location is true
		initialize set of location keys
		print the int key to a string literal
		store an "@" item for that location key 

		grid_locationConvert on the int to get observer row, column number
		for every row, col coordinate in grid
			if not isBlocked on that coordinate from observer location
				if location is less than radius away from observer
					print that location to string key
					if counters_get on gold counter for that location is >0
						insert gold symbol for this string key into set
					else if set_find on playerLocations set for that location is not NULL
						insert that player's symbol this string key into set
					else
						insert dummy symbol "g" for this location key into the set

		return the locations set
	else
		return null


#### `grid_isBlocked`
	if observer and point fall on same column
		iterate from observer's row + or - 1 to the point 
			if this location is not a room spot
				return true (meaning input point is blocked)
		return false (not blocked)

	if same row
		do above prodecure for rows
	
	define a small tolerance value
	calculate slope between observed point and observer using slope formula

	do the diagonal procedure:
	define a unit vector for column (just + or -1 based on whether observer columns < or > point's col)
	iterate from observer's col + unit vector to point's col
		calculate the row (floating point)
		if row is exactly on a grid point (within tolerance value)
			if point is not room spot
				return true (blockage)
		if both the int row below and row above aren't room spots
			return true
	
	repeat the diagonal procedure but define row unit vector, iterate through rows, and test the calculated columns (reverse roles of rows and columns)

	return false by default

#### `grid_updateView`
	If grid not null
		call grid_isVisible on the new location, with grid, players set, gold counter
		if resulting visible set is not null
			set_iterate through seen-before set, with visible set as arg, calling mergeHelper
			set_delete the seen-before set
			return the visible set
	return seen-before set

#### `mergeHelper`
	if set_find the string key (from seen-before) returns null for newly-visible
		insert the key into newly visible, with dummy “g” item

#### `grid_displaySpectator`
	if grid is not null,
		create an empty spectator's set of integer keys (locations) and character items
		convert integer location to string
		for each location in grid
			if grid_isOpen on location is true
				call set_find on player locations with location as key 
				if this returns non null
					insert the location as key, player symbol as item into spectator set
				if counters_get on gold counters for this location > 0
					insert the location as key, gold symbol "*" item into spectator set
				else
					insert location key, dummy item "g" into spectator set
			else
					insert location key, dummy item "g" into spectator set
		return spectator's set
	else
		return null		
			
 
#### `grid_Print`
	if grid and input locations set are not null
		Initialize empty printstring.
		for every int location in the grid
			print location int to a string key
			if set_find key on input set of locations gives null (means not visible)
				append a space “ “ to the printstring
			else if key corresponds to dummy item “g” (means visible, ordinary point)
				append the grid character from that location to the printstring
			else (means point contains special character like gold or player)
				Append the char stored in the set to the printstring.
			If int % grid width is 0
				add a newline character to the printstring
		return the printstring
	else
		return null

#### `grid_getNumberRows`
	if grid not null
		Gives number of rows in grid

#### `grid_getNumberCols`
	if grid not null	
		Gives number of columns in grid

#### `grid_delete`
	if grid not null
		loop through 2D char array in grid
			free each 1D string
		free the char array
		free the grid

---

## Testing plan

### unit testing

Each module will be tested by a unittest class before integrating it with main. 
For example, for player module, we will create a player, modify its coordinates, try to move it and delete it to make sure there are no memory leaks.

#### grid module
The grid module is tested with invalid filename (nonexistent file), for which it gives an error. 
Then, grid is tested on hole.txt
Grid reads from file, prints number of rows and columns in grid.
Display the specator's view of the grid (everything is visible)
Loop through all locations in grid, call grid_LocationConvert and grid_isOpen, which gives true for room, passage spots, false anywhere else.

Test grid_isOpen on negative locations, locations outside grid array bounds, for which it gives false.

Then, make set of player locations and symbols, counter of gold locations and amounts. Call grid_displaySpectator and grid_print, which gives the grid view but with gold and player symbols. By this test setup, gold and other players could appear in passages.


Loop through each of the players in the set, call grid_isVisible and grid_print, displaying each player's limited view with gold and other player symbols. 

Finally, loop through all locations in grid calling grid_updateView and grid_print on that location, which mimics a player moving through the grid and having their seen-before set constantly increased. By this test setup, the view does not increase as the player passes through not-open locations, but in valid locations, player sees other player and gold only in the visible portion of their seen-before set.

### integration testing

The complete main programs: the server and the client will be tested separately first with a number of invalid command line arguments. The server will be given a map to read a file. We will then create a test client to pass a variety of messages to the server and check that they are handled correctly. Each type of message, including error messages will be tested. 
For client, we will test that it can correctly recieve and print all types of messages by manually passing a variety of messages to it. Error messages and other edge cases like window size being too small will also be tested.

### system testing

Client and server will be tested together by running sample games. We will test a number of possible cases for example:
player stepping on another player
player attempting to move to a location that is not open for moving
player quitting in the middle of the game
new spectator joining when a previous one already exists
trying to have more than maxPlayers number of player join

---

## Limitations

> None forseen at this point in development.


## Extra credit
Grid implements a radius of visibility (defined as constant 5 ). Checks this when iterating through all points in the grid to determine if radius from them to player location is <= 5 (pythagorean theorem)
