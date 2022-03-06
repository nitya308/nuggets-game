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
 * crawler - explores a file starting from the given webpage, and at each page, the
 * crawler retrieves all the links in that page and creates a file with a unique id
 * for it indicating the depth from the original input webpage.
 *
 * usage: ./crawler seedURL pageDirectory maxDepth
 *   where seedURL is an 'internal' directory, to be used as the initial URL,
 *   where pageDirectory is the (existing) directory in which to write downloaded webpages, and
 *   where maxDepth is an integer in range [0..10] indicating the maximum crawl depth.
 *
 * Assumption: The pageDirectory does not contain any files whose name is an integer (i.e., 1, 2, ...).
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
/**************** local types ****************/
typedef struct game {
  hashtable_t* allPlayers;
  hashtable_t* addrID;
  addr_t* addresses;  // store all player addresses and an additional slot for spectatorAddress (last slot in array)
  int tempCount;
  int* numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  int spectatorAddressID;  // val=0 if no spectator joined, val=MaxPlayers if a spectator joined
  int port;
} game_t;

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

  // play the game
  // initialize the message module (without logging)
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

  // todo: handle ok
  if (ok) {
    exit(0);  // successfully ran program
  }
  else {
    exit(1);  // failed to run program, terminate with error code
  }
}

/* ***************** initializeGame ********************** */
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
    endGame();  // TODO: free the memory
    fprintf(stderr, "Failed to create allPlayers hashtable. Exiting...\n");
    exit(1);
  }
  game->addrID = hashtable_new(MaxPlayers);
  if (game->addrID == NULL) {
    endGame();  // TODO: free the memory
    fprintf(stderr, "Failed to create addresses hashtable. Exiting...\n");
    exit(1);
  }
  game->gold = counters_new();
  if (game->gold == NULL) {
    endGame();  // TODO: free the memory
    fprintf(stderr, "Failed to create gold counters. Exiting...\n");
    exit(1);
  }
  initializeGoldPiles();
  // counters_print(game->gold, stdout);
  game->addresses = mem_malloc((MaxPlayers + 1) * sizeof(addr_t));
  game->spectatorAddressID = 0;  // no spectator initially. set to MaxPlayers if spectator connected
  game->numPlayers = 0;
  game->tempCount = 0;
}

/* ***************** buildGrid ********************** */
static void
buildGrid(grid_t* grid, char** argv)
{
  char* filename = argv[1];
  game->grid = grid_read(filename);  // build the grid
}

/* ***************** initializeGoldPiles ********************** */
static void
initializeGoldPiles()
{
  int cols = grid_getNumberCols(game->grid);
  int rows = grid_getNumberRows(game->grid);
  int maxAvailableSpots = (rows * cols) - (rows * 2) - (cols * 2);
  int max = (GoldMaxNumPiles > maxAvailableSpots) ? maxAvailableSpots : GoldMaxNumPiles;
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

// generate an array of random valid locations on the grid
static void
generateRandomLocations(int numGoldPiles, int* arr)
{
  int nRows = grid_getNumberRows(game->grid);
  int nCols = grid_getNumberCols(game->grid);
  int i = 0;
  // counters_print(game->gold, stdout);
  while (i < numGoldPiles) {
    int location = rand() % (nRows * nCols);          // get the index in the map
    if (grid_isOpen(game->grid, location)) {          // if it is an available space
      if (counters_get(game->gold, location) != 0) {  // if it is an existing gold pile
        continue;                                     // do not store as valid location
      }
      else {  // if location not occupied by gold
        arr[i] = location;
        i++;
        counters_set(game->gold, location, 1);
      }
    }
  }
}

// generate an array of random gold amount, summing up to goldTotal
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
 * Builds the inverted-index data structure mapping words to (documentID, count) pairs
 * wherein each count represents the number of occurrences of the given word in the
 * given document. Ignore words with fewer than three characters, and "normalize"
 * the word before indexing.
 *
 * Pseudocode:
 *
 *   loops over document ID numbers, counting from 1
 *     loads a webpage from the document file 'pageDirectory/id'
 *     if successful,
 *      passes the webpage and docID to indexPage
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
 */
static bool
handleMessage(void* arg, const addr_t from, const char* message)
{
  printf("%s", "here in handle");
  fflush(stdout);
  if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
    const char* realName = message + strlen("PLAY ");  // get the real name after PLAY
    char* name = mem_malloc(strlen(realName) + 1);
    strcpy(name, realName);
    playerJoin(name, from);
    printf("%s", "updating everyone");
    fflush(stdout);
    hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
    hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // update all player's displays
    updateSpectatorDisplay();
    mem_free(name);
  }

  else if (strncmp(message, "SPECTATE", strlen("SPECTATE")) == 0) {
    printf("\n%s", "CALLING SPECTATOR JOIN!!");
    fflush(stdout);
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
          return true;
        }
        // update gold and play displays whenever a keystroke is pressed
        hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
        hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
        updateSpectatorDisplay();
      }
    }
    else {                // if capital letter
      if (move == 'Q') {  // if Q, tell client to QUIT and remove player from game
        if (hashtable_find(game->addrID, message_stringAddr(from)) != NULL) {
          // if move is from a current player, quit the player
          player_quit(message_stringAddr(from), game->allPlayers, game->gold, game->numGoldLeft);
          int* id = hashtable_find(game->addrID, message_stringAddr(from));
          *id = -1;
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
            return true;
          }
          // update gold and play displays whenever a keystroke is pressed
          hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
          hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
          updateSpectatorDisplay();
        }
      }
    }
  }
  return false;  // TODO: should we print an error???
}

static void updateSpectatorDisplay()
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
    // int* addrID = hashtable_find(game->addrID, game->spectatorAddress);
    addr_t specAddr = game->addresses[game->spectatorAddressID];  // get spectator address using its index
    message_send(specAddr, goldMsg);
    message_send(specAddr, displayMessage);  // send display message

    set_delete(playerLoc, itemDelete);
    set_delete(spectatorLocations, NULL);
    mem_free(display);
    mem_free(displayMessage);
    // set_delete(spectatorLocations, itemDelete);             // free spectatorLocations memory
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
  if (id != NULL && *id != -1 && player != NULL) {  // if address exists and player still in game
    printf("\n%s", "DOESN'T ENTER IF CONDITION");
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
  if (addrID != NULL && *addrID != -1 && player != NULL) {  // if player address exists and player still in game
    addr_t actualAddr = game->addresses[*addrID];           // get player's address
    // hashtable_print(game->allPlayers, stdout, playeritemprint);

    set_t* playerLocations = player_locations(game->allPlayers);

    set_t* newSeenBefore = grid_updateView(game->grid, player_getCurrCoor(player), player_getSeenBefore(player), playerLocations, game->gold);
    // set_print(newSeenBefore, stdout, setitemprint);

    char* display = grid_print(game->grid, newSeenBefore);
    char* displayMessage = mem_malloc(strlen("DISPLAY\n") + strlen(display) + 1);
    strcpy(displayMessage, "DISPLAY\n");
    strcat(displayMessage, display);           // send all locations that player can see and have seen
    message_send(actualAddr, displayMessage);  // send display message

    set_delete(playerLocations, itemDelete);
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
  if (id != NULL && *id != -1 && item != NULL) {  // if player still connected, tell client to quit
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