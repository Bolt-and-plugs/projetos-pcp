#include "seq_j.h"
#include "unistd.h"
#include "math.h"

long double **matrix_mul(long double **A, long double **B, int n) {
  long double **C = calloc(sizeof(long double *), n);
  for (int i = 0; i < n; i++) {
    C[i] = calloc(sizeof(long double), n);
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

bool stop_test(long double *x1, long double *x2, long double precision, int N) {
  for (int i = 0; i < N; i++) {
    long double result = fabsl(x2[i] - x1[i]);
    if (result > precision)
      return true;
  }
  return false;
}

void write_x_to_file(long double *x, int N) {
  char buffer[256];
  sprintf(buffer, "outputs/seq/mat-%d-%d.dat", N, N);
  FILE *fp = fopen(buffer, "w");

  if (!fp) {
    printf("Arquivo mal formado %s\n", buffer);
    return;
  }

  for (int i = 0; i < N; i++)
    fprintf(fp, "[%.4Lf]\t", x[i]);

  fclose(fp);
}

long double *seq_process(long double **A, long double *b, int N) {
  long double *x[2];
  long long it = 0;
  bool curr_x = false;

  for (int i = 0; i < 2; i++) {
    x[i] = calloc(sizeof(long double), N);
  }

  do {
    curr_x = !curr_x;
    for (int i = 0; i < N; i++) {
      long double inverted_aii = 1.0L / A[i][i];
      long double sum = 0.0L;

      for (int j = 0; j < N; j++) {
        if (j == i) 
          continue;
        sum += A[i][j] * x[curr_x][j];

      }
      x[!curr_x][i] = inverted_aii * (b[i] - sum);
    }
    it++;
  }
  while (stop_test(x[0], x[1], 10e-5, N));

  free(x[curr_x]);
  return x[!curr_x];
}
