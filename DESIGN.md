# CS50 Nuggets
## Design Spec
### add-drop, Winter 2022

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes player, grid, message modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Client

The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface

See the requirements spec for both the command-line and interactive UI.

```./client hostname port [playername]```

hostname is IP address where the server is running, port number is the port on which the server expects messages and the third (optional) argument determines whether to join as a *player* or *spectator*.

### Inputs and outputs

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
 
where *possible* means the adjacent gridpoint in the given direction is an empty spot, a pile of gold, or another player.

for each move key, the corresponding Capitalized character will move automatically and repeatedly in that direction, until it is no longer possible.
 
**OUTPUTS:**
 
The first line of the output is game status with player’s gold, unclaimed gold and any other information about the last message.
The rest of the output is a grid with all visible places (calculated based on line of sight). 
Every gridpoint is one of these characters:
   * ` ` solid rock - interstitial space outside rooms
   * `-` a horizontal boundary
   * `|` a vertical boundary
   * `+` a corner boundary
   * `.` an empty room spot
   * `#` an empty passage spot
 
**ERROR LOGS**

Any errors are logged to stderr with an informative message (such as if arguments are NULL, malloc out of memory errors)


### Functional decomposition into modules

* main: calls parseArgs and functions in message module
* parseArgs: parse and verify hostname and port and send message to server about whether client is player or spectator
* handleInput: performs error checks, reads from stdin, and calls message_send with the appropriate messages
* receiveMessage: called when message is recieved, performs action based on message
* checkDisplay: makes sure display is large enough for grid
 
### Pseudo code for logic/algorithmic flow

#### main
Calling parseArgs with the arguments provided. Initializes the message loop and calls functions in message module.

	call parseArgs
	initialize message module
	set address of server and store in in a variable
	call message loop to listen for messages
	call message done

#### parseArgs
parses arguments of hostname and port and stores them in the variables given by main. Sends a join as player or spectator message based on 3rd argument. If any errors are found in the arguments, prints an error to stderr and exits with non-zero integer.

	check if there are correct number of args
	parse and verify hostname and port from arguments
	if any bad arguments are found
		print an error to stderr and exits 0.
	if third argument (real name) is provided
		set isPlayer value in playerAttributes struct accordingly
		send message to server saying “PLAY real name”
	if no playerName provided
		set isPlayer value in playerAttributes struct accordingly
		send message to server saying “SPECTATE"

#### handleInput
Checks if server address is valid, reads from stdin, and calls message_send with the appropriate messages

	check if server address is valid
		if not, print error message
	get character from stdin
	check if character is EOF or EOT
		if so, call message_send with quit message
		if not, call message_send with the inputted character


#### receiveMessage
handles all messages recieved from the server. Takes in the messages as a string, and performs the appropriate action.

	check if first word of message is QUIT
		print explanation line
		return true
	check if first word of message is GOLD
		read message from server and set values in playerAttributes struct based on message
	if first word of message is GRID
		gets the number of rows and columns
	call checkDisplay with number of rows and cols
	check if first word of message is OK
		set playerID in playerAttributes struct
	check if first word of message is DISPLAY
		set display string in playerAttributes struct
		clear screen
		check if client is a player
			check if gold picked up, print appropriate message
			if not, print appropriate message
		if not, print appropriate message for spectator
		update display and refresh screen
	check if first word of message is ERROR
		print message to stderr
		clear screen
		check if client is a player
			print appropriate message
		if not, print appropriate message for spectator
		update display and refresh screen
	if first word of message is none of these
		print to stderr that message has bad format
	return false


#### checkDisplay
makes sure display screen is large enough for grid (nrow + 1 x ncol + 1) so game can be played properly

	initialize display properly
	while size of display is less than nrow + 1 x ncol + 1
		prompt user to increase window size and click enter


### Major data structures

#### playerAttributes
This struct stores:
player ID: a character letter (eg: 'A', 'B', 'C')
purse: integer number of coins it has
isPlayer: bool determining whether player or spectator
numGoldLeft: int of how much gold is remaining
goldCollected: int of how much gold was just picked up
display: stores display for player in a string

---

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user.

```./server map.txt [seed]```

where map.text is a valid map file and there is an optional seed which will be passed to srand.

### Inputs and outputs

The server takes in a map file as an input. A map file is a `.txt` file that contains a grid of ascii characters representing a map of walls, empty spots, passageways, and solid rock. The server also takes in an optional positive integer seed for the random-number generator.

It announces the port number in the terminal and sends messages back to the client.

Any errors are logged to our log file which we keep as stderr


### Functional decomposition into modules

* parseArgs: verifies all arguments and exits non-zero if error is found
* initializeGame: sets up the game
* spectatorJoin: allows a new spectator to join
* playerJoin: allows a new player to join the game
* handleMessages: carries out functions based on received message type


### Pseudo code for logic/algorithmic flow

The server will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	initialize the game
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
	when game ends, call message_done()
	clean up

#### parseArgs
verifies all arguments. If any errors are found in the arguments, prints an error to stderr and exits 0.

	verify that argument 1 mapfile can be opened for reading
	if argument 2 is provided
		if it is a positive integer
			store it in seed variable
	if any errors are found in the arguments, print an error to stderr and exit 0.

#### initializeGame
Do the initial setup for the game

	pass seed if it exists or call srand(getpid())
	call buildGrid to store map in a 2D array of characters
	drop random Gold piles in random rooms
	initialize the 'message' module
	initialize the network and announce the port number
	call message_loop(), to await clients

#### spectatorJoin
Allow a new spectator to join. If an existing spectator exists, kick it out and replace it with the new one.

	if spectator does not exist
		store address of spectator in spectator variable
	if spectator exists
		send QUIT message to current spectator
		replace address in spectator variable with address of new spectator
    send GRID message to spectator with grid size
		send GOLD message to spectator with current status of gold
		send DISPLAY message to spectator with entire map

#### playerJoin
Allow a new player to join if number of players is less than maxPlayers. Create a new player data structure for them, initialize it and save the player in the hashtable of all players.

	if number of player is less than maxPlayers
    create and initialize a new player struct
		store the new player it the allPlayers hashtable with its address
		send GRID message to player with grid size
		send GOLD message to player with 0 gold
		call grid module to get a display string
		send DISPLAY message to player with display string

#### handleMessages
Handle all messages passed from the client to the server based on protocol in requirements spec.

	if message first word is PLAY
		call playerJoin with player name
	if message first word is SPECTATE
		call spectatorJoin
	if message first word is QUIT
		call player_quit in player module
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

### Major data structures

#### player
This data structure stores the following information about each player
* player ID: a character letter (eg: 'A', 'B', 'C')
* real name: a string
* purse: integer number of coins it has
* current Coordinates: an integer representation of current position on grid
* seen Before: a set of inetger coordinates on grid it has already seen

#### allplayers
This will be a hashtable (from libscs50 data structures) that will store all the player in the game
The key of the hashtable will be a string representation of the player's address.
The item will be a player data type as defined above.

#### game
This holds the following information about the game:
* allplayers: a hashtable of all the player
* numGoldLeft: an integer amount of gold left in the game
* grid: a 2D array of characters the map in the game
* gold: a counter with keys as integer location and count as number of gold in that location
* spectator: stores an address of a spectator if it exists or else stores null
----------------

## grid module

Handles all functionality to do with the grid and calculates visibility

### Functional decomposition

1. grid_read: initializes the grid from file name
2. grid_isOpen: verifies whether given location is room/passage or not
3. grid_isRoom: verifies whether given location is room spot or not
4. grid_isVisible: gives set of visible locations from viewpoint
5. grid_updateView: gives visible set from new location + known set, with gold and players symbols in visible part only
6. grid_displaySpectator: gives set of all locations, with gold and players symbols as chars
7. grid_print: turns set of locations into formatted string 
8. grid_locationConvert: converts integer into 2-d coordinates
9. grid_getNumberRows: gives number of rows in grid
10. grid_getNumberCols: gives number of cols in grid
11. grid_delete: deletes the grid



### Pseudo code for logic/algorithmic flow

#### grid_read
	Reads from file 
	stores each char in a 2D array of characters
	stores the 2D array in Game data structure
 
#### grid_isOpen
	Takes int location input
	extracts grid from Game data structure
	finds the location in the grid
	return true if this is passage or room
	else return false

#### grid_isOpen
	Takes int location input
	extracts grid from Game data structure
	finds the location in the grid
	returns true is this location is room spot
	else return false
 
#### grid_isVisible
	Takes int location input, other player locations and gold locations
	initializes set of locations
	for every point in the grid
		if the point is not blocked by a wall/corner between it and player's vantage point
		if point is less than a defined radius away
			insert the location into set, with player or gold symbol if player or gold is in the location
	returns this set

#### grid_updateView
Creates a set of locations and characters to display at that location to represent other players and gold. (Takes set of player locations/symbols, counter of gold locations, new player location, player's seen before set)

	create a set of everything visible by calling	grid_isVisible
	for any location in player's seen before set 
	if location is not in visible set 
		append it to the visible set, but not the gold or player symbols
	deletes the seen before set
	returns the visible set
	

#### grid_displaySpectator
Returns whole grid as set, with symbols for players and gold stored as items. (Takes set of player locations/symbols, counter of gold locations)

	initialize empty spectator set
	for every location in grid
		if location is in player locations set
			insert the location key and player symbol into spectator set
		else if location is in counters of gold
			insert the location key and gold symbol into spectator set
		else
			insert location key and dummy item into spectator set
	return the set
 
#### grid_locationConvert
A function which takes an integer input, grid number of columns, grid number of rows
Turns it into a 2D coordinate on the grid through modulo division
 
#### grid_print
Takes grid and set of locations input
	
	Initialize empty printstring.
	For each location in the grid
		if location is not in the set of visible locations
			add a space “ “ to the printstring. 
		else if set stores gold or player symbol for that location
			add the gold/ player symbol to printstring
		else
			add the charcter from the grid array to printstring
		when the row ends
			add a newline character to the printstring
	return the printstring

#### grid_getNumberRows
Gives number of rows

#### grid_getNumberCols
Gives number of columns

#### grid_delete
Frees all associated memory

### Major data structures
Grid structure stores:
  2D array of chars, representing the grid
	number of columns
	number of rows


## player module

Handles all functionality to do with a player struct and modifies the player struct and Game variable based on movement of players

### Functional decomposition

1. player_new, which initializes a new player struct
2. player_updateCoordinate, which updates with new coordinate information
3. player_moveRegular, which calculates new coordinate based on direction of movement
4. player_moveCapital, which does the same as above except movement only stop until it is not possible anymore
5. player_collectGold, which increases gold in player’s purse if location contains gold
6. player_swapLocations, which swaps player locations if some player is in the newCoordinate 
7. player_quit, which deletes a player struct

### Pseudo code for logic/algorithmic flow

#### player_new

	create a new player struct 
	give it a new player ID
	store its real name
	set currentCoordinate to a random coordinate
	set purse to int get_gold for that currCoordinate (likely zero)
	initialize seenBefore to an empty set and add currentCoordinate
	return that player struct

#### player_updateCoordinate

	takes reference to a player datatype and new coordinate 
	updates the currCoordinate for player to newCoordinate
	update seenBefore to add new coordinate

#### player_moveRegular

	takes reference to a player datatype and the move character
	based on character(h, j, k, l) switches between left/right/up/down/diagonal
	calculates newcoordinate based on move direction
	if the spot in that grid is open
		if another player is in that location
			call player_swap location
		else
			call player_update_coordinate
			call player_collectgold
  
#### player_moveCapital

	takes reference to a player datatype and the move character
	based on character(h, j, k, l) switches between left/right/up/down/diagonal
	calculates newcoordinate based on move direction
	while the spot in that grid is open
		if another player is in that location
			call player_swap location
		else
			call player_update_coordinate
			call player_collectgold

#### player_collectGold

Takes in the new coordinates of the player and a reference to a player datatype and collects gold if gold exists

	extracts gold counter from the Game
	if player's new coordinate is found as a key in gold counter
		increment player's purse by number of gold in gold counter
		decrement numGoldLeft in Game by number of gold in gold counter
		set that gold counter count to 0.

#### player_swapLocations
Takes in pointer to a player struct and its integer new coordinate

	gets the hashtable of all player in the Game
	for each player in that hashtable
		if any player’s current location matches newCoordinate
			swaps the player locations.

#### player_quit
Takes the address of a player

	finds the player in the allPlayers hashtable using the address key
	Deletes and frees the player struct
	sets the item for that key in the hashtable to null

### Major data structures

It uses the player data structure defined above in server. It also uses the Game data structure from the server. 


---
## Extra credit
Grid implements a radius of visibility (defined as constant 5 )