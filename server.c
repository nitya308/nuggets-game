#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "grid/grid.h"
#include "libcs50/counters.h"
#include "libcs50/hashtable.h"
#include "libcs50/mem.h"
#include "player/player.h"
#include "support/log.h"
#include "support/message.h"

/**
 * server - handles all game logic and message sent from all the clients for the gold nuggets game
 *
 * usage: ./server map.txt [seed]
 *   where map.txt is the path to a map file for the game
 *   where seed is the random seed number
 *
 * Assumption: The map.txt file is a valid map file (see https://github.com/cs50winter2022/nuggets-info/blob/main/REQUIREMENTS.md#valid-maps)
 */

/* *********************************************************************** */
/* Private function prototypes */
static int parseArgs(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool isReadable(char* pathName);
static void playerJoin(char* name, const addr_t client);
static void spectatorJoin(const addr_t* address);
static void buildGrid(grid_t* grid, char** argv);
static void endGame();
static void deletePlayer(void* item);
// static void itemcount(void* arg, const char* key, void* item);
static void itemDelete(void* item);
static void sendDisplayMessage(void* arg, const char* addr, void* item);
static void sendGoldMessage(void* arg, const char* addr, void* item);
static void sendEndMessage(void* arg, const char* addr, void* item);
static void updateSpectatorDisplay();
static void initializeGame(char** argv);
static void initializeGoldPiles();
static void generateRandomLocations(int numGoldPiles, int* arr);
static void generateGoldDistribution(int numGoldPiles, int* arr);
static void itemprint(FILE* fp, const char* key, void* item);
static void playeritemprint(FILE* fp, const char* key, void* item);
static void setitemprint(FILE* fp, const char* key, void* item);

/**************** global types ****************/
typedef struct game {
  hashtable_t* allPlayers;
  hashtable_t* addrID;
  addr_t* addresses;  // store all player addresses and an additional slot for spectatorAddress (last slot in array)
  int* numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  int spectatorAddressID;  // val=0 if no spectator joined, val=MaxPlayers if a spectator joined
  int port;
} game_t;

/**************** local variables ****************/
static game_t* game;                    // game struct storing the state of the game
static const int MaxPlayers = 26;       // maximum number of players
static const int GoldTotal = 250;       // amount of gold in the game
static const int GoldMinNumPiles = 10;  // minimum number of gold piles
static const int GoldMaxNumPiles = 30;  // maximum number of gold piles

/* ***************** main ********************** */
int main(const int argc, char* argv[])
{
  // log_init(stderr);
  int exitStatus = parseArgs(argc, argv);
  if (exitStatus == 1) {
    exit(1);
  }
  initializeGame(argv);  // initialize the game with the map

  // initialize the message module and play the game
  int port;
  if ((port = message_init(stderr)) == 0) {
    fprintf(stderr, "Failed to initialize message module.\n");
    exit(2);  // failure to initialize message module
  }
  printf("Ready to play, waiting at port %d", port);
  game->port = port;

  // Loop, waiting for input or for messages; provide callback functions.
  bool ok = message_loop(&port, 0, NULL, handleInput, handleMessage);

  // shut down the message module
  message_done();

  if (ok) {
    exit(0);  // successfully ran program
  }
  else {
    exit(1);  // failed to run program, terminate with error code
  }
}

/* ***************** initializeGame ********************** */
/*
 * Initializes the game, allocating memory for the game struct and initialize the variables in it
 *
 * Pseudocode:
 *   allocate memory to game and check if successful
 *   call buildGrid to create grid_t by loading the map file
 *   set numGoldLeft
 *   create the allPlayers hashtable
 *   create the addrID hashtable that stores the ID to the addresses for each client connected
 *   create the counters_t for gold that stores (key, count), where key is the location on the grid and count is the number of gold at that locaton
 *   call initializeGoldPiles to create random gold piles in the map
 *   allocate memory for addresses that stores an array of all the addr_t of clients
 *   set spectatorAddressID and numPlayers to 0
 */
static void
initializeGame(char** argv)
{
  game = mem_malloc(sizeof(game_t));
  if (game == NULL) {
    fprintf(stderr, "Failed to create game. Exiting...\n");
    exit(1);
  }
  buildGrid(game->grid, argv);
  game->numGoldLeft = mem_malloc(sizeof(int*));
  *(game->numGoldLeft) = GoldTotal;
  game->allPlayers = hashtable_new(MaxPlayers);
  if (game->allPlayers == NULL) {
    endGame();      // end the game and free all memory
    fprintf(stderr, "Failed to create allPlayers hashtable. Exiting...\n");
    exit(1);
  }
  game->addrID = hashtable_new(MaxPlayers);
  if (game->addrID == NULL) {
    endGame();      // end the game and free all memory
    fprintf(stderr, "Failed to create addresses hashtable. Exiting...\n");
    exit(1);
  }
  game->gold = counters_new();
  if (game->gold == NULL) {
    endGame();      // end the game and free all memory
    fprintf(stderr, "Failed to create gold counters. Exiting...\n");
    exit(1);
  }
  initializeGoldPiles();      // initialize gold piles by generate random number of gold piles and random number of gold on the grid
  game->addresses = mem_malloc((MaxPlayers + 1) * sizeof(addr_t));
  game->spectatorAddressID = 0;  // no spectator initially. set value MaxPlayers if spectator connected
  game->numPlayers = 0;
}

/* ***************** buildGrid ********************** */
/*
 * Loads the map.txt given by the caller into a grid_t and stores it
 *
 * Pseudocode:
 *   call grid_read from the grid module on the map filename given to server and store in game->grid
 */
static void
buildGrid(grid_t* grid, char** argv)
{
  char* filename = argv[1];
  game->grid = grid_read(filename);  // build the grid
}

/* ***************** initializeGoldPiles ********************** */
/*  
 * Generates a random number of gold piles and a random number of gold in each pile for the game
 *
 * Pseudocode:
 *  calculate the maximum number of available spots on the grid
 *  compare the maxAvailableSpots with GoldMaxNumPiles and take the smaller number
 *  generate a random number of gold piles between GoldMinNumPiles and the smaller number calculated earlier
 *  create an array with size number of gold piles storing the locations to put the gold piles
 *  create an array with size number of gold piles storing the random number of gold in each pile summing up to GoldTotal
 *  loop through number of gold piles generated, setting the location and the gold count in game->gold
 *   
 */
static void
initializeGoldPiles()
{
  int cols = grid_getNumberCols(game->grid);
  int rows = grid_getNumberRows(game->grid);
  int maxAvailableSpots = (rows * cols) - (rows * 2) - (cols * 2);    // generate number of open spots to put gold
  int max = (GoldMaxNumPiles > maxAvailableSpots) ? maxAvailableSpots : GoldMaxNumPiles;  // get the smaller number of piles
  int numGoldPiles = (rand() % (max - GoldMinNumPiles + 1)) + GoldMinNumPiles;  // generate a value between min and max range of gold piles
  int goldDistributionArray[numGoldPiles];
  int randomLocations[numGoldPiles];
  generateRandomLocations(numGoldPiles, randomLocations);         // generate an array of random valid locations on the grid
  generateGoldDistribution(numGoldPiles, goldDistributionArray);  // generate an array of random gold amount, summing up to goldTotal
  int idx = 0;
  while (idx < numGoldPiles) {  // put the randomly generated gold piles down
    counters_set(game->gold, randomLocations[idx], goldDistributionArray[idx]);
    idx++;
  }
}

/* ***************** generateRandomLocations ********************** */
/*
 * Generates an array of random locations on the grid to put the gold piles
 *
 * Pseudocode:
 *   loop through the number of gold piles
 *      generate a random location
 *      if location is a valid spot on the grid to put the gold
 *        if the location is not occupied by gold
 *            store location
 */
static void
generateRandomLocations(int numGoldPiles, int* arr)
{
  int nRows = grid_getNumberRows(game->grid);
  int nCols = grid_getNumberCols(game->grid);
  int i = 0;
  while (i < numGoldPiles) {
    int location = rand() % (nRows * nCols);          // get the index in the map
    if (grid_isOpen(game->grid, location)) {          // if it is an available space
      if (counters_get(game->gold, location) != 0) {  // if it is an existing gold pile
        continue;                                     // do not store as valid location
      }
      else {  // if location not occupied by gold
        arr[i] = location;
        i++;
        counters_set(game->gold, location, 1);        // mark location for having gold piles
      }
    }
  }
}

/* ***************** generateGoldDistribution ********************** */
/*
 * Generates an array of random number of gold for each gold pile, summing up to GoldTotal
 *
 * Pseudocode:
 *   Calculate goldRemaining, the max value of gold that can be generated such that each gold pile has at least 1 gold in it.
 *   loop through the number of gold piles - 1,
 *      generate a random gold amount
 *      store the gold amount in the array
 *      update number of goldRemaining
 *   allocate the remaining gold unallocated to the last gold pile
 */
static void
generateGoldDistribution(int numGoldPiles, int* arr)
{
  int goldRemaining = GoldTotal - numGoldPiles;  // track number of gold left to allocate -1 for each pile
  int i = numGoldPiles - 1;
  while (i > 0) {
    int x = 0;
    if (goldRemaining > 0) {
      x = (rand() % goldRemaining);
    }
    int gold = x;
    arr[i] = gold + 1;
    goldRemaining -= gold;
    i--;
  }
  arr[0] = goldRemaining + 1;
}

/* ***************** parseArgs ********************** */
/*
 * checks the arguments given by the caller, ensuring that maps.txt is readable and setting the random seed number
 * if [seed] is provided. Otherwise, generate a random seed using process id.
 * 
 * We Return:
 *    1 if invalid seed given or map.txt given is not readable
 *    0 if valid map.txt and valid seed (if given)
 * 
 * Pseudocode:
 *    if 2 or 3 arguments provided, including the command itself, 
 *        if 3 arguments,
 *           return error code 1 if value is not a positive integer
 *           srand(value) if it is a positive integer
 *        else (if 2 arguments),
 *           srand(getPid())
 *        check if 2nd argument given is a readable file, returning error code if not readable
 *        
 *    else
 *        print to stderr and return error code
 *  
 */
static int
parseArgs(const int argc, char* argv[])
{
  if (argc == 2 || argc == 3) {
    if (argc == 3) {             // if map.txt and seed provided
      if (atoi(argv[2]) <= 0) {  // if seed provided but 0 or negative value,
        fprintf(stderr, "Seed provided must be a positive integer.\n");
        return 1;
      }
      else {  // if seed provided is positive integer
        srand(atoi(argv[2]));
      }
    }
    else {  // if seed not provided, get process id and set random sequence
      srand(getpid());
    }

    // check if map file provided is readable
    if (!isReadable(argv[1])) {
      fprintf(stderr, "Error. %s/1 is not readable\n", argv[1]);
      return 1;
    }
  }
  else {  // invalid number of arguments provided
    fprintf(stderr, "Invalid number of arguments provided. Please run ./server map.txt [seed]\n");
    return 1;  // exit with error code 1
  }
  return 0;  // successfully parsed args
}

/* ***************** isReadable ********************** */
/*
 * check if path is readable
 * return true if file able to open to read
 * return false if file cannot open to read
 */
static bool
isReadable(char* pathName)
{
  FILE* fp;
  if ((fp = fopen(pathName, "r")) == NULL) {  // if unable to read file, return error.
    fprintf(stderr, "Unable to read %s\n", pathName);
    return false;
  }
  fclose(fp);
  return true;
}

/* ***************** handleMessage ********************** */
/*
 * Handle all messages passed from the client to the server based on protocol
 * in requirements spec.
 * 
 * We return:
 *  true if ending game and exiting message loop
 *  false if game still ongoing and should remain in message loop
 * 
 * Pseudocode:
 *    if client sends PLAY:
 *        call playerJoin to join the player 
 *        send GOLD message to all clients connected
 *        send DISPLAY message to all clients connected
 *        update spectator's display
 *    else if client sends SPECTATE
 *        call spectatorJoin, initializing the spectator
 *    else if message starts with "KEY "
 *        find the player in game->allPlayers
 *        if character is lower character,
 *           call player_moverRegular
 *           if player_moveRegular returns true, it is a valid move and player moves and collects gold accordingly
 *                if game->numGoldLeft is 0, no more gold in game, end the game and send QUIT message to all clients
 *                update all clients' GOLD
 *                update all clients' DISPLAY
 *                updateSpectatorDisplay
 *           else, it is an invalid move and server sends message to client informing them that it is invalid
 *        if character is uppercase,
 *           if character is Q,
 *              if it is a player
 *                  call player_quit
 *              if it is a spectator
 *                  set spectatorAddressID to 0, quitting the spectator and marking that no spectator is connected
 *              send QUIT message to spectator/player
 *              sendGoldMessage to all clients
 *              sendDisplayMessage to all clients
 *              updateSpectatorDisplay
 *              
 *           else
 *              call player_moveCapital
 *              if player_moveCapital returns true, it is a valid move and player moves and collects gold accordingly
 *                if game->numGoldLeft is 0, no more gold in game, end the game and send QUIT message to all clients
 *                   update all clients' GOLD
 *                   update all clients' DISPLAY
 *                   updateSpectatorDisplay
 *               else, it is an invalid move and server sends message to client informing them that it is invalid
 */
static bool
handleMessage(void* arg, const addr_t from, const char* message)
{
  if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
    const char* realName = message + strlen("PLAY ");  // get the real name after PLAY
    char* name = mem_malloc(strlen(realName) + 1);
    strcpy(name, realName);
    playerJoin(name, from);     // join player
    hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
    hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // update all player's displays
    updateSpectatorDisplay();
    mem_free(name);
  }
  else if (strncmp(message, "SPECTATE", strlen("SPECTATE")) == 0) {
    spectatorJoin(&from);
  }
  else if (strncmp(message, "KEY ", strlen("KEY ")) == 0) {
    char move = message[strlen("KEY ")];
    // if message is a character
    player_t* player = hashtable_find(game->allPlayers, message_stringAddr(from));
    if (islower(move)) {                                                                                     // lower character
      if (!player_moveRegular(player, move, game->allPlayers, game->grid, game->gold, game->numGoldLeft)) {  // if not valid keystroke given
        fprintf(stderr, "Error. Invalid keystroke %s", message);                                             // invalid input keystroke
        message_send(from, "ERROR. Invalid keystroke.\n");                                                   // invalid input keystroke
      }
      else {
        // player was successfully moved
        if (*game->numGoldLeft == 0) {  // if no more gold left
          endGame();                    // end game, send summary to all players, delete players
          return true;                  // stay in message loop
        }
        // update gold and play displays whenever a keystroke is pressed
        hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
        hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
        updateSpectatorDisplay();
      }
    }
    else {                                                                     // if capital letter
      if (move == 'Q') {                                                       // if Q, tell client to QUIT and remove player from game
        if (hashtable_find(game->addrID, message_stringAddr(from)) != NULL) {  // if move is from a current player, quit the player
          player_quit(message_stringAddr(from), game->allPlayers);
        }
        else {  // if it is a spectator
          game->spectatorAddressID = 0;
        }
        message_send(from, "QUIT Thanks for playing!\n");
        // update gold and play displays whenever a keystroke is pressed
        hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
        hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
        updateSpectatorDisplay();
      }
      else {
        if (!player_moveCapital(player, move, game->allPlayers, game->grid, game->gold, game->numGoldLeft)) {
          // if not valid keystroke given
          fprintf(stderr, "Error. Invalid keystroke %s", message);  // invalid input keystroke
          message_send(from, "ERROR. Invalid keystroke.\n");
        }
        else {
          if (*game->numGoldLeft == 0) {  // if no more gold left
            endGame();                    // end game, send summary to all players, delete players
            return true;                  // exit message loop
          }
          // update gold and play displays whenever a keystroke is pressed
          hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
          hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
          updateSpectatorDisplay();
        }
      }
    }
  }
  return false;  // stay in message loop
}

static void 
updateSpectatorDisplay()
{
  if (game->spectatorAddressID != 0) {  // if spectator is connected, update spectator's display

    // creating gold message
    char goldMsg[50];
    sprintf(goldMsg, "GOLD 0 0 %d\n", *(game->numGoldLeft));

    // creating display message
    set_t* playerLoc = player_locations(game->allPlayers);
    set_t* spectatorLocations = grid_displaySpectator(game->grid, playerLoc, game->gold);
    char* display = grid_print(game->grid, spectatorLocations);
    char* displayMessage = mem_malloc(strlen("DISPLAY\n") + strlen(display) + 1);
    strcpy(displayMessage, "DISPLAY\n");
    strcat(displayMessage, display);

    addr_t specAddr = game->addresses[game->spectatorAddressID];  // get spectator address using its index
    message_send(specAddr, goldMsg);         // send gold messsage
    message_send(specAddr, displayMessage);  // send display message

    // clear memory space
    set_delete(playerLoc, itemDelete);
    set_delete(spectatorLocations, NULL);
    mem_free(display);
    mem_free(displayMessage);
  }
}

// end the game, sending quit messages to all connnected clients and freeing all game memory
static void
endGame()
{
  // Update gold and display one final time for all players
  hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
  hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
  updateSpectatorDisplay();

  char* summary = player_summary(game->allPlayers);

  // send quit message with summary to all players
  hashtable_iterate(game->allPlayers, summary, sendEndMessage);

  // send quit message with summary to spectator
  if (game->spectatorAddressID != 0) {
    char* quitSpectatorMessage = mem_malloc(strlen(summary) + strlen("QUIT GAME OVER:\n") + 1);
    strcpy(quitSpectatorMessage, "QUIT GAME OVER:\n");
    strcat(quitSpectatorMessage, summary);
    // int* addrID = hashtable_find(game->addrID, game->spectatorAddress);
    addr_t specAddr = game->addresses[game->spectatorAddressID];
    message_send(specAddr, quitSpectatorMessage);
    mem_free(quitSpectatorMessage);
  }

  mem_free(summary);

  // free all memory
  hashtable_delete(game->allPlayers, deletePlayer);  // delete every player in hashtable
  hashtable_delete(game->addrID, itemDelete);        // delete all the address ids, freeing the item
  counters_delete(game->gold);
  grid_delete(game->grid);
  mem_free(game->numGoldLeft);
  mem_free(game->addresses);
  // mem_free(game->spectatorAddress);
  mem_free(game);
}

// sends to each player the gold they had just collect, the gold in their purse, and the remaining gold in game
static void
sendGoldMessage(void* arg, const char* addr, void* item)
{
  player_t* player = item;
  int* id = NULL;
  id = hashtable_find(game->addrID, addr);
  if (id != NULL && player != NULL) {  // if address exists and player still in game
    char goldM[50];
    sprintf(goldM, "GOLD %d %d %d\n", player_getRecentGold(player), player_getpurse(player), *(game->numGoldLeft));
    addr_t actualAddr = game->addresses[*id];  // get the address of player
    message_send(actualAddr, goldM);           // send gold message
  }
}

// send display message to each player
static void
sendDisplayMessage(void* arg, const char* addr, void* item)
{
  // display message
  player_t* player = item;
  int* addrID = hashtable_find(game->addrID, addr);
  if (addrID != NULL && player != NULL) {          // if player address exists and player still in game
    addr_t actualAddr = game->addresses[*addrID];  // get player's address
    // hashtable_print(game->allPlayers, stdout, playeritemprint);

    set_t* playerLocations = player_locations(game->allPlayers);
    printf("\n%s", "HERE");

    set_t* newSeenBefore = grid_updateView(game->grid, player_getCurrCoor(player), player_getSeenBefore(player), playerLocations, game->gold);
    // set_print(newSeenBefore, stdout, setitemprint);

    char* display = grid_print(game->grid, newSeenBefore);
    char* displayMessage = mem_malloc(strlen("DISPLAY\n") + strlen(display) + 1);
    strcpy(displayMessage, "DISPLAY\n");
    strcat(displayMessage, display);           // send all locations that player can see and have seen
    message_send(actualAddr, displayMessage);  // send display message

    set_delete(playerLocations, itemDelete);
    set_delete(player_getSeenBefore(player), NULL);
    player_setSeenBefore(player, newSeenBefore);

    mem_free(display);
    mem_free(displayMessage);
  }
}

// delete a player
static void
deletePlayer(void* item)
{
  player_t* player = item;
  if (player != NULL) {
    player_delete(player);
  }
}

static void
sendEndMessage(void* arg, const char* addr, void* item)
{
  char* summary = arg;
  char* message = mem_malloc(strlen(summary) + strlen("QUIT GAME OVER:\n") + 1);
  strcpy(message, "QUIT GAME OVER:\n");
  strcat(message, summary);
  int* id = hashtable_find(game->addrID, addr);
  if (id != NULL && item != NULL) {  // if player still connected, tell client to quit
    addr_t actualAddr = game->addresses[*id];
    message_send(actualAddr, message);
  }
  mem_free(message);
}

// adapted from message.c
static bool
handleInput(void* arg)
{
  // allocate a buffer into which we can read a line of input
  // (it can't be any bigger than a message)
  char line[message_MaxBytes];

  // read a line from stdin
  if (fgets(line, message_MaxBytes, stdin) != NULL) {
    const int len = strlen(line);
    if (len > 0) {
      line[len - 1] = '\0';  // change newline to null
    }
    return false;
  }
  else {
    return true;  // EOF
  }
}

static void
itemprint(FILE* fp, const char* key, void* item)
{
  int* addrID = item;
  if (addrID == NULL || key == NULL) {
    fprintf(fp, "(null)");
  }
  else {
    addr_t addr = game->addresses[*addrID];
    fprintf(fp, "(%s, %s)", key, message_stringAddr(addr));
  }
  fflush(stdout);
}

static void
playeritemprint(FILE* fp, const char* key, void* item)
{
  fprintf(fp, "%s ", key);
  player_print((player_t*)item);
}

static void
setitemprint(FILE* fp, const char* key, void* item)
{
  fprintf(fp, "%s %s", key, (char*)item);
}

/* ***************** playerJoin ********************** */
/*
 * Initializes new player to game, and send GRID message to the client
 * Note: Does not send GOLD & DISPLAY message to client here
 */
static void
playerJoin(char* name, const addr_t client)
{
  if (game->numPlayers < MaxPlayers) {
    player_t* newPlayer = player_new(name, game->grid, game->allPlayers, game->numGoldLeft, game->gold, game->numPlayers);
    // player_print(newPlayer);

    if (game->numGoldLeft == 0) {  // if no more gold left
      printf("%s", "game ending");
      fflush(stdout);
      endGame();
    }
    int buffer = 20;

    // OK message
    int okLength = strlen("OK ") + buffer;
    char okMessage[okLength];
    snprintf(okMessage, okLength, "OK %s", player_getID(newPlayer));

    // grid message
    int gridLength = strlen("GRID") + buffer;
    char gridMessage[gridLength];
    snprintf(gridMessage, gridLength, "GRID %d %d", grid_getNumberRows(game->grid), grid_getNumberCols(game->grid));

    // create an id for the new player and store it in game->addrID and game->addresses respectively
    int* newAddrID = mem_malloc(sizeof(int));
    *newAddrID = game->numPlayers;
    game->addresses[game->numPlayers] = client;    // store the address of the player
    for (int i = 0; i <= game->numPlayers; i++) {  // debug addresses array
      fprintf(stdout, "{%s}", message_stringAddr(game->addresses[i]));
      fflush(stdout);
    }
    hashtable_insert(game->addrID, message_stringAddr(client), newAddrID);      // store new player's address
    hashtable_insert(game->allPlayers, message_stringAddr(client), newPlayer);  // store new player in allPlayers

    set_t* playerLocations = player_locations(game->allPlayers);
    set_t* currSeenBfr = player_getSeenBefore(newPlayer);
    set_delete(currSeenBfr, NULL);
    player_setSeenBefore(newPlayer, grid_updateView(game->grid, player_getCurrCoor(newPlayer), NULL, playerLocations, game->gold));

    message_send(client, okMessage);    // send the player message
    message_send(client, gridMessage);  // send grid message
    // hashtable_print(game->addrID, stdout, itemprint);  // debug
    (game->numPlayers)++;

    set_delete(playerLocations, itemDelete);
  }
}

/* ***************** spectatorJoin ********************** */
/*
 * Adds a spectator to the server and sends GRID, GOLD, DISPLAY message to the spectator
 */
static void
spectatorJoin(const addr_t* address)
{
  if (game->spectatorAddressID != 0) {  // if spectator exists, send quit message to spectator
    // int* addrID = hashtable_find(game->addrID, game->spectatorAddress);
    addr_t specAddr = game->addresses[game->spectatorAddressID];
    message_send(specAddr, "QUIT You have been replaced by a new spectator.\n");
    // mem_free(game->spectatorAddress);   // free spectatorAddress memory
  }
  else {                                    // if no spectator in game and one joins
    game->spectatorAddressID = MaxPlayers;  // set id to spot reserved for spectator's address in game->addresses
  }

  // hashtable_find(game->addrID, game->spectatorAddress);
  game->addresses[game->spectatorAddressID] = *address;  // store spectator address
  fprintf(stdout, "new spectator address stored: %s", message_stringAddr(game->addresses[game->spectatorAddressID]));

  // hashtable_insert(game->addresses, message_stringAddr(*address), &address);

  // grid message
  char gridMessage[30];
  sprintf(gridMessage, "GRID %d %d", grid_getNumberRows(game->grid), grid_getNumberCols(game->grid));

  // gold message
  char goldMessage[50];
  sprintf(goldMessage, "GOLD 0 0 %d\n", *(game->numGoldLeft));

  // display message
  set_t* playerLoc = player_locations(game->allPlayers);
  set_t* spectatorLocations = grid_displaySpectator(game->grid, playerLoc, game->gold);
  char* display = grid_print(game->grid, spectatorLocations);
  char* displayMessage = mem_malloc(strlen("DISPLAY\n") + strlen(display) + 1);
  strcpy(displayMessage, "DISPLAY\n");
  strcat(displayMessage, display);
  // int* addrID = hashtable_find(game->addresses, game->spectatorAddress);
  addr_t specAddr = game->addresses[game->spectatorAddressID];
  message_send(specAddr, gridMessage);     // send grid message
  message_send(specAddr, goldMessage);     // send gold message
  message_send(specAddr, displayMessage);  // send display message
  mem_free(display);
  mem_free(displayMessage);
  set_delete(playerLoc, itemDelete);
  set_delete(spectatorLocations, NULL);
}

// delete a name
static void
itemDelete(void* item)
{
  if (item != NULL) {
    mem_free(item);
  }
}