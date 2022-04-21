#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define FROM_MASTER 1
#define FROM_WORKER 2
#define MASTER 0

int RESULTADO;

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

  // printf("test: %d\n", mat[0][0]);

  return mat;
}

void printMatriz(int **m, int linhas, int colunas, int taskid, int val,
                 int peso) {
  printf("/***********************************/\n");
  printf("Taksid %d imprimiu a matriz: %d e peso %d\n", taskid, val, peso);
  for (int i = 0; i < linhas; i++) {
    for (int j = 0; j < colunas; j++) {
      printf("%d ", m[i][j]);
    }
    printf("\n");
  }
  printf("/***********************************/\n");
}

void free_matrix(int **mat) {
  free(mat[0]);
  free(mat);
}

int knapsack_serial(int MAXIMUM_CAPACITY, int wt[], int val[], int n) {
  // Matrix-based solution
  int **V = get_matrix(n + 1, MAXIMUM_CAPACITY + 1);

  // V Stores, for each (1 + i, j), the best profit for a knapscak
  // of capacity `j` considering every item k such that (0 <= k < i)
  int i, j;

  // evaluate item `i`
  for (i = 0; i < n; i++) {
    for (j = 1; j <= MAXIMUM_CAPACITY; j++) {
      if (wt[i] <= j) { // could put item in knapsack
        int previous_value = V[1 + i - 1][j];
        int replace_items = val[i] + V[1 + i - 1][j - wt[i]];

        // is it better to keep what we already got,
        // or is it better to swap whatever we have in the bag that weights up
        // to `j` and put item `i`?
        V[1 + i][j] = max(previous_value, replace_items);
      } else {
        // can't put item `i`
        V[1 + i][j] = V[1 + i - 1][j];
      }
    }
  }

  int retval = V[1 + n - 1][MAXIMUM_CAPACITY];

  // printMatriz(V, n + 1, MAXIMUM_CAPACITY + 1, 0, -1, -1);
  free_matrix(V);

  return retval;
}

int knapsack(int MAXIMUM_CAPACITY, int wt[], int val[], int n) {
  int size, taskid, numworkers, source, dest, rows, columns, offset, extras,
      linhasPerWorker, mtype, result;

  MPI_Status status;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

  int **v;
  if (taskid == 0) {
    rows = n + 1;
    columns = (MAXIMUM_CAPACITY + 1);

    // printf("test: w[%d] = %d\n", 3, wt[3]);
    // printf("test: w[%d] = %d\n", 4, wt[4]);

    // printf("Linhas-n: %d\n", n);
    // printf("Linhas: %d\n", rows);
    // printf("Colunas: %d\n", MAXIMUM_CAPACITY);
    // printf("Colunas (Capacidade): %d\n", columns);
    for (int i = 1; i < size; i++) {
      MPI_Send(&wt[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&val[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }

  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (taskid > 0) {
    wt = (int *)malloc((rows - 1) * sizeof(int));
    val = (int *)malloc((rows - 1) * sizeof(int));
    MPI_Recv(&wt[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(&val[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  }

  v = get_matrix(rows, columns);
  // printMatriz(v, rows, columns, taskid, -1, -1);

  // MPI_Scatter(v, n * MAXIMUM_CAPACITY / size, MPI_INT, v[taskid],
  //             n * MAXIMUM_CAPACITY / size, MPI_INT, 0, MPI_COMM_WORLD);

  // v Stores, for each (1 + i, j), the best profit for a knapscak
  // of capacity `j` considering every item k such that (0 <= k < i)
  int i, j;
  for (i = taskid; i < rows - 1; i += size) {
    for (j = 1; j < columns; j++) {
      int auxI;
      int auxJ;
      int elem;
      if (i > 0) {
        MPI_Recv(&auxJ, 1, MPI_INT, (i - 1) % size, i, MPI_COMM_WORLD, &status);
        MPI_Recv(&auxI, 1, MPI_INT, (i - 1) % size, i, MPI_COMM_WORLD, &status);
        MPI_Recv(&elem, 1, MPI_INT, (i - 1) % size, i, MPI_COMM_WORLD, &status);
        v[auxI][auxJ] = elem;
      }

      if (wt[i] <= j) {
        // printf("ESTOU AQUI taskdi: %d e linha: %d\n", taskid, i);
        int previous_value = v[i][j];
        int replace_items = val[i] + v[i][j - wt[i]];
        // printf("Linha anterior: %d linha para ser trocada: %d | indices: %d "
        // "e %d | taskdi: %d e linha: %d\n",
        // previous_value, replace_items, i + 1, j, taskid, i);
        v[1 + i][j] = max(previous_value, replace_items);
        // printf("PRINT FORA DO ELSE DO: taskid: 0");
        // printMatriz(v, rows, columns, taskid, val[i], wt[i]);
      } else {
        // printf("CAI AQUI NO 1° ELSE DO: taskdid: 0\n");
        v[1 + i][j] = v[i][j];
        // printMatriz(v, rows, columns, taskid, val[i], wt[i]);
      }

      {
        int auxI = i + 1;
        int auxJ = j;
        int elem = v[i + 1][j];
        MPI_Send(&auxJ, 1, MPI_INT, (i + 1) % size, i + 1, MPI_COMM_WORLD);
        MPI_Send(&auxI, 1, MPI_INT, (i + 1) % size, i + 1, MPI_COMM_WORLD);
        MPI_Send(&elem, 1, MPI_INT, (i + 1) % size, i + 1, MPI_COMM_WORLD);
        // printf("MENSAGEM ENVIADA PARA O: %d\n", (i + 1) % size);
      }
      // printMatriz(v, rows, columns, taskid, val[i], wt[i]);
      // }
    }
  }

  result = 0;
  MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (v[rows - 1][columns - 1] != 0) {
    // printf("taskid: %d\n", taskid);
    // printMatriz(v, rows, columns, taskid, val[i], wt[i]);
    result = v[rows - 1][columns - 1];
    // printf("TESTE DE RESULTADO: %d\n", result);
  }
  if (taskid == 0) {
    int result2 = knapsack_serial(MAXIMUM_CAPACITY, wt, val, n);
    printf("RESULTADO DA MOCHILA SERIAL: %d\n", result2);
  }

  // if (v[n][0] != 0) {
  //   printf("ESTOU AQUI PORRA\n");
  // }

  free_matrix(v);
  MPI_Finalize();
  return result;
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
  if (result > 0)
    printf("RESULTADO DA MOCHILA PARALELA: %d\n", result);

  return 0;
}
