#include "seq_j.h"

float **matrix_mul(float **A, float **B, int n) {
  float **C = calloc(sizeof(float *), n);
  for (int i = 0; i < n; i++) {
    C[i] = calloc(sizeof(float), n);
    if (!C[i]) {
      free(C);
      perror("ERROOOO");
      exit(0);
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  return C;
}

void seq_process(float **A, float *b, int N) {
  float **res = matrix_mul(A, A, N);
}
