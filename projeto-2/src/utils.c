#include "utils.h"
#include "math.h"
#include "time.h"

void read_input(const char *path, int **A, const int n) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (fscanf(fp, "%d", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
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

void measure_fn_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,
                     int N) {
  FILE *file;
  puts("Initializing sequential execution");
  const char *file_name = "time_related/times.dat";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  long double *result = fn(N, W, H);
  clock_gettime(CLOCK_MONOTONIC, &end);
  //write_x_to_file(result, N, false); no need anymore.
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n ", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed: %d.%.9ld | Num Blocks: %d | Blocks Dimension: %d x %d " , (int)_time.tv_sec, _time.tv_nsec, N, W, H);
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

void init_queue(queue* q){
  q->head = 1;
  q->tail = 0;
}

void enqueue(queue *q, int x, int y){
  q->queue_x[q->tail] = x;
  q->queue_y[q->tail] = y;
  q->tail++;
}

void dequeue(queue *q){
  q->head++;
}