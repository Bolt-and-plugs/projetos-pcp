#include "omp_j.h"
#include <omp.h>

long double **omp_matrix_mul(long double **A, long double **B, int n) {
  long double **C = calloc(sizeof(long double *), n);
  for (int i = 0; i < n; i++) {
    C[i] = calloc(sizeof(long double), n);
    if (!C[i]) {
      free(C);
      perror("ERROOOO");
      exit(0);
    }
  }

  int i, j, k;

  omp_set_num_threads(16);
  #pragma omp parallel for private(i, j, k) shared(A, B, C)
  for ( i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      for ( k = 0; k < n; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  return C;
}

void omp_process(long double **A, long double *b, int N) {
  long double **res = omp_matrix_mul(A, A, N);
}
