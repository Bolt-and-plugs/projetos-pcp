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
  float *x[2]; // so precisamos da iteracao k + 1 e k para o teste de parada
  long long it = 0;

  for (int i = 0; i < 2; i++) {
    x[i] = calloc(sizeof(float), N);
  }

  do {
    printf("Iteração de número %Ld\n", it);

    for (int i = 0; i < N; i++) {
      // iterar por aqui
    }
    
    it++;
  }
  while (stop_test(x[0], x[1], 10e-5, N));
}
