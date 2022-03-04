# Makefile for grid module
#Matthew Timofeev
#
#

OBJS = grid.o
TOBJS = gridtest.o
LIBS = -lm
LLIBS = ../libcs50/libcs50-given.a 
LIB = grid.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(LOGGING) -I../libcs50
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind -v --leak-check=full --show-leak-kinds=all

grid.o: ../libcs50/set.h ../libcs50/counters.h ../libcs50/hashtable.h grid.h ../libcs50/file.h ../libcs50/mem.h 
gridtest.o: grid.h ../libcs50/set.h ../libcs50/counters.h ../libcs50/hashtable.h ../libcs50/file.h ../libcs50/mem.h

gridtest:  $(TOBJS) $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(TOBJS) $(OBJS) $(LLIBS) $(LIBS) -o $@

# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)

all: gridtest

.PHONY: test valgrind clean


test: gridtest
	./gridtest


valgrind: gridtest
	$(VALGRIND) ./gridtest

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f gridtest
	rm -f grid
	rm -f core
