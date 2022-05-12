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
void print_matriz(int taskid, int val, int peso, int subColunas,
                  int *dependencias, int *novosDados, int capacidade);
void free_matrix(int **mat);

int *bcastValues(int taskid, int size, int subMatrizesColunasExtras, int bound,
                 int subMatrizesColunas, int j, int *dependencia,
                 int *novosDados);
void imprimeInformacoesDaMochila(int capacidade, int quantidadeItens,
                                 int *valores, int *pesos);
void envia_mensagens_iniciais(int rows, int size, int *wt, int *val, int cols,
                              int itens, int capacidade);
int *recebe_pesos(int *wt, int rows, MPI_Status status);
int *recebe_valores(int *val, int rows, MPI_Status status);
int **alocaMatriz(int subMatrizesLinhas, int cols, int taskid, int size,
                  int subMatrizesLinhasExtras, int **matriz, int itens,
                  int capacidade);
int knapsack_serial(int MAXIMUM_CAPACITY, int wt[], int val[], int n);
int *resolvedorBloco(int *dependencias, int *novosDados, int i, int j,
                     int *valores, int *pesos, int jGlobal);
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

  if (taskid == 0) {
    scanf("%d %d", &itens, &capacidade);
    valores = (int *)calloc(itens, sizeof(int));
    pesos = (int *)calloc(itens, sizeof(int));

    rows = itens + 1;
    cols = capacidade + 1;
    leArquivosDeEntrada(pesos, valores, itens);
    // gettimeofday(&serialFim, 0);
    // serialTime += timedifference_msec(serialIni, serialFim);
  }

  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&itens, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&capacidade, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // printf("taskid %d, cols: %d, rows: %d, itens: %d, capacidade:%d\n", taskid,
  //        cols, rows, itens, capacidade);

  if (taskid == 0) {
    gettimeofday(&serialIni, 0);
    envia_mensagens_iniciais(rows, size, pesos, valores, cols, itens,
                             capacidade);
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
      printf("Result: %d\n", result);
  }

  // if (taskid == size - 1) {
  //   gettimeofday(&serialIni, 0);
  //   float elapsed;

  //   gettimeofday(&t1, 0);

  //   elapsed = timedifference_msec(t0, t1);
  //   gettimeofday(&serialFim, 0);
  //   serialTime += timedifference_msec(serialIni, serialFim);

  //   // Para ver os tempos funcionando, basta descomentar essas duas linhas.
  //   // printf("serialTime: %f\n", serialTime);
  //   // printf("time: %f\n", elapsed);
  // }

  MPI_Finalize();
  return 0;
}

int knapsack_parallel(int capacidade, int *pesos, int *valores, int itens,
                      int cols, int taskid, int size, MPI_Status status,
                      struct timeval ini, struct timeval fim,
                      float *serialTime) {

  // Matriz para memoization
  // int recvRank = (taskid - 1) % size; // rank to receive data
  // int sendRank = (taskid + 1) % size; // rank to send data
  printf("cols: %d e %d\n", cols, size);
  // int **matriz = get_matrix(2, cols);
  int subMatrizesColunas = cols / size;
  int subMatrizesColunasExtras = cols % size;
  int bound = taskid == size - 1 ? subMatrizesColunasExtras + subMatrizesColunas
                                 : subMatrizesColunas;

  printf("subMatrizesCOlunas: %d | colunaExtras %d\n", subMatrizesColunas,
         subMatrizesColunasExtras);
  int *dependencia = (int *)malloc(cols * sizeof(int));
  int *novosDados = (int *)malloc(bound * sizeof(int));
  for (int i = 0; i < cols; i++) {
    dependencia[i] = 0;
  }

  int i, j;
  printf("taskid: %d e bound: %d\n", taskid, bound);
  printf("\n\n\n");
  for (i = 0; i < itens; i++) {

    for (j = 0; j < bound; j++) {
      if (j == 0 && taskid == 0) {
        novosDados[0] = 0;
        continue;
      }

      int jGlobal = (subMatrizesColunas * taskid) + j;
      //   // printf("jGloba: %d\n", jGlobal);
      novosDados = resolvedorBloco(dependencia, novosDados, i, j, valores,
                                   pesos, jGlobal);
    }

    MPI_Allgather(novosDados, subMatrizesColunas, MPI_INT, dependencia,
                  subMatrizesColunas, MPI_INT, MPI_COMM_WORLD);

    dependencia = bcastValues(taskid, size, subMatrizesColunasExtras, bound,
                              subMatrizesColunas, j, dependencia, novosDados);
  }
  int result = dependencia[cols - 1];
  if (result == 0)
    result = -1;

  free(dependencia);
  free(novosDados);
  return result;
}

int *bcastValues(int taskid, int size, int subMatrizesColunasExtras, int bound,
                 int subMatrizesColunas, int j, int *dependencia,
                 int *novosDados) {

  if (taskid == size - 1 && subMatrizesColunasExtras != 0) {
    for (j = 0; j < bound; j++) {
      int jGlobal = (subMatrizesColunas * taskid) + j;
      if (j >= subMatrizesColunas) {
        dependencia[jGlobal] = novosDados[j];
      }
    }
  }

  return dependencia;
}

int *resolvedorBloco(int *dependencias, int *novosDados, int i, int j,
                     int *valores, int *pesos, int jGlobal) {

  if (pesos[i] <= jGlobal) {

    // int previous_value = matriz[iAux][j];
    int previous_value = dependencias[jGlobal];

    // int replace_items = valores[i] + matriz[iAux][j - pesos[i]];
    int replace_items = valores[i] + dependencias[jGlobal - pesos[i]];

    // matriz[iGlobal][j] = max(previous_value, replace_items);
    novosDados[j] = max(previous_value, replace_items);
  } else {
    // matriz[iGlobal][j] = matriz[iGlobal][j]
    novosDados[j] = dependencias[jGlobal];
  }
  return novosDados;
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

void print_matriz(int taskid, int val, int peso, int subColunas,
                  int *dependencias, int *novosDados, int capacidade) {

  // printf("/***********************************/\n");
  printf("Taksid %d imprimiu a matriz: %d e peso %d\n", taskid, val, peso);
  for (int i = 0; i <= capacidade; i++) {
    printf("%d ", dependencias[i]);
  }
  // printf("\n");
  // for (int i = 0; i < subColunas; i++) {
  //   printf("%d ", novosDados[i]);
  // }
  // printf("\n/***********************************/\n\n");
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
void envia_mensagens_iniciais(int rows, int size, int *wt, int *val, int cols,
                              int itens, int capacidade) {
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
        // or is it better to swap whatever we have in the bag that weights
        // up to `j` and put item `i`?
        V[1 + i][j] = max(previous_value, replace_items);
      } else {
        // can't put item `i`
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
