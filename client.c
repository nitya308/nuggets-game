/**
 * client.c -- the client program for our Nuggets game
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include "./support/message.h"
#include "./support/log.h"


// Data structures
typedef struct playerAttributes {
  char playerID;
  int purse;
  bool isPlayer;
  int numGoldLeft;
  int goldCollected;
  char* display;
} playerAttributes_t;

// Function prototypes
static void parseArgs(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool receiveMessage(void* arg, const addr_t from, const char* message);
static void checkDisplay(int nrow, int ncol);


/**
 * main
 */
static int main(const int argc, char* argv[])
{
  // Validate command-line args first
  parseArgs(argc, argv);

  // Check if message module can be initialized
  if (message_init(NULL) == 0) {
    exit(2);
  }

  addr_t server;

  // Check if address can be formed
  if (message_setAddr(argv[1], argv[2], &server) == false) {
      printf(stderr, "Unable to form address from %s %s\n", argv[1], argv[2]);
      exit(3);
  }
  
  // Handle messages
  bool loopResult = message_loop(&server, 0, NULL, handleInput, receiveMessage);
  message_done();
  endwin();

  return 0; // if successful
}

/**
 * 
 */
static void parseArgs(const int argc, char* argv[])
{
  
}

/**
 * 
 */
static bool handleInput(void* arg)
{

}

/**
 * 
 */
static bool receiveMessage(void* arg, const addr_t from, const char* message)
{

}

/**
 * 
 */
static void checkDisplay(int nrow, int ncol)
{

}
