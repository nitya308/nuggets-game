/* 
 * playertest.c - test program for players module
 *
 * Nitya Agarwala, CS50, Jan 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "../libcs50/file.h"

/* **************************************** */
int main()
{
    player_t* p1 = NULL;
    player_t* p2 = NULL;

    // ***** Testing player_new *****
    printf("Create a player...\n");
    p1 = player_new("Alice", );
    if (p1 == NULL) {
        fprintf(stderr, "countrs_new failed for p1\n");
        exit(1);
    }

    // ***** Testing player_add *****
    // Cases where player_add should NOT work
    printf("\nTest adding to null player\n");
    if (player_add(NULL, 0) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    printf("\nTest adding to null player\n");
    if (player_add(p2, 0) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    printf("\nTest adding with negative key\n");
    if (player_add(p1, -5) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    // Cases where player_add should work
    printf("\nTest adding key %d that does not already exist\n", key0);
    numKeysAdded++;
    if (player_add(p1, key0) != ++value) {
        fprintf(stderr, "error: player_add failed");
    }
    printf("key = %d has value = %d (should be %d)\n", key0, player_get(p1, key0), value);

    printf("\nTest adding key %d that already exists\n", key0); //should increment by 1
    if (player_add(p1, key0) != ++value) {
        fprintf(stderr, "error: player_add failed");
    }
    printf("key = %d has value = %d (should be %d)\n", key0, player_get(p1, key0), value);

    // ***** Testing player_get *****
    // Cases where player_get should NOT return value other than 0
    printf("\nTest getting from null player\n");
    if (player_get(NULL, 1) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    printf("\nTest getting negative key\n");
    if (player_get(p1, -5) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    printf("\nTest getting key that does not exist\n");
    if (player_get(p1, 5000) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    // Cases where player_get should return actual value
    printf("\nTest getting key = %d \n", key0);
    printf("key = %d, value = %d (should be %d)\n", key0, player_get(p1, key0), value);
    if (player_get(p1, key0) != value) {
        fprintf(stderr, "error: player_get failed");
    }

    // ***** Testing player_set *****
    // Cases where player_set should return false
    printf("\nTest setting on a null player\n");
    if (player_set(NULL, 1, 1) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    printf("\nTest setting on a key < 0\n");
    if (player_set(p1, -10, 1) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    printf("\nTest setting count < 0\n");
    if (player_set(p1, 1, -10) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    // Cases where player_set should return true
    printf("\nSetting a key = %d that already exists to 200\n", key0);
    if (player_set(p1, key0, 200) != true) {
        fprintf(stderr, "error: player_set failed");
    }
    printf("New value of key %d = %d (should be 200)\n", key0, player_get(p1, key0));

    printf("\nSetting keys that do not exist\n");
    for (int i = 0; i < 5; i++) {
        if (player_set(p1, keys[i], values[i]) != true) {
            fprintf(stderr, "error: player_set failed");
        }
        numKeysAdded++;
    }
    printf("\nMaking sure keys have been set right\n");
    for (int i = 0; i < 5; i++) {
        if (player_get(p1, keys[i]) != values[i]) {
            fprintf(stderr, "error: player_set failed");
        }
    }

    // ***** Testing player_print *****
    // Cases where player_print should print NULL
    printf("\nTest printing null player < 0 (should print null)\n");
    player_print(p2, stdout);

    printf("\nTest printing null fp (should print nothing)\n");
    player_print(p1, NULL);

    // Cases where player_print should work
    printf("\nTest printing p1\n");
    player_print(p1, stdout);

    // ***** Testing player_iterate *****
    int numKeysinplayer = 0; // number of key stored in player

    // Cases where player_iterate should do nothing
    printf("\n\nTest iterating over null player (should do nothing)\n");
    player_iterate(NULL, stdout, keysCount);

    printf("\nTest iterating over null function (should do nothing)\n");
    player_iterate(p1, stdout, NULL);

    // Cases where player_iterate should work
    printf("\nCounting all keys added using player\n");
    player_iterate(p1, &numKeysinplayer, keysCount);
    if (numKeysinplayer != numKeysAdded) {
        fprintf(stderr, "error: player_iterate failed");
    }
    printf("\nNumber of keys in player (should be %d) = %d\n", numKeysAdded, numKeysinplayer);

    // ***** Testing player_delete *****
    // Cases where player_delete should ignore
    printf("\nDeleting null player\n");
    player_delete(NULL);

    // Where player_delete should work
    printf("\nDeleting the player p1...\n");
    player_delete(p1);
    p1 = NULL; // setting pointer to NULL so no one accidentally uses it again

    printf("\nplayer after deletion\n");
    player_print(p1, stdout);

    printf("\n\n***Testing complete***\n");
    exit(0); //testing complete
}

/* count the non-null items in the player
 * here we don't care what kind of item is in player.
 * credit: CS50 bagtest file provided to us
 */
static void keysCount(void* arg, const int key, const int count)
{
    int* nitems = arg;
    if (nitems != NULL && count!=0) {
      (*nitems)++;
    }       
}