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
static void playerJoin();
static void spectatorJoin();
static void buildGrid(grid_t* grid, char* argv);
static void gameDelete();
static void deletePlayer(void* item);
static void sendQuit(void* arg, const char* addr, void* item);
static void itemcount(void* arg, const char* key, void* item);
/**************** local types ****************/
typedef struct game {
  hashtable_t* allPlayers;
  hashtable_t* addresses;
  int numGoldLeft;
  int numPlayers;
  grid_t* grid;
  counters_t* gold;
  address_t* spectatorAddress;
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
  while (idx < numGoldPiles) {
    counters_set(game->gold, randomLocations[idx], goldDistributionArray[idx]);
    idx++;
  }
  // while numGoldPiles > 0:
  //
  //    if grid_isOpen(row*column)

  // counters_set(game->gold, idx, goldDistributionArray[numGoldPiles-1]);
}

static void
generateRandomLocations(int numGoldPiles, int* arr)
{
  int nRows = getNumberRows(game->grid);
  int nCols = getNumberCols(game->grid);
  int i = 0;
  while (i < numGoldPiles) {
    int location = rand() % nRows * nCols;  // get the index in the map
    // int goldVal =
    if (grid_isOpen(location)) {                      // if it is an available space
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

static void
generateGoldDistribution(int numGoldPiles, int* arr)
{
  int goldRemaining = GoldTotal;  // track number of gold left to allocate
  int i = numGoldPiles;
  // while (i)
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
 *
 * Pseudocode:
 *
 *
 *
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
    playerJoin(realName, game->allPlayers, game->addresses from, game->grid, &game->numPlayers);
  }
  else if (strncmp(message, "SPECTATE ", strlen("SPECTATE ")) == 0) {
    spectatorJoin();
  }
  else if (strncmp(message, "QUIT ", strlen("QUIT ")) == 0) {
    player_quit(from);
    player_delete()
  }
  else if (isalpha(message)) {  // if message is a character
    player_t* player = hashtable_find(game->allPlayers, message_stringAddr(from));
    if (islower(message)) {
      if (!player_moveRegular(player, message, game)) {
        // if character is not a valid move
        // todo: log error here and ignore message
      }
    }
    else {
      if (message == 'Q') {  // if quit
        player_quit(from, game->allPlayers);
      }
      else {
        if (!player_moveCapital(player, message, game)) {
          // if character is not a valid move
          // todo: log error here and ignore message
        }
      }
    }
    if (game->numGoldLeft == 0) {
      char* summary = player_summary(game->allPlayers);
      hashtable_iterate(game->allPlayers, summary, sendQuit);  // send quit message to all clients with summary
      hashtable_delete(game->allPlayers, deletePlayer);        // delete every player in hashtable
      return true;
    }
  }
}

static void
gameDelete()
{
  hashtable_delete(game->allPlayers, deletePlayer);  // delete every player in hashtable
  counters_delete(gold);
  grid_delete(game->grid);
  if (spectatorAddress != NULL) {
    mem_free(spectatorAddress);
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
sendQuit(void* arg, const char* addr, void* item)
{
  // message = "QUIT GAME OVER:\n" + arg;
  char* message = arg;
  addr_t addrCast = addr;
  message_send((addr_t*)addrCast, message);
}

static void
handleInput(void* arg)
{
}

static void
playerJoin(char* name, hashtable_t* allPlayers, hashtable_t* addresses, addr_t* client, grid_t* grid, int* numPlayers)
{
  if (*numPlayers < MaxPlayers) {
    player_t* newPlayer = player_new(name, grid);
    hashtable_insert(allPlayers, message_stringAddr(client), newPlayer);
    hashtable_insert(addresses, message_stringAddr(client), client);
    *numPlayers++;
    message_send(client, );  // send grid message
    message_send(client, );  // send gold message
    message_send(client, );  // send display message
  }
}

static void
spectatorJoin(addr_t* address)
{
  if (game->spectatorAddress == NULL) {
    game->spectatorAddress = address;
  }
  else {
    message_send(game->spectatorAddress, "QUIT");
    game->spectatorAddress = address;
    message_send(game->spectatorAddress, "GRID");     // send grid message
    message_send(game->spectatorAddress, "GOLD");     // send gold message
    message_send(game->spectatorAddress, "DISPLAY");  // send display message
  }
}