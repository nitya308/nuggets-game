/* 
 * playertest.c - test program for players module
 *
 * Nitya Agarwala, CS50, Jan 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "file.h"

/* **************************************** */
int main()
{
    player_t* p1 = NULL;
    player_t* p2 = NULL;
    int key0 = 100;                        // first key to store in counter
    int value = 0;                        // value of first key to put in counter
    int keys[5] = {1, 2, 3, 4, 5};        // array of keys to add to counter
    int values[5] = {10, 20, 30, 40, 50}; // array of values to add to counter
    int numKeysAdded = 0;                 // number of keys added to counter

    // ***** Testing player_new *****
    printf("Create a counter...\n");
    ctr1 = player_new();
    if (ctr1 == NULL) {
        fprintf(stderr, "countrs_new failed for ctr1\n");
        exit(1);
    }

    // ***** Testing player_add *****
    // Cases where player_add should NOT work
    printf("\nTest adding to null counter\n");
    if (player_add(NULL, 0) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    printf("\nTest adding to null counter\n");
    if (player_add(ctr2, 0) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    printf("\nTest adding with negative key\n");
    if (player_add(ctr1, -5) != 0) {
        fprintf(stderr, "error: player_add failed");
    }

    // Cases where player_add should work
    printf("\nTest adding key %d that does not already exist\n", key0);
    numKeysAdded++;
    if (player_add(ctr1, key0) != ++value) {
        fprintf(stderr, "error: player_add failed");
    }
    printf("key = %d has value = %d (should be %d)\n", key0, player_get(ctr1, key0), value);

    printf("\nTest adding key %d that already exists\n", key0); //should increment by 1
    if (player_add(ctr1, key0) != ++value) {
        fprintf(stderr, "error: player_add failed");
    }
    printf("key = %d has value = %d (should be %d)\n", key0, player_get(ctr1, key0), value);

    // ***** Testing player_get *****
    // Cases where player_get should NOT return value other than 0
    printf("\nTest getting from null counter\n");
    if (player_get(NULL, 1) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    printf("\nTest getting negative key\n");
    if (player_get(ctr1, -5) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    printf("\nTest getting key that does not exist\n");
    if (player_get(ctr1, 5000) != 0) {
        fprintf(stderr, "error: player_get failed");
    }

    // Cases where player_get should return actual value
    printf("\nTest getting key = %d \n", key0);
    printf("key = %d, value = %d (should be %d)\n", key0, player_get(ctr1, key0), value);
    if (player_get(ctr1, key0) != value) {
        fprintf(stderr, "error: player_get failed");
    }

    // ***** Testing player_set *****
    // Cases where player_set should return false
    printf("\nTest setting on a null counter\n");
    if (player_set(NULL, 1, 1) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    printf("\nTest setting on a key < 0\n");
    if (player_set(ctr1, -10, 1) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    printf("\nTest setting count < 0\n");
    if (player_set(ctr1, 1, -10) != false) {
        fprintf(stderr, "error: player_set failed");
    }

    // Cases where player_set should return true
    printf("\nSetting a key = %d that already exists to 200\n", key0);
    if (player_set(ctr1, key0, 200) != true) {
        fprintf(stderr, "error: player_set failed");
    }
    printf("New value of key %d = %d (should be 200)\n", key0, player_get(ctr1, key0));

    printf("\nSetting keys that do not exist\n");
    for (int i = 0; i < 5; i++) {
        if (player_set(ctr1, keys[i], values[i]) != true) {
            fprintf(stderr, "error: player_set failed");
        }
        numKeysAdded++;
    }
    printf("\nMaking sure keys have been set right\n");
    for (int i = 0; i < 5; i++) {
        if (player_get(ctr1, keys[i]) != values[i]) {
            fprintf(stderr, "error: player_set failed");
        }
    }

    // ***** Testing player_print *****
    // Cases where player_print should print NULL
    printf("\nTest printing null counter < 0 (should print null)\n");
    player_print(ctr2, stdout);

    printf("\nTest printing null fp (should print nothing)\n");
    player_print(ctr1, NULL);

    // Cases where player_print should work
    printf("\nTest printing ctr1\n");
    player_print(ctr1, stdout);

    // ***** Testing player_iterate *****
    int numKeysinCounter = 0; // number of key stored in counter

    // Cases where player_iterate should do nothing
    printf("\n\nTest iterating over null counter (should do nothing)\n");
    player_iterate(NULL, stdout, keysCount);

    printf("\nTest iterating over null function (should do nothing)\n");
    player_iterate(ctr1, stdout, NULL);

    // Cases where player_iterate should work
    printf("\nCounting all keys added using counter\n");
    player_iterate(ctr1, &numKeysinCounter, keysCount);
    if (numKeysinCounter != numKeysAdded) {
        fprintf(stderr, "error: player_iterate failed");
    }
    printf("\nNumber of keys in counter (should be %d) = %d\n", numKeysAdded, numKeysinCounter);

    // ***** Testing player_delete *****
    // Cases where player_delete should ignore
    printf("\nDeleting null counter\n");
    player_delete(NULL);

    // Where counter_delete should work
    printf("\nDeleting the counter ctr1...\n");
    player_delete(ctr1);
    ctr1 = NULL; // setting pointer to NULL so no one accidentally uses it again

    printf("\nCounter after deletion\n");
    player_print(ctr1, stdout);

    printf("\n\n***Testing complete***\n");
    exit(0); //testing complete
}

/* count the non-null items in the counter
 * here we don't care what kind of item is in counter.
 * credit: CS50 bagtest file provided to us
 */
static void keysCount(void* arg, const int key, const int count)
{
    int* nitems = arg;
    if (nitems != NULL && count!=0) {
      (*nitems)++;
    }       
}