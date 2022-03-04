# Makefile for nuggets-add-drop
#
# Vico Lee - 3 March 2022

L = libcs50
LIBS = -lncurses -lm
S = support
L = libcs50
P = player
G = grid
SERVEROBJS = server.o
CLIENTOBJS = client.o
LLIBS = $S/support.a $P/player.a $G/grid.a $L/libcs50-given.a

# uncomment the following to turn on file saving, finding logs.
# TESTING=-DAPPTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I./$L -I./$S -I./$P -I./$G
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all
.PHONY: all clean

server: $(SERVEROBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

client: $(CLIENTOBJS) $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

server.o: $S/message.h $S/log.h $L/mem.h $L/file.h $P/player.h $G/grid.h
client.o: $S/message.h $S/log.h $L/mem.h

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
# (cd $L && if [ -r set.c ]; then make $L.a; else cp $L-given.a $L.a; fi)
	make -C support
	make -C grid
	make -C player
	make server 
	make client
	
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
#make -C support clean
	make -C player clean
	make -C grid clean

