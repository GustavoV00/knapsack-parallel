/* A Naive recursive implementation
of 0-1 Knapsack problem */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  else {
    int result = max(val[n - 1] + knapSack(W - wt[n - 1], wt, val, n - 1),
                     knapSack(W, wt, val, n - 1));

    // printf("Imprimindo o resultado: %d %d %d %d\n", result, n - 1, val[n -
    // 1],
    //        wt[n - 1]);
    return result;
  }
}

int main() {
  // Driver program to val[n-1] above function
  // Driver program to val[n-1] above function
  int n, W;
  // time_t init_seq = time(NULL);
  clock_t init_seq, end_seq;
  init_seq = clock();
  // printf("%ld\n", init_seq);

  // tpivot1 = timestamp();

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
  }

  int result = knapSack(W, wt, val, n - 1);
  end_seq = clock();
  double total1 = (double)(end_seq - init_seq) / CLOCKS_PER_SEC;

  printf("%f\n", total1);

  printf("%d\n", result);
  return 0;
}
