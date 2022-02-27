#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include "message.h"
#include "counters.h"
#include "mem.h"
#include <unistd.h>
// #include "grid.h"

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
/**************** local types ****************/
typedef struct game {
  hashtable_t* allPlayers;
  int numGoldLeft;
  // grid_t* grid;
  counters_t* gold;
  char* spectatorAddress; 
} game_t;

static game_t* game;                   // game struct storing the state of the game
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles

/* ***************** main ********************** */
int
main (const int argc, char* argv[]) 
{  
  int exitStatus = parseArgs(argc, argv);
  if (exitStatus == 1) {
    exit(1);
  }
  initializeGame(argv);
  
  // initialize the message module (without logging)
  int port;
  if (port = message_init(stderr) == 0) {
    fprintf(stderr, "Failed to initialize message module.\n");
    exit(2); // failure to initialize message module
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
  grid_read(argv[2]); // build the grid
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
    if (argc == 3) {  // if map.txt and seed provided
      if (atoi(argv[3]) <= 0) { // if seed provided but 0 or negative value,
        fprintf(stderr, "Seed provided must be a positive integer.\n");
        return 1;
      } else { // if seed provided is positive integer
        srand(atoi(argv[3]));
      }
    } else { // if seed not provided, get process id and set random sequence
      srand(getpid());
    }
    
    // check if map file provided is readable
    if (!isReadable(argv[2])) {
      fprintf(stderr, "Error. %s/1 is not readable\n", argv[2]);
      return 1;
    }
  } else {  // invalid number of arguments provided
    fprintf(stderr, "Invalid number of arguments provided. Please run ./server map.txt [seed]\n");
    return 1;    // exit with error code 1
  }
  return 0; // successfully parsed args
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
  // if (from == NULL) {
  //   log_v("handleMessage called with arg=NULL");
  //   return true;
  // }
  // if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {
  //   const char* content = message + strlen("PLAY ");
  //   playerJoin()
  // } else if (strncmp(message, "PLAY ", strlen("PLAY ")) == 0) {

  // }
  // get the first word
  
}

static void
handleTimeout(void* arg)
{

}


static void
handleInput(void* arg)
{

}

