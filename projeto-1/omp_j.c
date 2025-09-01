#include "omp_j.h"
#include <omp.h>

float **omp_matrix_mul(float **A, float **B, int n) {
  float **C = calloc(sizeof(float *), n);
  for (int i = 0; i < n; i++) {
    C[i] = calloc(sizeof(float), n);
    if (!C[i]) {
      free(C);
      perror("ERROOOO");
      exit(0);
    }
  }

  int i, j, k;
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

void omp_process(float **A, float *b, int N) {
  omp_set_num_threads(8);
  float **res = omp_matrix_mul(A, A, N);
}
