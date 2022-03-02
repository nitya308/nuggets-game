# Makefile for grid module
#Matthew Timofeev
#
#

OBJS = grid.o 
TOBJS = gridtest.o
LIBS =
LLIBS = ../libcs50/libcs50.a 


CFLAGS = -Wall -pedantic -std=c11 -ggdb $(LOGGING) -I../lib
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

grid: $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(OBJS) $(LLIBS) $(LIBS) -o $@

grid.o: grid.h ../libcs50/set.h ../libcs50/counters.h ../libcs50/file.h ../libcs50/mem.h

gridtest:  $(OBJS) $(LLIBS)
	$(CC) $(CFLAGS) $(TOBJS) $(LLIBS) $(LIBS) -o $@

gridtest.o: grid.h ../libcs50/set.h ../libcs50/counters.h ../libcs50/file.h ../libcs50/mem.h


.PHONY: test valgrind clean


test: 
	make -C ../libcs50
	grid
	./gridtest


valgrind: grid
	$(VALGRIND) ./gridtest

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f gridtest
	rm -f grid
	rm -f core
