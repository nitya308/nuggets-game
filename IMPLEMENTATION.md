x# CS50 Nuggets
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

A main function, a function to parse the command-line arguments, and a function to receive messages and handle messages. The main function calls parseArgs and handles calling the functions of the message module. It also sends a message to the server depending on whether the client is a player or spectator.
```c
int main(const int argc, char* argv[]);
```

This function validates the command-line arguments, printing to stderr if any errors are encountered
```c
static void parseArgs(const int argc, char* argv[]);
```

This function performs error checks, reads from stdin, and calls message_send with the appropriate messages.
```c
static bool handleInput(void* arg);
```

This function handles the bulk of the logic when receiving messages such as ‘QUIT’, ‘GOLD’, ‘OK’, ‘DISPLAY’,  ‘GRID’, and ‘ERROR’, or any other type of message.
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
	if playerName was received in command-line
		set playerAttributes.isPlayer to true
		send play message to server
	else
		set playerAttributes.isPlayer to false
		send spectate message to server
	set bool variable to what calling message_loop returns
	call message_done()
	call endwin()
free display
	return 0

#### `parseArgs`:
	if argc is not 3 or 4
		print to stderr that there are an invalid number of args and exit
	if argv[1] or argv[2] is NULL
		print to stderr that the hostname or port is invalid and exit

#### `handleInput`
	set server address type to arg from param
	check if address pointer is NULL
		print to stderr
		return true
	if message_isAddr() returns false (address invalid)
		print to stderr
		return true
	get character c from stdin
	if c is Q or EOF
		call message_send with KEY Q
	else
		if client is player
			call message_send with the inputted character
	return false

#### `receiveMessage`:
	if first word of message is QUIT 
		endwin()
		print appropriate quit message
		return true
	else if first word of message is GOLD
		scan message for n p r
		set playerAttributes.goldCollected to n
		set playerAttributes.purse to p
		set playerAttributes.numGoldLeft to r
	else if first word of message is GRID
		scan nrows and ncols from input
		call checkDisplay(nrows, ncols)
	else if first word of message is OK
		set playerAttributes.playerID to input
	else if first word of message is DISPLAY
		set to formatted input to new string variable
		if playerAttributes.display isn’t NULL
			strcpy the string variable into display
		call clear() from ncurses
		if playerAttributes.isPlayer
			if playerAttributes.goldCollected is 0
				print appropriate message
			else
				print appropriate message w/ gold picked up
		else
			print appropriate message for spectator
		print display
		call refresh() from ncurses
	else if first word of message is ERROR
		print message to stderr
		clear()
		if playerAttributes.isPlayer
			print appropriate message about invalid move
		print playerAttributes.display
		call refresh() from ncurses
	else
		print to stderr that message has bad format
	return false

#### `checkDisplay`:
	call initscr()
	call cbreak()
	call noecho()
	initialize row and col variables
	malloc memory for display the size of the max message length
	call getmaxyx(stdscr, row, col) from ncurses
	while row < nrow + 1 or col < ncol + 1
		printw prompting user to increase window size and click enter
		while getch() doesn’t return a new line character
			continue
		call getmaxyx(stdscr, row, col)

---

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


### Detailed pseudo code

#### `main`:
	call parseArgs
	if parseArgs returns exit code 1, exit server
	else, initialize the game by calling initializeGame.
	initialize the 'message' module
	initialize the network and announce the port number
	call message_loop(), to await clients
	exit with 0 code

#### `parseArgs`:
	if 2 or 3 arguments provided, including the command itself,
		if 3 arguments,
			return error code 1 if value is not a positive integer
			srand(value) if it is a positive integer
		else (if 2 arguments),
			srand(getPid())
		check if 2nd argument given is a readable file, returning error code if not readable
	else
		print to stderr and return error code

### `handleInput`:
	Adapted from message module. See message.c.

#### `handleMessage`:
	if client sends PLAY:
			call playerJoin to join the player
			send GOLD message to all clients connected
			send DISPLAY message to all clients connected
			update spectator's display
	else if client sends SPECTATE
			call spectatorJoin, initializing the spectator
	else if message starts with "KEY "
			find the player in game->allPlayers
			if character is lower character,
				call player_moverRegular
				if player_moveRegular returns true, it is a valid move and player moves and collects gold accordingly
							if game->numGoldLeft is 0, no more gold in game, end he game and send QUIT message to all clients
							update all clients' GOLD
							update all clients' DISPLAY
							updateSpectatorDisplay
				else, it is an invalid move and server sends message to lient informing them that it is invalid
			if character is uppercase,
				if character is Q,
						if it is a player
								call player_quit
						if it is a spectator
								set spectatorAddressID to 0, quitting the spectator and marking that no spectator is connected
						send QUIT message to spectator/player
						sendGoldMessage to all clients
						sendDisplayMessage to all clients
						updateSpectatorDisplay

				else
						call player_moveCapital
						if player_moveCapital returns true, it is a valid move and player moves and collects gold accordingly
							if game->numGoldLeft is 0, no more gold in game, end the game and send QUIT message to all clients
								update all clients' GOLD
								update all clients' DISPLAY
								updateSpectatorDisplay
						else, it is an invalid move and server sends message to client informing them that it is invalid

#### `isReadable`:
	check if path is readable
	return true if file able to open to read
	return false if file cannot open to read

#### `playerJoin`:
	if numPlayers < MaxPlayers:
		create a new player using player_new
		if game->numGoldLeft is 0:
			call endGame
		create the OK message
		create the GRID message
		store the new player's addr_t in game->addresses
		store the index of that player's address in game->addresses in game->addrID
		store the new player in game->allPlayers
		update the player's seenBefore
		send OK and GRID message
		increment game->numPlayers
	else
		send QUIT message to new client trying to connect

#### `isEmpty`:
	loops through the input string, and check if isspace
		if character isspace,
			return false
	return true

#### `spectatorJoin`:
	if spectator exists
		send QUIT message to the existing spectator
	else
		initialize spectatorAddressID to MaxPlayers, the special index stored for spectator addresses in game->addresses
	store new spectator address in game->addresses
	create GRID message
	create GOLD message
	create DISPLAY message
	send them to spectator using message_send
	free all unused memory

#### `buildGrid`:
	call grid_read from the grid module on the map filename given to server and store in game->grid

#### `endGame`:
	send GOLD messsage to all players
	send DISPLAY message to all players
	updateSpectatorDisplay
	call player_summary and send end message to all players with the summary
	if spectator is connected
		send quit message to spectator
	free all unused memory, deleting allPlayers, addrID, gold, grid, addresses, numGoldLeft and game.

#### `deletePlayer`:
	if player is not yet deleted,
		call player_delete

#### `itemDelete`:
	deletes the item by calling mem_free

#### `sendDisplayMessage`:
	find the player's address id
	if player is still playing, and player is not null, and address id exists in game->addrID,
		get players newSeenBefore using grid_updateView
		call grid_print to get the string of the grid that the player can see
		create DISPLAY message
		send DISPLAY message using message_send
		free the messages

#### `sendGoldMessage`:
	find the player's address id
	if player is still playing, and player is not null, and address id exists in game->addrID,
		create GOLD message
		send GOLD message using message_send

#### `sendEndMessage`:
	create the QUIT GAME OVER message
	if the player exists and is still connected to server
		get the player's addr_t
		call message_send to player, sending the player the end of game message

#### `updateSpectatorDisplay`:
	if spectator is connected
		create a gold message
		create a display message
		send gold and display message to the spectator
	free all unused memory

#### `initializeGame`:
	allocate memory to game and check if successful
	call buildGrid to create grid_t by loading the map file
	set numGoldLeft
	create the allPlayers hashtable
	create the addrID hashtable that stores the ID to the addresses for each client connected
	create the counters_t for gold that stores (key, count), where key is the location on the grid and count is the number of gold at that locaton
	call initializeGoldPiles to create random gold piles in the map
	allocate memory for addresses that stores an array of all the addr_t of clients
	set spectatorAddressID and numPlayers to 0

#### `initializeGoldPiles`:
	calculate the maximum number of available spots on the grid
	compare the maxAvailableSpots with GoldMaxNumPiles and take the smaller number
	generate a random number of gold piles between GoldMinNumPiles and the smaller number calculated earlier
	create an array with size number of gold piles storing the locations to put the gold piles
	create an array with size number of gold piles storing the random number of gold in each pile summing up to GoldTotal
	loop through number of gold piles generated, setting the location and the gold count in game->gold

#### `generateRandomLocations`:
	loop through the number of gold piles
		generate a random location
		if location is a valid spot on the grid to put the gold
			if the location is not occupied by gold
				store location

#### `generateGoldDistribution`:
	Calculate goldRemaining, the max value of gold that can be generated such that each gold pile has at least 1 gold in it.
	loop through the number of gold piles - 1,
		generate a random gold amount
		store the gold amount in the array
		update number of goldRemaining
	allocate the remaining gold unallocated to the last gold pile


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
	if the coordinates given point to a room character in 2-d grid char array
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
		do above procedure for rows

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

We test each module, player and grid, rigorously. Within each module, we test the functions individually. Each module will be tested by a unittest program, titled `playertest.c` and `gridtest.c`, respectively, before integrating it with main. We also test run valgrind on the test file to ensure that there are no memory leaks. See [DESIGN](DESIGN.md) for more details.

### integration testing

For server, we test it with the miniclient in the support directory. Using that, we check if our server can correctly receive messages from the client and send properly formatted messages back to the client. We test error and edge cases to ensure that the program handles all cases gracefully.
For client, we test it with the shared server in the shared CS50 directory. Using that, we check if it can correctly receive and print all types of messages by manually passing a variety of messages to it. Error messages and other edge cases like window size being too small will also be tested.
For more details, please see the design spec.

### system testing

Client and server will be tested together by running sample games. We will re-run all the cases described in the design spec individually for server and client now together on both as a system. Some test cases are detailed below, but please refer to the design specification for more explanation.

**On client joining as a player:**
Moving in all directions (upper and lowercase)
Moving in a direction where moving is not allowed
Pressing an invalid keystroke
Stepping on another player (should swap)
More than maxPlayers trying to join
Quitting
Collecting all the gold and ending game
Prints summary message when game is over

**On client joining as spectator:**
Trying to move (the system should not allow spectator to try moving)
Updating each time a player joins
Updating each time a player moves
Updating each time a player quits
Showing appropriate end message when game ends
Having a new spectator join when one already exists
Spectator quitting

---

## Limitations

> None forseen.


## Extra credit
Grid implements a radius of visibility (defined as constant 1000 in grid.c ). Change to something like value =5 to see functionality. Checks this when iterating through all points in the grid to determine if radius from them to player location is <= 5 (pythagorean theorem)

Player implements: Player who quits before the end of the game gives up 
their gold, leaving a new pile at their last location. This is done in the plyer_quit function which updates the game’s gold counter and variable tracking the number of gold left.


