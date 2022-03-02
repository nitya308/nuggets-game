/**
 * client.c -- the client program for our Nuggets game
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
//#include "./libcs50/mem.h"
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

// Global game variable (while it is not the 'game' struct seen in server, it is the global game variable for client-side use)
playerAttributes_t* playerAttributes;


/**
 * main
 */
static int main(const int argc, char* argv[])
{
  // Validate argv[1] and argv[2] from command-line args first
  parseArgs(argc, argv);
  

  // Check if message module can be initialized
  if (message_init(NULL) == 0) {
    exit(3);
  }

  addr_t server;

  // Check if address can be formed
  if (message_setAddr(argv[1], argv[2], &server) == false) {
      fprintf(stderr, "Unable to form address from %s %s\n", argv[1], argv[2]);
      exit(4);
  }
  
  // Check if client is player or spectator
  if (argc == 4 && argv[3] != NULL) {
    playerAttributes->isPlayer = true;
    char* message = strcat("PLAY ", argv[3]);
    message_send(server, message);
  }

  else {
    playerAttributes->isPlayer = false;
    message_send(server, "SPECTATE");
  }

  // Handle messages
  bool loopResult = message_loop(&server, 0, NULL, handleInput, receiveMessage);
  message_done();
  endwin();

  mem_free(playerAttributes->display);

  return 0; // if successful
}

/**
 * 
 */
static void parseArgs(const int argc, char* argv[])
{
  // Check that command-line usage is correct
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "There are an invalid number of arguments.\n");
    exit(1);
  }

  // Check if hostname and port are valid
  if (argv[1] == NULL || argv[2] == NULL) {
    fprintf(stderr, "Hostname or port is invalid.\n");
    exit(2);
  }
}

/**
 * 
 */
static bool handleInput(void* arg)
{
  // Using 'arg' to receive server addr_t
  addr_t* serverp = arg;

  // Check that arg is not NULL pointer
  if (serverp == NULL) {
    fprintf(stderr, "handleInput called with NULL argument\n");
    return true;
  }

  // Check that arg is an address value
  if (!message_isAddr(*serverp)) {
    fprintf(stderr, "Argument is not an address value\n");
    return true;
  }

  // Read client keystroke
  char c = getch();
  if (c == "Q") {
    // EOF/EOT case: stop looping
    message_send(*serverp, "KEY Q");
    return true;
  }
  
  else {
    // send as message to server
    char* message = strcat("KEY", c);
    message_send(*serverp, message);
  }
  return false;
}

/**
 * 
 */
static bool receiveMessage(void* arg, const addr_t from, const char* message)
{
  if (strncmp(message, "QUIT", strlen("QUIT")) == 0) {
    endwin();
    // print explanation? does server do this??
    return true;
  }

  if (strncmp(message, "GOLD", strlen("GOLD")) == 0) {
    int n, p, r;
    char* strMessage;
    sscanf(message, "%s %d %d %d", strMessage, &n, &p, &r);
    playerAttributes->goldCollected = n;
    playerAttributes->purse = p;
    playerAttributes->numGoldLeft = r;
  }

  if (strncmp(message, "GRID", strlen("GRID")) == 0) {
    int nrows;
    int ncols;
    char* strMessage;
    sscanf(message, "%s %d %d", strMessage, &nrows, &ncols);
    checkDisplay(nrows, ncols);
  }

  if (strncmp(message, "OK", strlen("OK")) == 0) {
    playerAttributes->playerID = message;
  }

  if (strncmp(message, "DISPLAY", strlen("DISPLAY")) == 0) {
    playerAttributes->display = message;
    clear();
    if (playerAttributes->isPlayer) {
      if (playerAttributes->goldCollected == 0) {
        printf("Player %s has %d nuggets (%d nuggets unclaimed). GOLD received: %d\n", playerAttributes->playerID, playerAttributes->purse, playerAttributes->numGoldLeft, playerAttributes->goldCollected);
      }
      else {
          printf("Player %s has %d nuggets (%d nuggets unclaimed).\n", playerAttributes->playerID, playerAttributes->purse, playerAttributes->numGoldLeft);
      }
    }
    else {
      printf("Spectator: %d nuggets unclaimed.\n", playerAttributes->numGoldLeft);
    }
    
    printf(playerAttributes->display);
    refresh();

  }

  if (strncmp(message, "ERROR", strlen("ERROR")) == 0) {
    printf(stderr, "Error message received from server.\n");
    clear();
    if (playerAttributes->isPlayer) {
      printf("Player %s has %d nuggets (%d nuggets unclaimed). Unknown keystroke\n", playerAttributes->playerID, playerAttributes->purse, playerAttributes->numGoldLeft, playerAttributes->goldCollected);
    }
    printf(playerAttributes->display);
    refresh();
  }

  else {
    fprintf(stderr, "Server message has bad format.\n");
  }
}

/**
 * 
 */
static void checkDisplay(int nrow, int ncol)
{
  // Initialize display
  initscr();
  cbreak();
  noecho();

  // Set uo row and column variables
  int row;
  int col;

  playerAttributes->display = mem_malloc_assert(65507, "Out of memory\n");
  getmaxyx(stdscr, row, col);

  while (row < nrow + 1 || col < ncol + 1) {
    printw("Please increase the size of your display window and click enter\n");
    if (getch() == "\n") {
      getmaxyx(stdscr, row, col);
    }
  }
}
