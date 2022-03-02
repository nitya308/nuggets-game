#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "counters.h"
#include "grid.h"
#include "hashtable.h"
#include "mem.h"
#include "message.h"
#include "player.h"

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
static void handleInput(void* arg);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool isReadable(char* pathName);
static void playerJoin(char* name, hashtable_t* allPlayers, hashtable_t* addresses, addr_t* client, grid_t* grid, int* numPlayers);
static void spectatorJoin(addr_t* address);
static void buildGrid(grid_t* grid, char* argv);
static void endGame();
static void deletePlayer(void* item);
static void itemcount(void* arg, const char* key, void* item);
static void stringDelete(void* item);
static void sendDisplayMessage(void* arg, const char* addr, void* item);
static void sendGoldMessage(void* arg, const char* addr, void* item);
static void sendEndMessage(void* arg, const char* addr, void* item);
/**************** local types ****************/
typedef struct game {
  hashtable_t* allPlayers;
  hashtable_t* addresses;
  int numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  addr_t* spectatorAddress;
} game_t;

static game_t* game;                    // game struct storing the state of the game
static const int MaxNameLength = 50;    // max number of chars in playerName
static const int MaxPlayers = 26;       // maximum number of players
static const int GoldTotal = 250;       // amount of gold in the game
static const int GoldMinNumPiles = 10;  // minimum number of gold piles
static const int GoldMaxNumPiles = 30;  // maximum number of gold piles

/* ***************** main ********************** */
int main(const int argc, char* argv[])
{
  int exitStatus = parseArgs(argc, argv);
  if (exitStatus == 1) {
    exit(1);
  }
  initializeGame(argv);  // initialize the game with the map

  // play the game
  // initialize the message module (without logging)
  int port;
  if (port = message_init(stderr) == 0) {
    fprintf(stderr, "Failed to initialize message module.\n");
    exit(2);  // failure to initialize message module
  }
  printf("Ready to play, waiting at port %d", port);

  // Loop, waiting for input or for messages; provide callback functions.
  bool ok = message_loop(port, 0, NULL, handleInput, handleMessage);

  // shut down the message module
  message_done();

  exit(0);  // successfully ran program
}

/* ***************** initializeGame ********************** */
static void
initializeGame(char* argv)
{
  game = mem_malloc(sizeof(game_t));
  if (game == NULL) {
    fprintf(stderr, "Failed to create game. Exiting...\n");
    exit(1);
  }
  buildGrid(game->grid, argv[2]);
  game->numGoldLeft = GoldTotal;
  game->allPlayers = hashtable_new(MaxPlayers);
  if (game->allPlayers == NULL) {
    fprintf(stderr, "Failed to create allPlayers hashtable. Exiting...\n");
    exit(1);
  }
  game->gold = counters_new();
  if (game->gold == NULL) {
    fprintf(stderr, "Failed to create gold counters. Exiting...\n");
    exit(1);
  }
  game->spectatorAddress = NULL;
}

/* ***************** buildGrid ********************** */
static void
buildGrid(grid_t* grid, char* argv)
{
  game->grid = grid_read(argv[2]);  // build the grid
  initializeGoldPiles();
}

/* ***************** initializeGoldPiles ********************** */
static void
initializeGoldPiles()
{
  int numGoldPiles = (rand() % (GoldMaxNumPiles - GoldMinNumPiles + 1)) + GoldMinNumPiles;  // generate a value between min and max range of gold piles
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
  int nRows = getNumberRows(game->grid);
  int nCols = getNumberCols(game->grid);
  int i = 0;
  while (i < numGoldPiles) {
    int location = rand() % (nRows * nCols);          // get the index in the map
    if (grid_isOpen(game->grid, location)) {          // if it is an available space
      if (counters_get(game->gold, location) != 0) {  // if it is an existing gold pile
        continue;                                     // do not store as valid location
      }
      else {  // if location not occupied by gold
        arr[i] = location;
        numGoldPiles--;
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
    int gold = (rand() % goldRemaining) + 1;
    arr[i] = gold;
    goldRemaining -= gold;
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
      if (atoi(argv[3]) <= 0) {  // if seed provided but 0 or negative value,
        fprintf(stderr, "Seed provided must be a positive integer.\n");
        return 1;
      }
      else {  // if seed provided is positive integer
        srand(atoi(argv[3]));
      }
    }
    else {  // if seed not provided, get process id and set random sequence
      srand(getpid());
    }

    // check if map file provided is readable
    if (!isReadable(argv[2])) {
      fprintf(stderr, "Error. %s/1 is not readable\n", argv[2]);
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

/* ***************** handleMessages ********************** */
/*
 * Handle all messages passed from the client to the server based on protocol
 * in requirements spec.
 */
static bool
handleMessage(void* arg, const addr_t from, const char* message)
{
  if (&from == NULL) {
    log_v("handleMessage called with arg=NULL");
    return true;
  }
  if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
    const char* realName = message + strlen("PLAY ");  // get the real name after PLAY
    playerJoin(realName, game->allPlayers, game->addresses, &from, game->grid, &game->numPlayers);
    hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
    hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // update all player's displays
  }
  else if (strncmp(message, "SPECTATE ", strlen("SPECTATE ")) == 0) {
    spectatorJoin(&from);
  }
  else if (isalpha(message)) {  // if message is a character
    player_t* player = hashtable_find(game->allPlayers, message_stringAddr(from));
    if (islower(message)) {                                       // lower character
      if (!player_moveRegular(player, message, game)) {           // if not valid keystroke given
        fprintf(stderr, "Error. Invalid keystroke %s", message);  // invalid input keystroke
        message_send(from, "ERROR. Invalid keystroke.\n");        // invalid input keystroke
      }
      else {
        // player was successfully moved
        if (game->numGoldLeft == 0) {  // if no more gold left
          endGame();                   // end game, send summary to all players, delete players
          return true;
        }
        // update gold and play displays whenever a keystroke is pressed
        hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
        hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
      }
    }
    else {                   // if capital letter
      if (message == 'Q') {  // if Q, tell client to QUIT and remove player from game
        message_send(from, "QUIT Thanks for playing!\n");
        player_quit(message_stringAddr(from), game->allPlayers);
      }
      else {
        if (!player_moveCapital(player, message, game)) {
          // if not valid keystroke given
          fprintf(stderr, "Error. Invalid keystroke %s", message);  // invalid input keystroke
          message_send(from, "ERROR. Invalid keystroke.\n");
        }
        else {
          if (game->numGoldLeft == 0) {  // if no more gold left
            endGame();                   // end game, send summary to all players, delete players
            return true;
          }
          // update gold and play displays whenever a keystroke is pressed
          hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);     // send gold messages to all players
          hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);  // send display messages to all players
        }
      }
    }
  }
  if (game->spectatorAddress != NULL) {  // if spectator is connected, update spectator's display
    set_t* spectatorLocations = grid_displaySpectator(game->grid, player_locations(game->allPlayers), game->gold);
    char* displayMessage = "DISPLAY\n";
    strcat(displayMessage, grid_print(game->grid, spectatorLocations));
    message_send(*game->spectatorAddress, displayMessage);  // send display message
    set_delete(spectatorLocations, stringDelete);           // free spectatorLocations memory
  }
}

// end the game, sending quit messages to all connnected clients and freeing all game memory
static void
endGame()
{
  char* summary = player_summary(game->allPlayers);

  // send quit message with summary to all players
  hashtable_iterate(game->allPlayers, summary, sendEndMessage);

  // send quit message with summary to spectator
  if (game->spectatorAddress != NULL) {
    char* quitSpectatorMessage = "QUIT GAME OVER:\n";
    strcat(quitSpectatorMessage, summary);
    message_send(*game->spectatorAddress, quitSpectatorMessage);
  }

  // free all memory
  hashtable_delete(game->allPlayers, deletePlayer);  // delete every player in hashtable
  hashtable_delete(game->addresses, NULL);
  counters_delete(game->gold);
  grid_delete(game->grid);
  mem_free(game->spectatorAddress);
  mem_free(game);
}

// sends to each player the gold they had just collect, the gold in their purse, and the remaining gold in game
static void
sendGoldMessage(void* arg, const char* addr, void* item)
{
  player_t* player = item;
  addr_t* addrCast = hashtable_find(game->addresses, addr);  // convert string address to addr_t
  if (addrCast != NULL && player != NULL) {                  // if player address exists and player still in game
    int buffer = 20;
    int goldLength = strlen("GOLD") + buffer;
    char goldMessage[goldLength];
    snprintf(goldMessage, strlen(goldMessage) + buffer, "GOLD %d %d %d\n", player->recentGoldCollected, player->purse, game->numGoldLeft);
    message_send(*addrCast, goldMessage);  // send gold message
  }
}

// send display message to each player
static void
sendDisplayMessage(void* arg, const char* addr, void* item)
{
  // display message
  player_t* player = item;
  addr_t* addrCast = hashtable_find(game->addresses, addr);  // convert string address to addr_t
  if (addrCast != NULL && player != NULL) {                  // if player address exists and player still in game
    set_t* seenBefore = player->seenBefore;
    char* displayMessage = "DISPLAY\n";
    strcat(displayMessage, grid_print(game->grid, seenBefore));  // send all locations that player can see and have seen
    message_send(*addrCast, displayMessage);                     // send display message
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
  char* message = "QUIT GAME OVER:\n";
  char* summary = arg;
  strcat(message, summary);
  addr_t* addrCast = hashtable_find(game->addresses, addr);
  if (addrCast != NULL && item != NULL) {  // if player still connected, tell client to quit
    message_send(*addrCast, message);
  }
}

static void
handleInput(void* arg)
{
}

/* ***************** playerJoin ********************** */
/*
 * Initializes new player to game, and send GRID message to the client
 * Note: Does not send GOLD & DISPLAY message to client here
 */
static void
playerJoin(char* name, hashtable_t* allPlayers, hashtable_t* addresses, addr_t* client, grid_t* grid, int* numPlayers)
{
  if (*numPlayers < MaxPlayers) {
    player_t* newPlayer = player_new(name, grid);

    int buffer = 20;

    // grid message
    int gridLength = strlen("GRID") + buffer;
    char gridMessage[gridLength];
    snprintf(gridMessage, strlen(gridMessage) + buffer, "GRID %d %d", grid_getNumberRows(game->grid), grid_getNumberCols(game->grid));

    // gold message
    // int goldLength = strlen("GOLD") + buffer;
    // char goldMessage[goldLength];
    // snprintf(goldMessage, strlen(goldMessage) + buffer, "GOLD %d %d %d", newPlayer->purse, newPlayer->purse, game->numGoldLeft);

    // char* displayMessage = "DISPLAY\n";
    // strcat(displayMessage, grid_print(game->grid, newPlayer->seenBefore));

    hashtable_insert(allPlayers, message_stringAddr(*client), newPlayer);  // store new player in allPlayers
    hashtable_insert(addresses, message_stringAddr(*client), client);      // store new player's address
    message_send(*client, gridMessage);                                    // send grid message
    hashtable_iterate(game->allPlayers, NULL, sendGoldMessage);            // send gold messages to all players
    hashtable_iterate(game->allPlayers, NULL, sendDisplayMessage);         // send display messages to all players
    // message_send(*client, goldMessage);     // send gold message
    // message_send(*client, displayMessage);  // send display message
    *numPlayers++;
  }
}

/* ***************** playerJoin ********************** */
/*
 * Adds a spectator to the server and sends GRID, GOLD, DISPLAY message to the spectator
 */
static void
spectatorJoin(addr_t* address)
{
  if (game->spectatorAddress == NULL) {  // if no spectator, set address
    game->spectatorAddress = address;
  }
  else {
    message_send(*game->spectatorAddress, "QUIT You have been replaced by a new spectator.\n");
    game->spectatorAddress = address;  // update new address
  }
  int buffer = 10;

  // grid message
  int gridLength = strlen("GRID") + buffer;
  char gridMessage[gridLength];
  snprintf(gridMessage, strlen(gridMessage) + buffer, "GRID %d %d", grid_getNumberRows(game->grid), grid_getNumberCols(game->grid));

  // gold message
  int goldLength = strlen("GOLD 0 0") + buffer;
  char goldMessage[goldLength];
  snprintf(goldMessage, strlen(goldMessage) + buffer, "GOLD 0 0 %d", game->numGoldLeft);

  // display message
  set_t* spectatorLocations = grid_displaySpectator(address, player_locations(game->allPlayers), game->gold);
  char* displayMessage = "DISPLAY\n";
  strcat(displayMessage, grid_print(game->grid, spectatorLocations));

  message_send(*game->spectatorAddress, gridMessage);     // send grid message
  message_send(*game->spectatorAddress, goldMessage);     // send gold message
  message_send(*game->spectatorAddress, displayMessage);  // send display message
  set_delete(spectatorLocations, stringDelete);           // free spectatorLocations memory
}

// delete a name
static void
stringDelete(void* item)
{
  if (item != NULL) {
    mem_free(item);
  }
}