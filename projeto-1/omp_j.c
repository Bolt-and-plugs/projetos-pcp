#include "omp_j.h"
#include "math.h"
#include <string.h>
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
  for ( i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      for ( k = 0; k < n; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  return C;
}

static long double **matrix_mul(long double **A, long double **B, int n) {
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

static bool has_solution(long double **A, int N)  {
  // criterio da linha
  long double sum = 0 ;
  int i;

  bool must_continue = false;

  #pragma omp parallel private(i) reduction(||:must_continue)
  for (i = 0; i < N; i++) {
    int j;
    #pragma omp parallel shared(j)
    for (j = 0; j < N; j++) {
      if (i == j)
        continue;
      #pragma omp atomic
      sum += A[i][j];
    }
    if (A[i][i] >= sum)
      must_continue = true;
  }
  return must_continue;
}

static void write_x_to_file(long double *x, int N) { 
  char buffer[256];
  sprintf(buffer, "outputs/omp/mat-%d-%d.dat", N, N); // TODO change path to handle num_threads, chunk_size and scheduler_method
  FILE *fp = fopen(buffer, "w");

  if (!fp) {
    printf("Arquivo mal formado %s\n", buffer);
    return;
  }

  for (int i = 0; i < N; i++)
    fprintf(fp, "[%.4Lf]\t", x[i]);

  fclose(fp);
}


bool omp_stop_test(long double *x1, long double *x2, long double precision, int N) {
    bool must_continue = false;
    #pragma omp parallel for reduction(||:must_continue)
    for (int i = 0; i < N; i++) {
        long double result = fabsl(x2[i] - x1[i]);

        if (result > precision) {
            must_continue = true;
        }
    }

    return must_continue;
}

void omp_process(long double **A, long double *b, int N, int num_threads, char *schedule,int chunk) {
  long double *x[2];
  long long it = 0;
  puts("executando");

  /*if (!has_solution(A, N)) {
  #  perror("A matriz de coeficientes não possui solução através do método de jacobi");
    exit(0);
  }*/

  for (int i = 0; i < 2; i++) {
    x[i] = calloc(sizeof(long double), N);
  }

  int curr_x = 0;
  // alterar threads resenha
  omp_set_num_threads(num_threads);
  printf("Schedule :%s | Chunk: %d  ", schedule, chunk); 
  do {
    curr_x = !curr_x;
    
    if (strcmp(schedule, "static") == 0) {
        omp_set_schedule(omp_sched_static, chunk);
    } else if (strcmp(schedule, "dynamic") == 0) {
        omp_set_schedule(omp_sched_dynamic, chunk);
    } else if (strcmp(schedule, "guided") == 0) {
        omp_set_schedule(omp_sched_guided, chunk);
    } else {
        printf("Aviso: Schedule inválido. Usando o padrão (runtime).\n");
        return;
    }
    int i;
    #pragma omp parallel for private(i) shared(x, A, b) schedule(runtime)
    for (i = 0; i < N; i++) {
      long double inverted_aii = 1.0L / A[i][i];
      long double sum = 0.0L;

      int j;
      #pragma omp parallel for private(j) shared(sum)
      for (j = 0; j < N; j++) {
        if (j == i) 
          continue;
        #pragma omp atomic 
        sum += A[i][j] * x[curr_x][j];
      }
      x[!curr_x][i] = inverted_aii * (b[i] - sum);
    }
    it++;
  }
  while (omp_stop_test(x[0], x[1], 10e-5, N));

  //print_arr(x[curr_x], N);

  write_x_to_file(x[curr_x], N);
}
