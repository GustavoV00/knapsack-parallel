/* A Naive recursive implementation
of 0-1 Knapsack problem */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double timestamp(void) {
  struct timeval tim;
  gettimeofday(&tim, NULL);

  return tim.tv_sec + (tim.tv_usec / 1000000.0);
}

void print_cost() { return; }

// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

// Returns the maximum value that can be
// put in a knapsack of capacity W

int knapSack(int W, int wt[], int val[], int n, int **m) {

  // base condition
  if (n < 0)
    return 0;

  if (m[n][W] != -1)
    return m[n][W];

  if (wt[n] > W) {

    // Store the value of function call
    // stack in table before return
    m[n][W] = knapSack(W, wt, val, n - 1, m);
    return m[n][W];

  } else {
    // Store value in a table before return
    int max1, max2, k;

#pragma omp task shared(max1)
    { max1 = val[n] + knapSack(W - wt[n], wt, val, n - 1, m); }

#pragma omp task shared(max2)
    { max2 = knapSack(W, wt, val, n - 1, m); }

#pragma omp taskwait
    { k = max(max1, max2); }

    // Return value of table after storing
    m[n][W] = k;
    return m[n][W];
  }
}

int test(int W, int wt[], int val[], int N) {
  int **m = malloc(N * sizeof(int *));

  for (int i = 0; i < N; i++) {
    m[i] = malloc((W + 1) * sizeof(int));
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < W + 1; j++) {
      m[i][j] = -1;
      // printf("%d ", m[i][j]);
    }
    // printf("\n");
  }

  int result;
#pragma omp parallel
  {
#pragma omp single
    { result = knapSack(W, wt, val, N - 1, m); }
  }

  return result;
}

int main() {
  // Driver program to val[n-1] above function
  // Driver program to val[n-1] above function
  // omp_set_num_threads(4);
  int n, W;
  double tpivot1 = 0, tpivot2 = 0, tpivot3 = 0; // time counting
  tpivot1 = timestamp();

  // n -> items
  // W -> Peso máximo
  scanf("%d %d", &n, &W);

  // val -> n_i
  // wt -> w_i
  int *val = (int *)calloc(n, sizeof(int));
  int *wt = (int *)calloc(n, sizeof(int));

  int i;
  for (i = 0; i < n; ++i) {
    scanf("%d %d", &(val[i]), &(wt[i]));
    // printf("value: %d ", val[i]);
    // printf("weight: %d\n", wt[i]);
  }

  tpivot2 = timestamp();
  printf("%d\n", test(W, wt, val, n));
  tpivot3 = timestamp();
  printf("Start:%.6lf\nFinish:%.6lf\n", tpivot3 - tpivot2, tpivot3 - tpivot1);
  return 0;
}
