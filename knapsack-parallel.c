/* A Naive recursive implementation
of 0-1 Knapsack problem */
// #include <omp.h>
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

  if (wt[n - 1] > W)
    return knapSack(W, wt, val, n - 1);

  // Return the maximum of two cases:
  // (1) nth item included
  // (2) not included
  else
    return max(val[n - 1] + knapSack(W - wt[n - 1], wt, val, n - 1),
               knapSack(W, wt, val, n - 1));
}

//// PARALELO ######

// Driver program to test above function
int main() {
  int n, W;
  double time;
  int threadsReal;
  int resultTest[NUM_THREADS];

  scanf("%d %d", &n, &W);
  int *val = (int *)calloc(n, sizeof(int));
  int *wt = (int *)calloc(n, sizeof(int));

  int i;
  // int nthrds;
#pragma omp for
  for (i = 0; i < n; ++i) {
    scanf("%d %d", &(val[i]), &(wt[i]));
  }

  time = timestamp();

#pragma omp parallel num_threads(NUM_THREADS)
  {
    int id = omp_get_thread_num();
    int threadsFake = omp_get_num_threads();
    int newN = n / threadsFake;
    int newW = W / threadsFake;
    int *newVal = (int *)calloc(newN, sizeof(int));
    int *newWt = (int *)calloc(newN, sizeof(int));

    if (id == 0)
      threadsReal = threadsFake;

    resultTest[id] = knapSack(newW * id, wt, val, newN * id);
  }

  int sum = 0;
  for (int i = 0; i < NUM_THREADS; i++) {
    sum += resultTest[i];
  }
  printf("Soma final: %d\n", sum);

  time = timestamp() - time;
  printf("Tempo: %.6lf\n", time);
  return 0;
}
