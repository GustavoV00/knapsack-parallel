#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define FROM_MASTER 1
#define FROM_WORKER 2
#define MASTER 0

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

void print_matriz(int **m, int linhas, int colunas, int taskid, int val,
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

  printf("/**********MOCHILA SERIAL***********/\n");
  print_matriz(V, n + 1, MAXIMUM_CAPACITY + 1, 0, -1, -1);
  int retval = V[n][MAXIMUM_CAPACITY];
  printf("RESULTADO MOCHILA SERIAL: %d\n", retval);
  printf("/**********MOCHILA SERIAL***********/\n");

  free_matrix(V);

  return retval;
}

void envia_mensagens_iniciais(int rows, int size, int *wt, int *val,
                              int columns) {
  for (int i = 1; i < size; i++) {
    // MPI_Send(&rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    // MPI_Send(&columns, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    MPI_Send(&wt[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    MPI_Send(&val[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
  }
}

int *recebe_pesos(int *wt, int rows, MPI_Status status) {
  wt = (int *)malloc((rows - 1) * sizeof(int));
  MPI_Recv(&wt[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  return wt;
}

int *recebe_valores(int *val, int rows, MPI_Status status) {
  val = (int *)malloc((rows - 1) * sizeof(int));
  MPI_Recv(&val[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  return val;
}

void enviar_mensagem(int value, int size, int i) {
  int elem = value;

  MPI_Send(&elem, 1, MPI_INT, (i + 1) % size, i + 1, MPI_COMM_WORLD);
}

int **receber_mensagem(int **v, int j, int size, int i, MPI_Status status) {
  int elem;

  MPI_Recv(&elem, 1, MPI_INT, (i - 1) % size, i, MPI_COMM_WORLD, &status);
  v[(i - 1) % size][j] = elem;
  return v;
}

int **troca_valores_matriz(int **v, int size, int *wt, int *val, int j, int i,
                           int id) {
  int prev_i = id - 1;
  int atual_i = id;
  if (id == 0) {
    prev_i = size - 1;
  }

  int previous_value = v[prev_i][j];
  if (i == 0)
    previous_value = 0;

  int replace_items = val[i] + v[prev_i][j - wt[i]];

  v[atual_i][j] = max(previous_value, replace_items);
  return v;
}

int knapsack(int MAXIMUM_CAPACITY, int wt[], int val[], int n) {
  int size, rows, columns, taskid, result, maior;

  MPI_Status status;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  if (size == 1) {
    result = knapsack_serial(MAXIMUM_CAPACITY, wt, val, n);
    MPI_Finalize();
    return result;
  } else {

    int **v;
    if (taskid == 0) {
      rows = n + 1;
      columns = (MAXIMUM_CAPACITY + 1);
      envia_mensagens_iniciais(rows, size, wt, val, columns);
    }

    MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("ESTOU AQUI\n");
    if (taskid > 0) {
      wt = recebe_pesos(wt, rows, status);
      val = recebe_valores(val, rows, status);
    }

    v = get_matrix(size, columns);

    int i, j;
    for (i = taskid; i < rows - 1; i += size) {
      for (j = 1; j < columns; j++) {
        int id = i % size;

        if (taskid == 0) {
          if (wt[i] <= j) {
            if (i > 0)
              v = receber_mensagem(v, j, size, i, status);

            v = troca_valores_matriz(v, size, wt, val, j, i, id);

          } else {
            if (i > 0)
              v = receber_mensagem(v, j, size, i, status);
            v[id][j] = v[size - 1][j];
          }

          enviar_mensagem(v[id][j], size, i);
        }
        if (taskid > 0) {
          if (wt[i] <= j) {

            v = receber_mensagem(v, j, size, i, status);
            v = troca_valores_matriz(v, size, wt, val, j, i, id);

          } else {
            v = receber_mensagem(v, j, size, i, status);
            v[id][j] = v[id - 1][j];
          }

          enviar_mensagem(v[id][j], size, i);
        }
      }
      print_matriz(v, size, columns, taskid, val[i], wt[i]);
    }

    // if (taskid > 0) {
    //   int elem = v[taskid][columns - 1];
    //   MPI_Send(&elem, 1, MPI_INT, 0, taskid, MPI_COMM_WORLD);
    // }

    maior = v[taskid][columns - 1];
    // if (taskid == 0) {
    //   maior = v[taskid][columns - 1];
    //   int elem;
    //   for (int i = taskid + 1; i < size; i++) {
    //     MPI_Recv(&elem, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
    //     if (elem > maior)
    //       maior = elem;
    //   }
    // }

    free_matrix(v);
    MPI_Finalize();
    return maior;
  }
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
    printf("RESULTADO DA MOCHILA: %d\n", result);

  return 0;
}
