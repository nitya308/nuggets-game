/**
 * client.c -- the client program for our Nuggets game
 * This program contains a main function to handle program flow,
 * a function to parse command-line args, a function to handle
 * client input, a function to handle server output, and a function
 * to make the display sufficiently large.
 *
 * Ashna Kumar    3/7/22
 */

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "mem.h"
#include "message.h"

// Data structures
typedef struct playerAttributes {
  char playerID;
  int purse;
  bool isPlayer;
  int numGoldLeft;
  int goldCollected;
  char* display;
} playerAttributes_t;

// Global game variable (while it is not the 'game' struct seen in server;
// it is the global game variable for client-side use)
playerAttributes_t playerAttributes;

// Function prototypes
static void parseArgs(const int argc, char* argv[]);
static bool handleInput(void* arg);
static bool receiveMessage(void* arg, const addr_t from, const char* message);
static void checkDisplay(int nrow, int ncol);

/**************** main **********************/
/**
 * Main function that handles flow of program,
 * calls other functions, and initializes other
 * modules.
 *
 * Caller provides:
 *   Command-line arguments
 * We guarantee:
 *   Calling functions, initializing modules,
 *   Ensuring address can be created
 *   Exiting upon fatal error
 * We return:
 *   true if successful
 *   false if unsuccessful
 */
int main(const int argc, char* argv[])
{
  // Validate argv[1] and argv[2] from command-line args first
  parseArgs(argc, argv);

  // Check if message module can be initialized
  if (message_init(NULL) == 0) {
    fprintf(stderr, "Unable to initialize message module\n");
    exit(3);
  }

  addr_t server;

  // Check if address can be formed
  if (!message_setAddr(argv[1], argv[2], &server)) {
    fprintf(stderr, "Unable to form address from %s %s\n", argv[1], argv[2]);
    exit(4);
  }

  // Check if client is player or spectator
  if (argc == 4 && argv[3] != NULL) {
    playerAttributes.isPlayer = true;
    char message[56] = "PLAY ";  // Long enough to fit play and maxNameLength
    strcat(message, argv[3]);
    message_send(server, message);
  }

  else {
    playerAttributes.isPlayer = false;
    message_send(server, "SPECTATE");
  }

  // Handle messages
  message_loop(&server, 0, NULL, handleInput, receiveMessage);
  message_done();
  endwin();

  mem_free(playerAttributes.display);

  return 0;  // true if success, false if fail
}

/**************** parseArgs **********************/
/**
 * Parses and verifies server hostname and port
 *
 * Caller provides:
 *   command-line arguments
 * We guarantee:
 *   Exiting upon fatal error
 * We return:
 *   Nothing
 */
static void parseArgs(const int argc, char* argv[])
{
  // Check that command-line usage is correct
  if (argc != 3 && argc != 4) {
    fprintf(stderr, "There are an invalid number of arguments.\n");
    exit(1);
  }

  // Check if hostname and port are valid
  if (argv[1] == NULL || argv[2] == NULL) {
    fprintf(stderr, "Hostname or port is invalid.\n");
    exit(2);
  }
}

/**************** handleInput **********************/
/**
 * Verifies server addr pointer and handles input from
 * client keystrokes.
 *
 * Caller provides:
 *   pointer to void
 * We guarantee:
 *   Checking if server address pointer is null or invalid
 *   Sending quit message to server
 *   Sending keystroke message to server
 * We return:
 *   True upon error
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
  if (c == 'Q' || c == EOF) {
    // EOF/EOT case: stop looping
    message_send(*serverp, "KEY Q");
  }

  else {
    // send any other keystroke to server if client is player
    if (playerAttributes.isPlayer) {
      char str[2] = {c, '\0'};
      char message[6] = "KEY ";
      strcat(message, str);
      message_send(*serverp, message);
    }
  }
  return false;
}

/**************** receiveMessage **********************/
/**
 * Processes each message correctly and carries out the
 * appropriate logic to go with it.
 *
 * Caller provides:
 *   arg param, server address, message from server
 * We guarantee:
 *   each possible type of message from the server is handles
 * We return:
 *   True if game is quit
 *   False otherwise
 */
static bool receiveMessage(void* arg, const addr_t from, const char* message)
{
  // In the case of a quit message, print appropriate output to client
  if (strncmp(message, "QUIT ", strlen("QUIT ")) == 0) {
    const char* quitContent = message + strlen("QUIT ");
    endwin();
    printf("\n%s\n", quitContent);
    return true;
  }

  // In the case of gold message, assign variables depending on message
  else if (strncmp(message, "GOLD", strlen("GOLD")) == 0) {
    int n, p, r;
    // Parse message to get specific integer values
    sscanf(message, "GOLD %d %d %d", &n, &p, &r);
    playerAttributes.goldCollected = n;
    playerAttributes.purse = p;
    playerAttributes.numGoldLeft = r;
  }

  // In the case of grid message, call checkDisplay with # of rows and columns
  else if (strncmp(message, "GRID", strlen("GRID")) == 0) {
    int nrows;
    int ncols;
    sscanf(message, "GRID %d %d", &nrows, &ncols);
    checkDisplay(nrows, ncols);
  }

  // In the case of ok message, set player ID variable to given ID
  else if (strncmp(message, "OK", strlen("OK")) == 0) {
    const char* id = message + strlen("OK ");
    playerAttributes.playerID = *id;
  }

  // In the case of display message,
  else if (strncmp(message, "DISPLAY", strlen("DISPLAY")) == 0) {
    // Create variable to store message
    const char* displayContent = message + strlen("DISPLAY\n");
    if (playerAttributes.display != NULL) {
      strcpy(playerAttributes.display, displayContent);
    }
    clear();

    // Print these messages only if client is a player
    if (playerAttributes.isPlayer) {
      if (playerAttributes.goldCollected == 0) {
        printw("Player %c has %d nuggets (%d nuggets unclaimed).\n",
               playerAttributes.playerID, playerAttributes.purse, playerAttributes.numGoldLeft);
      }
      else {
        printw("Player %c has %d nuggets (%d nuggets unclaimed). GOLD received: %d\n",
               playerAttributes.playerID, playerAttributes.purse, playerAttributes.numGoldLeft,
               playerAttributes.goldCollected);
      }
    }
    // If client is spectator
    else {
      printw("Spectator: %d nuggets unclaimed.\n", playerAttributes.numGoldLeft);
    }

    printw(displayContent);
    refresh();
  }

  // In the case of error message from server, clear display and print to stderr
  else if (strncmp(message, "ERROR", strlen("ERROR")) == 0) {
    fprintf(stderr, "Error message received from server.\n");
    clear();
    // Tell player their keystroke was inaccurate
    if (playerAttributes.isPlayer) {
      printw("Player %c has %d nuggets (%d nuggets unclaimed). Invalid move\n",
             playerAttributes.playerID, playerAttributes.purse, playerAttributes.numGoldLeft);
    }
    printw("%s", playerAttributes.display);
    refresh();
  }

  // In the case of any other message received from server
  else {
    fprintf(stderr, "Server message has bad format.\n");
  }
  return false;
}

/**************** checkDisplay **********************/
/**
 * Ensures client's display size is large enough to fit grid.
 *
 * Caller provides:
 *   Number of rows and columns in grid
 * We guarantee:
 *   That the game will not prcoeed until the clien't display
 *   size is sufficiently large
 * We return:
 *   Nothing
 */
static void checkDisplay(int nrow, int ncol)
{
  // Initialize display
  initscr();
  cbreak();
  noecho();

  // Set up row and column variables
  int row;
  int col;

  playerAttributes.display = mem_malloc_assert(65507, "Out of memory for display\n");
  getmaxyx(stdscr, row, col);

  // While dimensions are not large enough, prompt user to expand their display window
  while (row < nrow + 1 || col < ncol + 1) {
    printw("Please increase the size of your display window and click enter\n");
    while (getch() != '\n') {
      continue;
    }
    getmaxyx(stdscr, row, col);
  }
}