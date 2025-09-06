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

bool write_file(const char *path, char *buffer) { return true; }

void measure_fn_time(void(fn)(long double **, long double *, int, int, char*, int), long double **A, long double *b,
                     int N, int num_threads, char* schedule, int chunk) {
  FILE *file;
  const char *file_name = "time_related/parallel_time.dat";
  puts("Initialing parallel execution");

  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, b, N, num_threads, schedule, chunk);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed:%d.%.9ld |Matrix Size:%d |Schedule:%s |Chunk:%d\n |Num_Threads:%d" , (int)_time.tv_sec, _time.tv_nsec, N, schedule, chunk, num_threads);
  fclose(file);

}

void measure_fn_seq_time(void(fn)(long double **, long double *, int), long double **A, long double *b,
                     int N) {
  FILE *file;
  puts("Initialing sequential execution");
  const char *file_name = "time_related/sequential_time.dat";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, b, N);
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
