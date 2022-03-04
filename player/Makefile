# Makefile for 'player' module
#
# Nitya Agarwala, Feb 2022

OBJS = player.o ../grid/grid.o
TOBJS = playertest.o
LIBS = -lm
LLIBS = ../libcs50/libcs50-given.a 
LIB = player.a

# uncomment the following to turn on verbose memory logging
#TESTING=-DMEMTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I../libcs50 -I../grid
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)


# player.o: player.h ../libcs50/set.h ../libcs50/counters.h ../libcs50/hashtable.h ../libcs50/file.h ../libcs50/mem.h ../grid/grid.h 
player.o: player.h
# playertest.o: player.h ../libcs50/set.h ../libcs50/counters.h ../libcs50/hashtable.h ../libcs50/file.h ../libcs50/mem.h ../grid/grid.h
# ../grid/grid.o: ../grid/grid.h
playertest.o: player.h

playertest:  $(TOBJS) $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(TOBJS) $(OBJS) $(LLIBS) $(LIBS) -o $@


all: playertest

.PHONY: test valgrind clean

# expects a file `test.names` to exist; it can contain any text.
test: playertest
	./playertest

# expects a file `test.names` to exist; it can contain any text.
valgrind: playertest
	$(VALGRIND) ./playertest

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f playertest
	rm -f player
	rm -f core