// Gustavo Valente nunes
// GRR 20182557

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_THREADS 4

void print_cost() { return; }

// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

int knapSack(int w, int wt[], int val[], int n, int **m) {

  if (n < 0)
    return 0;

  if (m[n][w] != -1)
    return m[n][w];

  if (wt[n] > w) {

    m[n][w] = knapSack(w, wt, val, n - 1, m);
    return m[n][w];

  } else {
    int max1, max2, k;

#pragma omp task shared(max1)
    { max1 = val[n] + knapSack(w - wt[n], wt, val, n - 1, m); }

#pragma omp task shared(max2)
    { max2 = knapSack(w, wt, val, n - 1, m); }

#pragma omp taskwait
    { k = max(max1, max2); }

    m[n][w] = k;
    return m[n][w];
  }
}

int **test(int w, int wt[], int val[], int n) {
  int **m = malloc(n * sizeof(int *));

  for (int i = 0; i < n; i++) {
    m[i] = malloc((w + 1) * sizeof(int));
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < w + 1; j++) {
      m[i][j] = -1;
      // printf("%d ", m[i][j]);
    }
    // printf("\n");
  }

  return m;
}

int main() {
  int n, w;
  // time_t init_seq = time(nULL);
  // clock_t init_seq, end_seq, init_paral, end_paral;
  // init_seq = clock();
  // printf("%ld\n", init_seq);

  // tpivot1 = timestamp();

  // n -> items
  // w -> Peso máximo
  scanf("%d %d", &n, &w);

  // val -> n_i
  // wt -> w_i
  int *val = (int *)calloc(n, sizeof(int));
  int *wt = (int *)calloc(n, sizeof(int));

  int i;
  for (i = 0; i < n; ++i) {
    scanf("%d %d", &(val[i]), &(wt[i]));
  }

  int **m = test(w, wt, val, n);

  // end_seq = clock();
  // double total1 = (double)(end_seq - init_seq) / CLOCKS_PER_SEC;
  // printf("%f\n", total1);

  // init_paral = clock();

  int result;
#pragma omp parallel num_threads(NUM_THREADS)
  {
#pragma omp single
    { result = knapSack(w, wt, val, n - 1, m); }
  }

  // end_paral = clock();
  // double total2 = (double)(end_paral - init_paral) / CLOCKS_PER_SEC;
  // printf("%f\n", total2);
  // printf("%f\n", total1 + total2);

  printf("%d\n", result);
  return 0;
}
