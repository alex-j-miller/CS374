PROG = circuitSatisfiabilityChunks
SRC = $(PROG).c
CC = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 
LFLAGS = -o $(PROG) -lm

$(PROG): $(PROG).c parallelLoopChunks.h
	$(CC) $(SRC) $(CFLAGS) $(LFLAGS)

clean:
	rm -f $(PROG) *.o *~ *#

