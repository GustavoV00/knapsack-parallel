#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define FROM_MASTER 1
#define FROM_WORKER 2
#define MASTER 0

int max(int a, int b);
int **get_matrix(int rows, int columns);
void leArquivosDeEntrada(int *pesos, int *valores, int itens);
int encontraValorMaximo(int capacidade, int itens, int *pesos, int *valores);
void print_matriz(int **m, int linhas, int colunas, int taskid, int val,
                  int peso);
void free_matrix(int **mat);

void imprimeInformacoesDaMochila(int capacidade, int quantidadeItens,
                                 int *valores, int *pesos);
void envia_mensagens_iniciais(int rows, int size, int *wt, int *val);
int *recebe_pesos(int *wt, int rows, MPI_Status status);
int *recebe_valores(int *val, int rows, MPI_Status status);
int **alocaMatriz(int subMatrizesLinhas, int cols, int taskid, int size,
                  int subMatrizesLinhasExtras, int **matriz, int itens,
                  int capacidade);
int knapsack_serial(int MAXIMUM_CAPACITY, int wt[], int val[], int n);
int **resolvedorBloco(int **matriz, int i, int j, int *valores, int *pesos,
                      int iGlobal);
int **recebeLinha(int **matriz, int recvRank, int k, int cols,
                  MPI_Status status);
int **enviaLinha(int **matriz, int sendRank, int k, int cols,
                 int subMatrizesLinhas);
int knapsack_parallel(int capacidade, int *pesos, int *valores, int itens,
                      int cols, int taskid, int size, MPI_Status status);

// diver program m to test above function
int main() {

  MPI_Init(NULL, NULL);
  MPI_Status status;

  int taskid;
  int size;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  int rows, cols;
  int itens, capacidade;
  int *valores, *pesos;

  if (taskid == 0) {

    scanf("%d %d", &itens, &capacidade);
    valores = (int *)calloc(itens, sizeof(int));
    pesos = (int *)calloc(itens, sizeof(int));

    rows = itens + 1;
    cols = capacidade + 1;
    leArquivosDeEntrada(pesos, valores, itens);
  }

  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&itens, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&capacidade, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (taskid == 0) {
    envia_mensagens_iniciais(rows, size, pesos, valores);
  }

  if (taskid > 0) {
    pesos = recebe_pesos(pesos, rows, status);
    valores = recebe_valores(valores, rows, status);
  }
  // imprimeInformacoesDaMochila(capacidade, itens, valores, pesos);

  if (size == 1) {
    int result = knapsack_serial(capacidade, pesos, valores, itens);
    printf("result: %d\n", result);

  } else {

    // printf("\n");
    int result = knapsack_parallel(capacidade, pesos, valores, itens, cols,
                                   taskid, size, status);
    if (result >= 0)
      printf("result: %d\n", result);
  }

  MPI_Finalize();
  return 0;
}

int knapsack_parallel(int capacidade, int *pesos, int *valores, int itens,
                      int cols, int taskid, int size, MPI_Status status) {

  // Matriz para memoization
  int recvRank = (taskid - 1) % size; // rank to receive data
  int sendRank = (taskid + 1) % size; // rank to send data
  int **matriz = NULL;
  int subMatrizesLinhas = itens / size;
  int subMatrizesLinhasExtras = 0;

  int subMatrizesColunas = capacidade / size;
  int subMatrizesColunasExtras = 0;

  int iniCol;

  matriz = alocaMatriz(subMatrizesLinhas, cols, taskid, size,
                       subMatrizesLinhasExtras, matriz, itens, capacidade);

  // printf("%d e %d\n", itens, capacidade);
  int result = -1;
  int tags = 0;
  for (int k = 0; k < size; k++) {
    int iGlobal;
    iGlobal = (subMatrizesLinhas * taskid);

    if (k == size - 1)
      subMatrizesColunasExtras = capacidade % size;

    if (taskid > 0) {
      matriz = recebeLinha(matriz, recvRank, k, cols, status);
      tags += 1;
    }

    tags += 1;
    if (taskid == size - 1)
      subMatrizesLinhasExtras = itens % size;

    for (int i = 0; i < subMatrizesLinhas + subMatrizesLinhasExtras; i++) {

      iniCol = (k * subMatrizesColunas) + 1;
      // printf("iniCol: %d\n", iniCol);
      int bound = (subMatrizesColunas * (k + 1)) + subMatrizesColunasExtras;
      for (int j = iniCol; j <= bound; j++) {
        resolvedorBloco(matriz, i, j, valores, pesos, iGlobal);
      }
      if (taskid == size - 1)
        result = matriz[i + 1][bound];

      iGlobal += 1;
      // if (taskid == 0)
      //   print_matriz(matriz, subMatrizesLinhas + 1, cols, taskid, -1, -1);

      // else if (taskid == size - 1)
      //   print_matriz(matriz, subMatrizesLinhas + subMatrizesLinhasExtras + 1,
      //                cols, taskid, -1, -1);
      // else
      //   print_matriz(matriz, subMatrizesLinhas + 2, cols, taskid, -1, -1);
    }

    if (taskid < size - 1) {
      matriz = enviaLinha(matriz, sendRank, k, cols, subMatrizesLinhas);
    }
  }

  // MPI_Barrier(MPI_COMM_WORLD);
  // if (taskid == 0)
  //   print_matriz(matriz, subMatrizesLinhas + 1, cols, taskid, -1, -1);

  // else if (taskid == size - 1)
  //   print_matriz(matriz, subMatrizesLinhas + subMatrizesLinhasExtras + 1,
  //   cols,
  //                taskid, -1, -1);
  // else
  //   print_matriz(matriz, subMatrizesLinhas + 1, cols, taskid, -1, -1);

  // int result = -1;
  // if (taskid == size - 1) {
  //   result = matriz[subMatrizesLinhas + 2][cols];
  // }
  free_matrix(matriz);

  return result;
}

int **resolvedorBloco(int **matriz, int i, int j, int *valores, int *pesos,
                      int iGlobal) {

  if (pesos[iGlobal] <= j) {
    // printf("iGloba: %d | valores: %d | anterior: %d | result: %d | indices:
    // %d "
    //        "e %d | taskid: %d\n",
    // iGlobal, valores[iGlobal], matriz[i][j - pesos[iGlobal]],
    // valores[iGlobal] + matriz[i][j - pesos[iGlobal]], iGlobal, j,
    // taskid);

    int previous_value = matriz[i][j];
    int replace_items = valores[iGlobal] + matriz[i][j - pesos[iGlobal]];

    matriz[i + 1][j] = max(previous_value, replace_items);
  } else {
    // printf("PQ? : %d | indices: %d e %d\n", matriz[i][j], iGlobal, j);
    matriz[i + 1][j] = matriz[i][j];
  }
  return matriz;
}

int **enviaLinha(int **matriz, int sendRank, int k, int cols,
                 int subMatrizesLinhas) {

  // printf("ESTOU AQUI ENVIADO PARA: %d\n", sendRank);
  // printf("\n\n\n\n\n");

  // for (int u = 0; u <= cols; u++) {
  //   printf("%d ", matriz[subMatrizesLinhas - 1][u]);
  // }

  // printf("\n\n\n\n\n");
  MPI_Send(&matriz[subMatrizesLinhas][0], cols, MPI_INT, sendRank, k,
           MPI_COMM_WORLD);
  return matriz;
}

int **recebeLinha(int **matriz, int recvRank, int k, int cols,
                  MPI_Status status) {
  // printf("Taskid: %d está recebendo de: %d\n", taskid, recvRank);
  MPI_Recv(&matriz[0][0], cols, MPI_INT, recvRank, k, MPI_COMM_WORLD, &status);
  // printf("\n\n\n\n\n");

  // for (int u = 0; u <= cols; u++) {
  //   printf("%d ", matriz[0][u]);
  // }
  // printf("\n\n\n\n\n");
  return matriz;
}

int **alocaMatriz(int subMatrizesLinhas, int cols, int taskid, int size,
                  int subMatrizesLinhasExtras, int **matriz, int itens,
                  int capacidade) {

  int subMatrizesColsExtras = capacidade % size;
  if (taskid == 0)
    matriz = get_matrix(subMatrizesLinhas + 1, cols + subMatrizesColsExtras);

  else if (taskid == size - 1) {
    subMatrizesLinhasExtras = itens % size;
    // printf("subMatrizesLinhasExtras: %d\n", subMatrizesLinhasExtras);
    matriz = get_matrix(subMatrizesLinhas + subMatrizesLinhasExtras + 2,
                        cols + subMatrizesColsExtras);
  } else {
    matriz = get_matrix(subMatrizesLinhas + 2, cols + subMatrizesColsExtras);
  }

  return matriz;
}

int getTamanhoColunasSubMatrizes(int capacidade, int processadores) {
  return capacidade / processadores;
}

void imprimeInformacoesDaMochila(int capacidade, int quantidadeItens,
                                 int *valores, int *pesos) {
  printf("\n");
  printf("Capacidade da mochila: %d\n", capacidade);
  printf("Quantidade de itens: %d\n", quantidadeItens);
  printf("\n");

  for (int i = 0; i < quantidadeItens; i++) {
    printf("%d ", valores[i]);
  }
  printf("\n");

  for (int i = 0; i < quantidadeItens; i++) {
    printf("%d ", pesos[i]);
  }
  printf("\n");
}

void leArquivosDeEntrada(int *pesos, int *valores, int itens) {
  int i;
  for (i = 0; i < itens; ++i) {
    scanf("%d %d", &(valores[i]), &(pesos[i]));
  }
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
  printf("/***********************************/\n\n");
}

void free_matrix(int **mat) {
  free(mat[0]);
  free(mat);
}

// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

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

void envia_mensagens_iniciais(int rows, int size, int *wt, int *val) {
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

  // print_matriz(V, n + 1, MAXIMUM_CAPACITY + 1, 0, -1, -1);
  int retval = V[1 + n - 1][MAXIMUM_CAPACITY];

  free_matrix(V);

  return retval;
}
