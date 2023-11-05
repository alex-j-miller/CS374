PROG    = homework02
CC      = mpicc
CFLAGS  = -Wall -ansi -pedantic -std=c99
LFLAGS1 = -o $(PROG1) -lm

$(PROG): $(PROG).c
	module load openmpi-2.0/gcc; \
	$(CC) $(CFLAGS) $(PROG).c $(LFLAGS)

clean:
	rm -f $(PROGS) a.out *~ *# *.o *.out slurm*