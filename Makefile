# Makefile for nuggets-add-drop
#
# Vico Lee - 3 March 2022

L = libcs50
SERVEROBJS = server.c
CLIENTOBJS = client.c 
# LIBS = /libcs50/libcs50.a /support/support.a grid/grid.a player/player.a

# uncomment the following to turn on file saving, finding logs.
# TESTING=-DAPPTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I/libcs50 -I/support -I/player -I/grid
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all
.PHONY: all clean


server: $(SERVEROBJS) $(LIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

client: $(CLIENTOBJS) $(LIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
	
############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
	(cd $L && if [ -r set.c ]; then make $L.a; else cp $L-given.a $L.a; fi)
	make -C grid
	make -C player
	make -C support
	server client



############### TAGS for emacs users ##########
TAGS:  Makefile */Makefile */*.c */*.h */*.md */*.sh
	etags $^

test:
#	bash -v testing.sh &> testing.out

# valgrind: server
# 	$(VALGRIND) 

############## clean  ##########
clean:
	rm -f *~
	rm -f TAGS
	rm -f core
	rm -f *~ ~.o
	make -C libcs50 clean
	make -C support clean
	make -C grid clean
	make -C player clean

