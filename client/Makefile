# Makefile for 'client'
#
# Ashna Kumar   3/2/22

S = ../support
L = ../libcs50
LIBS = -lncurses
LLIBS = $S/support.a $L/libcs50-given.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$L -I$S
CC = gcc
MAKE = make
# For memory leaks
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

.PHONY: all test valgrind clean

all: client

client: client.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

client.o: $S/message.h $S/log.h $L/mem.h

valgrind: client
	$(VALGRIND) ./client

clean:
	rm -rf *~ *.o *.dSYM
	rm -f client
	rm -f core