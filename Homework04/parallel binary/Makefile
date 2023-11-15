PROG   = parSquareSum
SRC    = $(PROG).cpp
CC     = mpic++
CFLAGS = -Wall -ansi -pedantic -std=c++11
LFLAGS = -o $(PROG) 

$(PROG): $(SRC) OO_MPI_IO.h
	$(CC) $(CFLAGS) $(SRC) $(LFLAGS)

clean:
	rm -f $(PROG) a.out *~ *.o 

cleanBin:
	rm -f *.bin /scratch/adams/*.bin

