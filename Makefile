S = support
L = libcs50
G = grid
P = player
LIBS = -lncurses -lm
LLIBS = $S/support.a $P/player.a $G/grid.a $L/libcs50-given.a

# add -DAPPEST for functional tracking report
# add -DMEMTEST for memory tracking report
# (and run `make clean; make` whenever you change this)
FLAGS = # -DAPPTEST # -DMEMTEST

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I./$L -I./$S -I./$P -I./$G
CC = gcc
MAKE = make

.PHONY: all clean

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
	(cd $L && if [ -r set.c ]; then make $L.a; else cp $L-given.a $L.a; fi)
	make -C support
	make -C grid
	make -C player
	make server
	make client

########### server ##################

server: server.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

########### client ##################

client: client.o $(LLIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

########### miniclient ##################

miniclient: miniclient.o message.o log.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# querier source dependencies
server.o: $S/message.h $S/log.h $L/mem.h $L/file.h $P/player.h $G/grid.h
client.o: $S/message.h $S/log.h $L/mem.h
miniclient.o: message.h
message.o: message.h
log.o: log.h


############### TAGS for emacs users ##########
TAGS:  Makefile */Makefile */*.c */*.h */*.md */*.sh
	etags $^

############## clean  ##########
clean:
	rm -f *~ *.o
	rm -f TAGS
	rm -f server
	rm -f client
	make -C support clean
	make -C grid clean
	make -C player clean