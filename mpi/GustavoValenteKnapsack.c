// Gustavo Valente Nunes
// GRR 20182557

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
float timedifference_msec(struct timeval t0, struct timeval t1);
int knapsack_parallel(int capacidade, int *pesos, int *valores, int itens,
                      int cols, int taskid, int size, MPI_Status status,
                      struct timeval ini, struct timeval fim,
                      float *serialTime);

// diver program m to test above function
int main() {

  MPI_Init(NULL, NULL);
  MPI_Status status;

  int taskid;
  int size;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  float serialTime = 0;
  struct timeval t0;
  struct timeval t1;
  struct timeval serialIni;
  struct timeval serialFim;

  int rows, cols;
  int itens, capacidade;
  int *valores, *pesos;
  if (taskid == size - 1)
    gettimeofday(&t0, 0);

  if (taskid == 0) {
    gettimeofday(&serialIni, 0);

    scanf("%d %d", &itens, &capacidade);
    valores = (int *)calloc(itens, sizeof(int));
    pesos = (int *)calloc(itens, sizeof(int));

    rows = itens + 1;
    cols = capacidade + 1;
    leArquivosDeEntrada(pesos, valores, itens);
    gettimeofday(&serialFim, 0);
    serialTime += timedifference_msec(serialIni, serialFim);
  }

  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&itens, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&capacidade, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (taskid == 0) {
    gettimeofday(&serialIni, 0);
    envia_mensagens_iniciais(rows, size, pesos, valores);
    gettimeofday(&serialFim, 0);
    serialTime += timedifference_msec(serialIni, serialFim);
  }

  if (taskid > 0) {
    pesos = NULL;
    pesos = recebe_pesos(pesos, rows, status);
    valores = NULL;
    valores = recebe_valores(valores, rows, status);
  }
  // imprimeInformacoesDaMochila(capacidade, itens, valores, pesos);

  if (size == 1) {
    int result = knapsack_serial(capacidade, pesos, valores, itens);
    printf("result: %d\n", result);

  } else {
    gettimeofday(&serialIni, 0);
    gettimeofday(&serialFim, 0);
    serialTime += timedifference_msec(serialIni, serialFim);
    int result =
        knapsack_parallel(capacidade, pesos, valores, itens, cols, taskid, size,
                          status, serialIni, serialFim, &serialTime);
    if (result >= 0)
      printf("%d\n", result);
  }

  if (taskid == size - 1) {
    gettimeofday(&serialIni, 0);
    float elapsed;

    gettimeofday(&t1, 0);

    elapsed = timedifference_msec(t0, t1);
    gettimeofday(&serialFim, 0);
    serialTime += timedifference_msec(serialIni, serialFim);
    // printf("serialTime: %f\n", serialTime);
    // printf("time: %f\n", elapsed);
  }

  MPI_Finalize();
  return 0;
}

int knapsack_parallel(int capacidade, int *pesos, int *valores, int itens,
                      int cols, int taskid, int size, MPI_Status status,
                      struct timeval ini, struct timeval fim,
                      float *serialTime) {

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
      gettimeofday(&ini, 0);
      for (int j = iniCol; j <= bound; j++) {
        resolvedorBloco(matriz, i, j, valores, pesos, iGlobal);
      }
      gettimeofday(&fim, 0);
      *serialTime += timedifference_msec(ini, fim);

      if (taskid == size - 1)
        result = matriz[i + 1][bound];

      iGlobal += 1;
    }

    if (taskid < size - 1) {
      matriz = enviaLinha(matriz, sendRank, k, cols, subMatrizesLinhas);
    }
  }
  free_matrix(matriz);

  return result;
}

int **resolvedorBloco(int **matriz, int i, int j, int *valores, int *pesos,
                      int iGlobal) {

  if (pesos[iGlobal] <= j) {
    int previous_value = matriz[i][j];
    int replace_items = valores[iGlobal] + matriz[i][j - pesos[iGlobal]];

    matriz[i + 1][j] = max(previous_value, replace_items);
  } else {
    matriz[i + 1][j] = matriz[i][j];
  }
  return matriz;
}

// Envia última linha da matriz
int **enviaLinha(int **matriz, int sendRank, int k, int cols,
                 int subMatrizesLinhas) {

  MPI_Send(&matriz[subMatrizesLinhas][0], cols, MPI_INT, sendRank, k,
           MPI_COMM_WORLD);
  return matriz;
}

// Recebe última linha da matriz
int **recebeLinha(int **matriz, int recvRank, int k, int cols,
                  MPI_Status status) {
  MPI_Recv(&matriz[0][0], cols, MPI_INT, recvRank, k, MPI_COMM_WORLD, &status);
  return matriz;
}

// ALoca matriz de dados
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

// Imprime as informações iniciais da mochila
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

// Le o input inicial de dados
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

// Envia as mensagens iniciais
void envia_mensagens_iniciais(int rows, int size, int *wt, int *val) {
  for (int i = 1; i < size; i++) {
    MPI_Send(&wt[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    MPI_Send(&val[0], rows - 1, MPI_INT, i, 0, MPI_COMM_WORLD);
  }
}

// Recebe o peso inicial
int *recebe_pesos(int *wt, int rows, MPI_Status status) {
  wt = (int *)malloc((rows - 1) * sizeof(int));
  MPI_Recv(&wt[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  return wt;
}

// Recebe os itens iniciais
int *recebe_valores(int *val, int rows, MPI_Status status) {
  val = (int *)malloc((rows - 1) * sizeof(int));
  MPI_Recv(&val[0], rows - 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  return val;
}

int knapsack_serial(int MAXIMUM_CAPACITY, int wt[], int val[], int n) {
  int **V = get_matrix(n + 1, MAXIMUM_CAPACITY + 1);

  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 1; j <= MAXIMUM_CAPACITY; j++) {
      if (wt[i] <= j) { // could put item in knapsack
        int previous_value = V[1 + i - 1][j];
        int replace_items = val[i] + V[1 + i - 1][j - wt[i]];

        V[1 + i][j] = max(previous_value, replace_items);
      } else {
        V[1 + i][j] = V[1 + i - 1][j];
      }
    }
  }

  int retval = V[1 + n - 1][MAXIMUM_CAPACITY];

  free_matrix(V);

  return retval;
}
float timedifference_msec(struct timeval t0, struct timeval t1) {
  return (t1.tv_sec - t0.tv_sec) * 1000.0f +
         (t1.tv_usec - t0.tv_usec) / 1000.0f;
}
