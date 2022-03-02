# Makefile for 'player' module
#
# Nitya Agarwala, Feb 2022

OBJS = playertest.o player.o ../lib/hashtable.o ../lib/file.o ../lib/mem.o ../set/set.o ../grid/grid.o
LIBS =

# uncomment the following to turn on verbose memory logging
#TESTING=-DMEMTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I../lib
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

playertest: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

playertest.o: player.h
player.o: player.h  ../lib/file.h ../lib/hashtable.h ../grid/grid.h
../lib/file.o: ../lib/file.h
../set/set.o: ../set/set.h

.PHONY: test valgrind clean

# expects a file `test.names` to exist; it can contain any text.
test: playertest

# expects a file `test.names` to exist; it can contain any text.
valgrind: playertest
	$(VALGRIND) ./playertest

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f playertest
	rm -f core