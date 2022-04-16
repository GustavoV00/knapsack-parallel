#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

// Aloca uma matriz de dados;
int **get_matrix(int rows, int columns) {
  int **mat;
  int i;

  // for each line
  mat = (int **)calloc(rows, sizeof(int *));

  mat[0] = (int *)calloc(rows * columns, sizeof(int));

  // set up pointers to rows
  for (i = 1; i < rows; i++)
    mat[i] = mat[0] + i * columns;

  return mat;
}

void free_matrix(int **mat) {
  free(mat[0]);
  free(mat);
}

int knapsack(int MAXIMUM_CAPACITY, int wt[], int val[], int n) {
  // Starta um comunicador mpi
  MPI_Init(NULL, NULL);

  // Tamanho do comunicador.
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // processor id, but its generally called rank.
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int **V;
  if (rank == 0) {
    int test = 0;
    // Matrix-based solution (Alocate and starts its values with 0)
    V = get_matrix(n + 1, MAXIMUM_CAPACITY + 1);
    for (int i = 1; i < size; i++) {
      MPI_Send(&V[0][0], (n ) * (MAXIMUM_CAPACITY ), MPI_INT, i, 10 + i,
               MPI_COMM_WORLD);

      printf("FOI ENVIADO\n");
//      MPI_Recv(&test, 1, MPI_INT, i, 10 + i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//      printf("Informação recebida do %d, com valor %d\n", i, test);
    }
  } else {
    int enviado = 1;
    for (int i = 1; i < size; i++) {
      MPI_Recv(&V[0][0], (n ) * (MAXIMUM_CAPACITY ), MPI_INT, 0, 10 + i,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

//    MPI_Send(&enviado, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
  }

  // V Stores, for each (1 + i, j), the best profit for a knapscak
  // of capacity `j` considering every item k such that (0 <= k < i)
  // int i, j;

  // // evaluate item `i`
  // for (i = 0; i < n; i++) {
  //   for (j = 1; j <= MAXIMUM_CAPACITY; j++) {
  //     if (wt[i] <= j) { // could put item in knapsack
  //       int previous_value = V[1 + i - 1][j];
  //       int replace_items = val[i] + V[1 + i - 1][j - wt[i]];

  //       // is it better to keep what we already got,
  //       // or is it better to swap whatever we have in the bag that weights
  //       // up
  //       // to `j` and put item `i`?
  //       V[1 + i][j] = max(previous_value, replace_items);
  //     } else {
  //       // can't put item `i`
  //       V[1 + i][j] = V[1 + i - 1][j];
  //     }
  //   }
  // }

  // int retval = V[1 + n - 1][MAXIMUM_CAPACITY];

  if (rank == 0)
    free_matrix(V);

  MPI_Finalize();
  return 0;
}

// Driver program to test above function
int main() {
  int n, W;

  scanf("%d %d", &n, &W);
  int *val = (int *)calloc(n, sizeof(int));
  int *wt = (int *)calloc(n, sizeof(int));

  int i;
  for (i = 0; i < n; ++i) {
    scanf("%d %d", &(val[i]), &(wt[i]));
  }

  int result = knapsack(W, wt, val, n);
  if (result != -1)
    printf("resultado: %d\n", result);

  return 0;
}

/*
SOME NOTES

Job, Process, Task:
  Job is work that needs to be done.
  A task is a piece of work that needs to be done.
  The process is a series of actions that is done for a particular purpose.
  Job and task define the work to be done, whereas process defines the way the
  work can be done or how the work should be done.

MPI_Init(argc, argv):
  Cria todas as váriaveis, o comunicador é formado é formado em volta de todos
  esses processos que foram spawned (criados ?), e ranks únicos são associados a
  cada processo.

MPI-Comm_size():
  return the size of the communicator
  MPI_COMM_WORLD:
    encloses all of the processes in the job, so this call should
    return the amount of processes that were requested for the job.

MPI_Comm_rank():
   returns the rank of a process in a communicator.

MPI_Get_processor_name():
  obtains the actual name of the processor on which the process is executing.

MPI_Finalize():
  is used to clean up the MPI environment. No more MPI calls can be made after
  this one.


MPI_Wtime =

MPI_Send(void *buf, int count, MPI_Datatype datatype,
int dest, int tag, MPI_Comm comm)

MPI_Recv(void *buf, int count, MPI_Datatype datatype,
int source, int tag, MPI_Comm comm, MPI_Status *status)


Um HelloWorld no estilo MPI.
  if (!mpi_iniciou) {
    // Numero de size
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Pega a ordem (rank) dos size.
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);
  }

  MPI_Finalize();

Exemplo de send e recv:
  int token;
  if (world_rank != 0) {
      MPI_Recv(&token, 1, MPI_INT, world_rank - 1, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Process %d received token %d from process %d\n",
             world_rank, token, world_rank - 1);
  } else {
      // Set the token's value if you are process 0
      token = -1;
  }
  MPI_Send(&token, 1, MPI_INT, (world_rank + 1) % world_size,
           0, MPI_COMM_WORLD);

  // Now process 0 can receive from the last process.
  if (world_rank == 0) {
      MPI_Recv(&token, 1, MPI_INT, world_size - 1, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Process %d received token %d from process %d\n",
             world_rank, token, world_size - 1);
  }

Exemplo de Mpi_status struct:
  const int MAX_NUMBERS = 100;
  int numbers[MAX_NUMBERS];
  int number_amount;
  if (world_rank == 0) {
      // Pick a random amount of integers to send to process one
      srand(time(NULL));
      number_amount = (rand() / (float)RAND_MAX) * MAX_NUMBERS;

      // Send the amount of integers to process one
      MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
      printf("0 sent %d numbers to 1\n", number_amount);
  } else if (world_rank == 1) {
      MPI_Status status;
      // Receive at most MAX_NUMBERS from process zero
      MPI_Recv(numbers, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD,
               &status);

      // After receiving the message, check the status to determine
      // how many numbers were actually received
      MPI_Get_count(&status, MPI_INT, &number_amount);

      // Print off the amount of numbers, and also print additional
      // information in the status object
      printf("1 received %d numbers from 0. Message source = %d, "
             "tag = %d\n",
             number_amount, status.MPI_SOURCE, status.MPI_TAG);
  }

  int number_amount;
  if (world_rank == 0) {
      const int MAX_NUMBERS = 100;
      int numbers[MAX_NUMBERS];
      // Pick a random amount of integers to send to process one
      srand(time(NULL));
      number_amount = (rand() / (float)RAND_MAX) * MAX_NUMBERS;

      // Send the random amount of integers to process one
      MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
      printf("0 sent %d numbers to 1\n", number_amount);
  } else if (world_rank == 1) {
      MPI_Status status;
      // Probe for an incoming message from process zero
      MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

      // When probe returns, the status object has the size and other
      // attributes of the incoming message. Get the message size
      MPI_Get_count(&status, MPI_INT, &number_amount);

      // Allocate a buffer to hold the incoming numbers
      int* number_buf = (int*)malloc(sizeof(int) * number_amount);

      // Now receive the message with the allocated buffer
      MPI_Recv(number_buf, number_amount, MPI_INT, 0, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("1 dynamically received %d numbers from 0.\n",
             number_amount);
      free(number_buf);
  }

*/
