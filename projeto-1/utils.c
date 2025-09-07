#include "utils.h"
#include "math.h"
#include "time.h"

void read_input(const char *path, long double **A, long double *b, const int n) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (fscanf(fp, "%Lf", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }

  for (int i = 0; i < n; i++) {
    if (fscanf(fp, "%Lf", &b[i]) != 1) {
      fprintf(stderr, "Error reading vector data at b[%d]\n", i);
      fclose(fp);
      exit(EXIT_FAILURE);
    }
  }
}

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
  td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
  td->tv_sec = t2.tv_sec - t1.tv_sec;
  if (td->tv_sec > 0 && td->tv_nsec < 0) {
    td->tv_nsec += NS_PER_SECOND;
    td->tv_sec--;
  } else if (td->tv_sec < 0 && td->tv_nsec > 0) {
    td->tv_nsec -= NS_PER_SECOND;
    td->tv_sec++;
  }
}

static void write_x_to_file(long double *x, int N, bool omp) {
  char buffer[256];
  if(omp)
    sprintf(buffer, "outputs/omp-mat-%d-%d.dat", N, N);
  else 
    sprintf(buffer, "outputs/seq-mat-%d-%d.dat", N, N);
  FILE *fp = fopen(buffer, "w");

  if (!fp) {
    printf("Arquivo mal formado %s\n", buffer);
    return;
  }

  for (int i = 0; i < N; i++)
    fprintf(fp, "[%.4Lf]\t", x[i]);

  fclose(fp);
}

void measure_fn_omp_time(long double*(fn)(long double **, long double *, int, int, char*, int), long double **A, long double *b,
                     int N, int num_threads, char* schedule, int chunk) {
  FILE *file;
  const char *file_name = "time_related/parallel_time.dat";
  puts("Initializing parallel execution");

  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  long double *result = fn(A, b, N, num_threads, schedule, chunk);
  write_x_to_file(result, N, true);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed: %d.%.9ld | Matrix Size:%d | Schedule:%s | Chunk:%d | Num_Threads:%d\n" , (int)_time.tv_sec, _time.tv_nsec, N, schedule, chunk, num_threads);
  fclose(file);

}

void measure_fn_seq_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,
                     int N) {
  FILE *file;
  puts("Initializing sequential execution");
  const char *file_name = "time_related/sequential_time.dat";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  long double *result = fn(A, b, N);
  write_x_to_file(result, N, false);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n ", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed: %d.%.9ld | Matrix Size: %d\n" , (int)_time.tv_sec, _time.tv_nsec, N);
  fclose(file);
}


long double *arr_norm(long double *x1, long double *x2, int N) {
  long double *x3 = malloc(sizeof(long double) * N);
  for (int i = 0; i < N; i++) {
    x3[i] = fabsl(x2[i] - x1[i]);
  }

  return x3;
}

void print_arr(long double *x, int N) {
  for (int i = 0; i < N; i++) {
    printf("[%Lf]\t", x[i]);
  }
  puts("");
}

void print_mat(long double **x, int N, int M) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      printf("[%Lf]\t", x[i][j]);
    }
    puts("");
  }
}
