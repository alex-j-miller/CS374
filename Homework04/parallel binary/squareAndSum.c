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
#include <string.h>
#include <math.h>

#define MASTER 0
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
  int chunkSize = -1;
  int* scatterArray = NULL;
  int* chunkSizeArray = NULL;
  int* offsetArray = NULL;
  int* chunkArray = NULL;
  // int* gatherArray = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
    double start_time = MPI_Wtime();

    char* fileName;
    FILE* filePtr;
    long numBytes;
    long numItems;
    Item* arrayPtr = NULL;

  if (id == MASTER) {
    fileName = processCmdLineArgs(argc, argv);
    filePtr = fopen(fileName, "rb");
    check(filePtr, fileName);

    numBytes = getFileSize(filePtr);
    numItems = numBytes / sizeof(Item);
    arrayPtr = (Item*) calloc( numItems , sizeof(Item) );

    fread(arrayPtr, sizeof(Item), numItems, filePtr);
    fclose(filePtr);
  }

  if (numProcesses > numItems) {
      if (id == MASTER) {
          printf("Please run with -np N less than or equal to %ld\n.", numItems);
      }
      MPI_Finalize();
      exit(0);
  }

  chunkSizeArray = (int*) malloc( numProcesses * sizeof(int) );    // chunkSizes
  offsetArray = (int*) malloc( numProcesses * sizeof(int) );       // offsets

  // find chunk size for part of processes
  int chunkSize1 = (int)ceil(((double)numItems) / numProcesses);
  int chunkSize2 = chunkSize1 - 1;
  int remainder = numItems % numProcesses;

  for (int i = 0; i < numProcesses; ++i) {
      if (remainder == 0 || (remainder != 0 && i < remainder)) {
          chunkSizeArray[i] = chunkSize1;
          offsetArray[i] = chunkSize1 * i;
      } else {
          chunkSizeArray[i] = chunkSize2;
          offsetArray[i] = (remainder * chunkSize1) + (chunkSize2 * (i - remainder));
      }
  }

  printf("BEFORE Scatter %d scatterArray", id);

  chunkSize = chunkSizeArray[id];
  chunkArray = (int*) malloc(chunkSize * sizeof(int));          // allocate chunk array
  
  printf("BEFORE Scatter %d chunkArray", id);
  MPI_Scatterv(scatterArray, chunkSizeArray, offsetArray, MPI_INT, chunkArray, chunkSize, MPI_INT, MASTER, MPI_COMM_WORLD); // scatter scatterArray into chunkArray
  printf("AFTER Scatter %d chunkArray", id);
  for (unsigned i = 0; i < chunkSize; ++i) { // compute using chunk
    chunkArray[i] *= 2;
  }
  printf("AFTER doubling %d chunkArray", id);
  if (id == 0) {                                           // master clean up                                                                           free(gatherArray); 
    free(scatterArray);
  }
  
  // int numSent = numItems / numProcesses;
  // double* aRcv = (double*) calloc( numSent , sizeof(double) );
  // MPI_Barrier(MPI_COMM_WORLD);
  // // MPI_Scatter(arrayPtr, numSent, MPI_DOUBLE, aRcv, numSent, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  // MPI_Scatter(sendBuffer, numSent, MPI_DOUBLE, receiveBuffer, numSent, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  // double sub_sum = arraySquareAndSum(aRcv, numSent);
  // double* sub_sums = NULL;

  // if (id == 0) {
  //   sub_sums = calloc(sizeof(double) , numProcesses);
  // }

  // // MPI_Gather(&sub_sum, 1, MPI_DOUBLE, sub_sums, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  // MPI_Gather(&sub_sum, 1, MPI_DOUBLE, sub_sums, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // double sum;
  // if (id == 0) {
  //   sum = arraySquareAndSum(sub_sums, numProcesses);
  // }

  // printf("The sum of the squares of the values in the file '%s' is %g\n\tTotal Time: %f\n", argv[1], sum, MPI_Wtime() - start_time);
  
  // if (id == 0) {
  //   free(sendBuffer);
  //   free(receiveBuffer);
  //   free(arrayPtr);
  //   free(sub_sums);
  // }
  printf("Time: %f", MPI_Wtime() - start_time);
  MPI_Finalize();
  return 0;
}

/* readArray fills an array with Item values from a file.
 * Receive: fileName, a char*,
 *          a, the address of a pointer to an Item array,
 *          n, the address of an int.
 * PRE: fileName contains N, followed by N double values.
 * POST: a points to a dynamically allocated array
 *        containing the N values from fileName
 *        and n == N.
 */

void readArray(char * fileName, Item** a, int * n) {
  int count, howMany;
  Item* tempA;
  FILE * fin;

  fin = fopen(fileName, "rb");
  if (fin == NULL) {
    fprintf(stderr, "\n*** Unable to open input file '%s'\n\n", fileName);
    exit(1);
  }

  // fscanf(fin, "%d", &howMany);
  fread(&howMany, sizeof(howMany), 1, fin);
  tempA = calloc(howMany, sizeof(Item));
  if (tempA == NULL) {
    fprintf(stderr, "\n*** Unable to allocate %d-length array", howMany);
    exit(1);
  }

  for (count = 0; count < howMany; count++)
   fscanf(fin, "%lf", &tempA[count]);

  fclose(fin);

  *n = howMany;
  *a = tempA;
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

