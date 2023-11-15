/* squareAndSum.c computes the sum of the squares
 *  of the values in an input file,
 *  whose name is given on the command-line.
 *
 * The program is written using typedef to declare
 *  Item as a generic type, currently double.
 *
 * Joel Adams, Fall 2023
 * for CS 374 (HPC) at Calvin University.
 * Alex Miller 
 * Calvin University 
 * Nov 15, 2023
 * Homework 04
 */

#include <stdio.h>      /* I/O */
#include <stdlib.h>     /* calloc(), exit(), etc. */
#include <mpi.h>
#include <vector> 
#include "OO_MPI_IO.h"          // ParallelReader


typedef double Item;

void readArray(char * fileName, Item ** a, int * n);
double arraySquareAndSum(std::vector<double> val, int numValues);
char* processCmdLineArgs(int argc, char** argv);
void check(FILE* fptr, char* fileName);
long getFileSize(FILE* fPtr);

int main(int argc, char * argv[])
{
  Item sum;
  Item totalSum;
  double startTime = 0.0;
  double readTime = 0.0;
  double totalTime = 0.0;
  double sumTime = 0.0;
  double sumStart = 0.0;
  const int MASTER = 0;
  int id = -1;
  int numProcs = -1;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);

  startTime = MPI_Wtime();

  char* fileName = processCmdLineArgs(argc, argv);

  ParallelReader<double> 
     reader(fileName, MPI_DOUBLE, id, numProcs);
  std::vector<double> vec;

  reader.readChunk(vec);

  long numItems = reader.getNumItemsInFile();  // integer division
  Item* arrayPtr = (Item*) malloc( numItems*sizeof(Item) );


  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: squareAndSum <inputFile>\n\n");
    exit(1);
  }
  readTime = MPI_Wtime() - startTime;
  sumStart = MPI_Wtime();

  sum = arraySquareAndSum(vec, numItems);


  sumTime = MPI_Wtime() - sumStart;
  totalTime = readTime + sumTime;
  MPI_Reduce(&sum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  reader.close();

if(id == MASTER) {
  printf("The sum of the squares of the values in the file '%s' is %g\n", argv[1], totalSum);
  printf("Read Time:\tSum Time:\tTotal Time:\n%f \t%f  \t%f\n", readTime, sumTime, totalTime);
}

  free(arrayPtr);
  MPI_Finalize();
  return 0;
}

/* utility to check and process the command line argument
 * @param: argc, an int
 * @param: argv, a char**
 * Precondition: argc == 2
 *            && argv[1] is the name of the input file.
 * @return: argv[1].
 */
char* processCmdLineArgs(int argc, char** argv) {
   if (argc != 2) {
      fprintf(stderr, "\n\n*** Usage: ./seqTextOut N\n\n");
      exit(1);
   }

   return argv[1];
}


/* arraySquareAndSum sums the squares of the values
 *  in an array of numeric Items.
 * Receive: a, a pointer to the head of an array of Items;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

Item arraySquareAndSum(std::vector<double> val, int numValues) {
  Item result = 0.0;

  for (int i = 0; i < numValues; ++i) {
    result += (val[i] * val[i]);
  }

  return result;
}