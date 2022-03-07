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
* parseArgs: parse and verify number of args, hostname, and port
* handleInput: performs error checks, reads from stdin, and calls message_send with the appropriate messages
* receiveMessage: called when message is received, performs action based on message
* checkDisplay: makes sure display is large enough for grid
 
### Pseudo code for logic/algorithmic flow

#### main
Calling parseArgs with the arguments provided. Checks if client is player or spectator. Initializes the message loop and calls functions in message module.

	call parseArgs
	initialize message module
	set address of server and store in in a variable
	check if client is player or spectator
		send appropriate message to server
	listen for messages
	close message module

#### parseArgs
Parses arguments of hostname and port and stores them in the variables given by main. If any errors are found in the arguments, prints an error to stderr and exits with non-zero integer.

	check if there are correct number of args
	parse and verify hostname and port from arguments
	if any bad arguments are found
		print an error to stderr and exit

#### handleInput
Checks if server address is valid, reads from stdin, and calls message_send with the appropriate messages

	check if server address is valid
		if not, print error message
	get character from stdin
	check if character signifies end of file or quit
		if so, call message_send with quit message
	if not, call message_send with the inputted character if client is a player

#### receiveMessage
Handles all messages received from the server. Takes in the messages as a string, and performs the appropriate action.

	check if first word of message is QUIT
		print explanation line
		return true
	check if first word of message is GOLD
		read message from server and set values in playerAttributes struct based on message
	check if first word of message is GRID
		get the number of rows and columns to call checkDisplay with
	check if first word of message is OK
		set playerID in playerAttributes struct
	check if first word of message is DISPLAY
		set display string in playerAttributes struct
		clear screen
		check if client is a player
			check if no gold picked up, print appropriate message
			if not, print appropriate message
		if not, print appropriate message for spectator
		update display and refresh screen
	check if first word of message is ERROR
		print message to stderr
		clear screen
		check if client is a player
			print appropriate message
		print display and refresh screen
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
* handleMessage: carries out functions based on received message type
* isEmpty: checks if an input string is completely empty
* isReadable: checks if a file path is readable
* buildGrid: builds the grid for the given map
* endGame: ends the game, sending messages to all relevant clients, freeing memory
* deletePlayer: deletes the player struct
* itemDelete: deletes the item in the set
* sendDisplayMessage: creates a DISPLAY message and sends it to relevant clients
* sendGoldMessage: creates a GOLD message and sends it to relevant clients
* sendEndMessage: creates a QUIT message with player summary and sends it to relevant clients
* updateSpectatorDisplay: updates the spectator’s display by sending DISPLAY message to spectator
* initializeGoldPiles: initializes random piles of random number of gold and place them on the grid when starting the game
* generateRandomLocations: creates an array of random locations to put the random piles of gold
* generateGoldDistribution: creates an array of random count of gold for each pile
* handleInput: used to handle input from stdin on server side (do nothing)


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
checks the arguments given by the caller, ensuring that maps.txt is readable and setting the random seed number if [seed] is provided. Otherwise, generate a random seed using process id..

	if valid number of argos provided,
		validate seed is positive integer
		Or generate a seed value by using the pid.
	else 
		Return error code

#### handleInput
	Adapted from the message module. See message.c.


#### handleMessages
Handle all messages passed from the client to the server based on protocol in requirements spec.

	if message first word is PLAY
		call playerJoin with player name and send GOLD and DISPLAY message
		Update spectator’s display
	if message first word is SPECTATE
		call spectatorJoin

	If message starts with KEY
		If lower character after KEY
			Move the player by 1 step if valid move and collect gold if available
			If no more gold left, end the game
		If uppercase character after KEY
			If character is Q
				Quit the player/spectator respectively
				Send messages to all clients if necessary
			If not Q,
				Move the player to end of room
				If not more gold left, end game.
				Else update DISPLAY and GOLD accordingly

#### isReadable
Check if a file is readable
	
	If file is readable, return true.
	Else return false.

#### playerJoin
Allow a new player to join if number of players is less than maxPlayers. Create a new player data structure for them, initialize it and save the player in the hashtable of all players.

	if number of player is less than maxPlayers
    create and initialize a new player struct
		If number of gold left is 0, end the game
		store the new player it the allPlayers hashtable with its address
		Send OK message to player
		send GRID message to player with grid size
		send GOLD message to player with 0 gold
		call grid module to get a display string
		send DISPLAY message to player with display string
		Update player’s seen before locations
		Store player’s address
	else, send QUIT message to new client.


#### isEmpty
Checks if a string is filled with empty spaces

 	loops through the input string, and check if isspace
		if character isspace,
			return false
	return true

#### spectatorJoin
Allow a new spectator to join. If an existing spectator exists, kick it out and replace it with the new one.

	if spectator exists
		send QUIT message to current spectator
	else initialize spectator’s address id to MaxPlayers
	replace address in spectator variable with address of new spectator
    	send GRID message to spectator with grid size
	send GOLD message to spectator with current status of gold
	send DISPLAY message to spectator with entire map

#### buildGrid
Builds the grid based on the input map.txt file and store into the game struct

	call grid_read from the grid module on the map filename given to server and store in game->grid

#### endGame
Ends the game, sending QUIT messages and player summaries to clients accordingly, freeing all the memory used in the game

 	send GOLD messsage to all players
  	send DISPLAY message to all players
  	updateSpectatorDisplay
  	call player_summary and send end message to all players with the summary
  	if spectator is connected
		send quit message to spectator
  	free all unused memory, deleting allPlayers, addrID, gold, grid, addresses, numGoldLeft and game.

#### deletePlayer
Used in hashtable_delete, delete the player struct stored in allPlayers

	if player is not yet deleted,
		call player_delete

#### itemDelete
Used in set_delete and hashtable_delete, freeing memory accordingly
	deletes the item by calling mem_free

#### sendDisplayMessage
Generates the DISPLAY message and send to clients accordingly

 	find the player's address id
  	if player is still playing, and player is not null, and address id exists in addrID,
		get players newSeenBefore using grid_updateView
		call grid_print to get the string of the grid that the player can see
		create DISPLAY message
		send DISPLAY message using message_send
		free the messages

#### sendGoldMessage:
Generate the GOLD message and send the clients respectively

  	find the player's address id
  	if player is still playing, and player is not null, and address id exists in addrID,
		create GOLD message
		send GOLD message using message_send

#### sendEndMessage:
Creates the end game message and sends it to clients accordingly

	create the QUIT GAME OVER message
	if the player exists and is still connected to server
		get the player's addr_t
		call message_send to player, sending the player the end of game message

#### updateSpectatorDisplay:
Creates a DISPLAY message for spectator and sends it to connected spectator
  	if spectator is connected
		create a gold message
		create a display message
		send gold and display message to the spectator
  	free all unused memory


#### initializeGame
Do the initial setup for the game

	create game struct
 	call buildGrid to create grid_t by loading the map file
  	set numGoldLeft
  	create the allPlayers hashtable
  	create the addrID hashtable that stores the ID to the addresses for each client connected
  	create the counters_t for gold that stores (key, count), where key is the location on the grid and count is the number of gold at that location
  	call initializeGoldPiles to create random gold piles in the map
  	allocate memory for addresses that stores an array of all the addr_t of clients
  	set spectatorAddressID and numPlayers to 0

#### initializeGoldPiles:
Generate a random number of gold piles each of random number of gold, totaling goldTotal.

  	calculate the maximum number of available spots on the grid
  	compare the maxAvailableSpots with GoldMaxNumPiles and take the smaller number
  	generate a random number of gold piles between GoldMinNumPiles and the smaller number calculated earlier
  	create an array with size number of gold piles storing the locations to put the gold piles
  	create an array with size number of gold piles storing the random number of gold in each pile summing up to GoldTotal
  	loop through number of gold piles generated, setting the location and the gold count in game->gold


#### generateRandomLocations:
Generate an array storing random locations in the grid

  	loop through the number of gold piles
		generate a random location
		if location is a valid spot on the grid to put the gold
			if the location is not occupied by gold
				store location

#### generateGoldDistribution:
Generate an array of random number of gold, totaling goldTotal.

  	Calculate goldRemaining, the max value of gold that can be generated such that each gold pile has at least 1 gold in it.
  	loop through the number of gold piles - 1,
		generate a random gold amount
		store the gold amount in the array
		update number of goldRemaining
  	allocate the remaining gold unallocated to the last gold pile


### Major data structures

#### player
This data structure stores the following information about each player
* player ID: a character letter (eg: 'A', 'B', 'C')
* name: a string
* purse: integer number of coins it has
* recentGoldCollected: integer number of gold player had collected in most recent move
* current Coordinates: an integer representation of current position on grid
* seen Before: a set of integer coordinates on grid it has already seen

#### allplayers
This will be a hashtable (from libcs50 data structures) that will store all the player in the game
The key of the hashtable will be a string representation of the player's address.
The item will be a player data type as defined above.

#### grid
Refer to grid module below for more information

#### gold
This is a counters struct (from libcs50 data structures) that will store the location of gold piles as keys and count as the number of gold in that location.

#### addresses
This is an array storing addr struct. It stores all the addresses of clients that have connected with the server.

#### game
This holds the following information about the game:
* allplayers: a hashtable of all the player
* addrID: a hashtable mapping an address to an index in the addresses array
* addresses: an array storing clients’ addresses. Spectator’s address is always in the last index of the array
* numGoldLeft: an integer amount of gold left in the game
* numPlayers: an integer of the number of players that have joined 
* grid: a grid struct (refer to grid module)
* gold: a counters struct with keys as integer location and count as number of gold in that location
* spectatorAddressID: an integer index that is used in addresses array to get the address of the spectator
* port: the port that the server is running on
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

#### grid_isRoom
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

	create a set of everything visible by calling grid_isVisible
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
      		add the character from the grid array to printstring
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


## Testing Plan

### Unit Testing

### player module
All functions are tested individually. We first create a small test grid, and then add two players to it with known values of gold. Player_print is used for debugging purposes. 
We test moveRegular and moveCapital to see if it can move correctly based on the character provided. We also have it step on another player to make sure they are swapped. 
Player_locations and player_summary are tested by creating locations and summaries of known player so we can verify whether they are correct.
Finally, we delete the players to test player_delete.
We also run valgrind on the test file to make sure there are no memory leaks.

#### grid module
The grid module is tested with invalid filename (nonexistent file), for which it gives an error. 
Then, grid is tested on hole.txt
Grid reads from file, prints number of rows and columns in grid.
Display the spectator's view of the grid (everything is visible)
Loop through all locations in grid, call grid_LocationConvert grid_isOpen, and grid_isRoom, which states whether spots are open, and if so, whether room spots or not.

Test grid_isOpen on negative locations, locations outside grid array bounds, for which it gives false.

Then, make set of player locations and symbols, counter of gold locations and amounts. Call grid_displaySpectator and grid_print, which gives the grid view but with gold and player symbols. By this test setup, gold and other players could appear in passages.

Loop through each of the players in the set, call grid_isVisible and grid_print, displaying each player's limited view with gold and other player symbols. 

Finally, loop through all locations in grid calling grid_updateView and grid_print on that location, which mimics a player moving through the grid and having their seen-before set constantly increased. By this test setup, the view does not increase as the player passes through not-open locations, but in valid locations, player sees other player and gold only in the visible portion of their seen-before set.

### Integration Testing
The main programs, server and client, will be tested separately using miniclient and miniserver provided in the starter kit.We run valgrind on both to make sure there are no memory leaks or errors.

The miniclient sends the following messages to the server:

PLAY name
PLAY (without a name)
SPECTATE
KEY valid
KEY invalid
QUIT
Any type of invalid keys

Other edge cases we will check include:

player stepping on another player
player attempting to move to a location that is not open for moving
new spectator joining when a previous one already exists
trying to have more than maxPlayers number of player join


The client sends messages to and receives messages from the sample server provided in the `cs50-dev/shared/linux` directory using the following test cases:

1. One player joining the game and pressing all possible keystrokes to see if they are able to move correctly and if the game and player’s gold status is displaying correctly at the top and updating appropriately as the player picks up gold. Invalid keystrokes should be tested as well to ensure no movements are displayed and the client is told they made an invalid move.
2. Another player joining the same game and so on and so forth until the 27th player, in which the client should receive a message about being unable to play.
3. Playing a game (with both one and multiple players) until all gold is collected and ensuring the game over message prints the scores correctly in the terminal.
4. Joining a game as a player and quitting before making any moves to ensure the quit message displays correctly in the terminal.
5.  Joining a game as a player and quitting after making moves to ensure the quit message displays correctly in the terminal.
6. Joining as a spectator while another spectator is currently in the game to ensure the other spectator is kicked out and given the appropriate explanation while the new spectator is allowed to join.
7. Joining as a spectator while players are already in the game so ensure all player’s icons and movements are visible and being continually updated. The keystrokes players usually use to move as well as other invalid keystrokes should be tested, as well, to ensure the server ignores any spectator keystrokes that do not signify quitting. Check to see if game’s gold is being displayed and updating correctly.
8. Quitting as a spectator to ensure the appropriate quit message displays correctly in the terminal.
9. Test client with an invalid number of arguments (no arguments, one argument, and more than 4 arguments)
10. Test client with a large map file and ensure the user is prompted to expand their window to the appropriate size before the map file is displayed.
11. Test that client handles any bad messages from server, out of memory errors, message module errors, or errors parsing the arguments by printing to a log file.
12. Run valgrind to ensure there are no memory leaks for any of these test cases.


### System Testing
Client and server now run together with all the cases described above.

---
## Extra credit
Grid implements a radius of visibility (defined as constant at the top of the file). It can be modified to any value and will limit the visibility range to that value. It is currently set = 1000. Change the value to a smaller range to see functionality.

Player implements: Player who quits before the end of the game gives up their gold, leaving a new pile at their last location. This is done in the plyer_quit function which updates the game’s gold counter and variable tracking the number of gold left.



