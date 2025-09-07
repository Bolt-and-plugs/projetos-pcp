#include "omp_j.h"
#include "math.h"
#include <string.h>
#include <math.h>

static long double **omp_matrix_mul(long double **A, long double **B, int n) {
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
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      for (k = 0; k < n; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  return C;
}

bool omp_stop_test(long double *x1, long double *x2, long double precision,
                   int N) {
  bool must_continue = false;
  #pragma omp parallel for reduction(|| : must_continue)
  for (int i = 0; i < N; i++) {
    long double result = fabsl(x2[i] - x1[i]);

    if (result > precision) {
      must_continue = true;
    }
  }

  return must_continue;
}

long double *omp_process(long double **A, long double *b, int N, int num_threads,
                 char *schedule, int chunk) {
  long double *x[2];
  long long it = 0;
  bool curr_x = false;

  for (int i = 0; i < 2; i++) {
    x[i] = calloc(sizeof(long double), N);
  }

  omp_set_num_threads(num_threads);
  printf("Schedule: %s | Chunk: %d  ", schedule, chunk);
  do {
    curr_x = !curr_x;

    if (strcmp(schedule, "static") == 0) {
      omp_set_schedule(omp_sched_static, chunk);
    } else if (strcmp(schedule, "dynamic") == 0) {
      omp_set_schedule(omp_sched_dynamic, chunk);
    } else if (strcmp(schedule, "guided") == 0) {
      omp_set_schedule(omp_sched_guided, chunk);
    } else {
      printf("Aviso: Schedule inválido. Usando o auto.\n");
      omp_set_schedule(omp_sched_auto, chunk);
    }
    int i;
    #pragma omp parallel for private(i) shared(x, A, b)
    for (i = 0; i < N; i++) {
      long double inverted_aii = 1.0L / A[i][i];
      long double sum = 0.0L;

      for (int j = 0; j < N; j++) {
        if (j == i)
          continue;
        #pragma omp atomic
        sum += A[i][j] * x[curr_x][j];
      }
      x[!curr_x][i] = inverted_aii * (b[i] - sum);
    }
    it++;
  } while (omp_stop_test(x[0], x[1], 10e-5, N));

  free(x[curr_x]);
  return x[!curr_x];
}
