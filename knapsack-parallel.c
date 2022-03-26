/* A Naive recursive implementation
of 0-1 Knapsack problem */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NUM_THREADS 2

double timestamp(void) {
  struct timeval tp;
  gettimeofday(&tp, NULL);

  return ((double)(tp.tv_sec * 1000.0 + tp.tv_usec / 1000.0));
}

// A utility function that returns
// maximum of two integers
int max(int a, int b) { return (a > b) ? a : b; }

// Returns the maximum value that can be
// put in a knapsack of capacity W
int knapSack(int W, int wt[], int val[], int n) {
  // Base Case

  if (n == 0 || W == 0)
    return 0;

  // If weight of the nth item is more than
  // Knapsack capacity W, then this item cannot
  // be included in the optimal solution

  if (wt[n - 1] > W) {
    // #pragma omp task shared(result)

    // #pragma omp taskwait
    return knapSack(W, wt, val, n - 1);
  }

  // Return the maximum of two cases:
  // (1) nth item included
  // (2) not included
  else {
    int max1 = 0, max2 = 0;
    int result;

#pragma omp task shared(max1)
    { max1 = val[n - 1] + knapSack(W - wt[n - 1], wt, val, n - 1); }

#pragma omp task shared(max2)
    { max2 = knapSack(W, wt, val, n - 1); }

#pragma omp taskwait
    result = max(max1, max2);

    // printf("Imprimindo o resultado: %d %d %d %d %d\n", result, n - 1,
    //        val[n - 1], wt[n - 1], omp_get_thread_num());
    return result;
  }
}

int main() {
  // Driver program to val[n-1] above function
  // Driver program to val[n-1] above function
  omp_set_num_threads(4);
  int n, W;
  double time;

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
  time = timestamp();
#pragma omp parallel
  {

#pragma omp single

    printf("%d\n", knapSack(W, wt, val, n));
  }
  time = timestamp() - time;
  time = timestamp() - time;
  printf("Tempo: %.6lf\n", time);
  return 0;
}
