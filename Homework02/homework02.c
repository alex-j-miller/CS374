/* homework02.c
 * Uses the master-worker and message-passing patterns to pass a message around a ring of processes
 * 
 * Alex Miller
 * Calvin University
 * 11/01/2023
 * CS 374
 */

#include <stdio.h>
#include <mpi.h>

int masterTask(int, int);
int workerTask(int, int);

int main(int argc, char** argv) {
  int id = -1, numWorkers = -1, length = -1;
  char hostName[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);
  MPI_Get_processor_name (hostName, &length);

  if ( id == 0 ) {  // process 0 is the master 
    masterTask(id, numWorkers);
  } else {          // processes with ids > 0 are workers 
    workerTask(id, numWorkers);
  }

  MPI_Finalize();
  return 0;
}

int masterTask( int id, int numWorkers ){
/* masterTask() 
* Record the starting time.
* Create a message containing its rank.
* Send that message to the next process (i.e., with rank 1),
* Receive a message from the last worker process (i.e., with rank n-1), and
* Print the message received to the screen, along with the time required for the message to traverse the ring.
* 
* parameter id, the process id
* paramater numWorkers, the number of workers
*
*/

  double startTime = 0.0, totalTime = 0.0; startTime = MPI_Wtime(); // records the start time

  int sendValue = id;
  int receivedValue = -3;

  MPI_Status status;

  printf("Rank: %d \n", id); // Create a message containing its rank.
  printf("Number of Nodes: %d\n", numWorkers);

  while (receivedValue != id) {
    MPI_Send(&sendValue, 1, MPI_INT, id + 1, 1, MPI_COMM_WORLD); // Send that message to the next process (i.e., with rank 1)
    MPI_Recv(&receivedValue, 1, MPI_FLOAT, numWorkers - 1, 1, MPI_COMM_WORLD, &status); // Receive a message from the last worker process (i.e., with rank n-1)
    printf(" %d", receivedValue); // Print the message received to the screen
    sendValue = receivedValue;
  }

  totalTime = MPI_Wtime() - startTime;
  printf("\nTime: %f secs\n", totalTime); // Print the time required for the message to traverse the ring
  return 0;
}

int workerTask(int id, int numWorkers) {
/* workerTask() 
* Receive a message from the process "before" it in the ring (i.e., rank i-1),
* Create a new message consisting of the message just received, followed by a space, followed by their rank (e.g., i), and
* Send this new message on to the next process (i.e., with rank i+1), using the modulus operation to "wrap around" from the last worker to the master.
*
* parameter id, the process id
* paramater numWorkers, the number of workers.
*/
  int sendValue = id;
  int receivedValue = -2;

  MPI_Status status;

  while(receivedValue != id) {
    MPI_Recv(&receivedValue, 1, MPI_FLOAT, id - 1, 1, MPI_COMM_WORLD, &status); // recieve from previous worker
    MPI_Send(&sendValue, 1, MPI_INT, ( id + 1 ) % numWorkers, 1, MPI_COMM_WORLD); // send to next worker
    sendValue = receivedValue;
  }
  return 0;
}