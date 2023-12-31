/* squareAndSum.c computes the sum of the squares
 *  of the values in an input file,
 *  whose name is given on the command-line.
 *
 * The program is written using typedef to declare
 *  Item as a generic type, currently double.
 *
 * Joel Adams, Fall 2023
 * for CS 374 (HPC) at Calvin University.
 * 
 * Alex Miller
 * Calvin University
 * 6 Dec 2023
 * 
 */

#include <stdio.h>      /* I/O */
#include <stdlib.h>     /* calloc(), exit(), etc. */
#include <mpi.h>

typedef double Item;

void readArray(char * fileName, Item ** a, int * n);
double arraySquareAndSum(Item* a, int numValues);

char* processCmdLineArgs(int argc, char** argv);
void check(FILE* fptr, char* fileName);
long getFileSize(FILE* fPtr);

int main(int argc, char * argv[])
{
  int numProcesses = -1;
  int id = -1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  double start_time = MPI_Wtime();

  char* fileName = processCmdLineArgs(argc, argv);

  FILE* filePtr = fopen(fileName, "rb");
  check(filePtr, fileName);

  long numBytes = getFileSize(filePtr);
  long numItems = numBytes / sizeof(Item);
  Item* arrayPtr = (Item*) malloc( numItems * sizeof(Item) );

  fread(arrayPtr, sizeof(Item), numItems, filePtr);

  fclose(filePtr);

  double afterArrTime = MPI_Wtime() - start_time;
  double sum_start = MPI_Wtime();
  double sum = arraySquareAndSum(arrayPtr, numItems);

  double sumTime = MPI_Wtime() - sum_start;

  printf("The sum of the squares of the values in the file '%s' is %g\nTimes: %f, %f, %f\n", argv[1], sum, afterArrTime, sumTime, afterArrTime + sumTime);
  return 0;
}

/* arraySquareAndSum sums the squares of the values
 *  in an array of numeric Items.
 * Receive: a, a pointer to the head of an array of Items;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

Item arraySquareAndSum(Item* a, int numValues) {
  Item result = 0.0;

  for (int i = 0; i < numValues; ++i) {
    result += (a[i] * a[i]);
  }

  return result;
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

/* utility to check that opening the file succeeded.
 * @param: fPtr, a FILE*
 * @param: fileName, a char*
 * Precondition: fPtr holds the result of a call to fopen()
 *            && fileName is the name of the arg to fopen().
 * Postcondition: if fPtr == NULL, an error message has been displayed.
 */
void check(FILE* fPtr, char* fileName) {
   if (fPtr == NULL) {
      fprintf(stderr, "\n*** Unable to open file '%s'\n\n",
                                fileName);
   }
}

/* utility to calculate the size of an open file in bytes
 * @param: fPtr, a FILE*
 * Precondition: fPtr is to a file that has just been opened.
 * @return: the number of bytes in the file.
 */
long getFileSize(FILE* fPtr) {
    // seek to the end of the file.
    fseek(fPtr, 0L, SEEK_END);

    // calculate the size of the file
    long res = ftell(fPtr);

    // return the file ptr to the beginng
    fseek(fPtr, 0L, SEEK_SET);

    return res;
}

