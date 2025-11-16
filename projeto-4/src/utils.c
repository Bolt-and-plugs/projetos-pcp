#include "utils.h"

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
  if (omp)
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

void read_input(const char *path, int **A, int *N, int *M) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  if (fscanf(fp, "%d %d\n", N, M) != 2) {
    fprintf(stderr, "Error reading N and M at [%d][%d]\n", i, j);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < *N; i++) {
    for (int j = 0; j < *M; j++) {
      if (fscanf(fp, "%d", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }
}

bool write_file(const char *path, int **A, int N, int M) {
  FILE *fp = fopen(path, "w");

  if (!fp) {
    fprintf(stderr, "Could not write file down");
    fclose(fp);
    return false;
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      if (fprintf(fp, "%d", A[i][j]) < 0) {
        fprintf(stderr, "Could not write file down");
        fclose(fp);
        return false;
      }
    }
    if (fprintf(fp, "\n") < 0) {

      fprintf(stderr, "Could not write file down");
      fclose(fp);
      return false;
    }
  }

  return true;
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

bool is_every_elem_one(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 1) return false;

  return true;
}

bool is_every_elem_zero(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 0) return false;

  return true;
}

void print_mat(long double **x, int N, int M) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      printf("[%Lf]\t", x[i][j]);
    }
    puts("");
  }
}



void measure_fn_time(void *(fn)(int **, int, int), int **A, int N,
                     int M) {
  FILE *file;
  puts("Initializing sequential execution");
  const char *file_name = "assets/output/time_measure.txt";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, N, M);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld | Matrix Size: %d\n ", (int)_time.tv_sec, _time.tv_nsec, N);
  fprintf(file,"Time elapsed: %d.%.9ld | Matrix Size: %d\n" , (int)_time.tv_sec, _time.tv_nsec, N);
  fclose(file);
}

// queue
bool init_queue(Queue **q, int x, int y) {
  *q = malloc(sizeof(Queue));

  if (!*q) {
    fprintf(stderr, "Could not Initialize Queue"); 
    return false;
  }

  (*q)->entry.bound_x = x;
  (*q)->entry.bound_y = y;
  (*q)->next = NULL;
  return true;
}
bool clear_queue(Queue **q) {
  if (!*q)
    return false;
  
  Queue *l = NULL;
  while (*q != NULL) {
    l = *q;
    *q = (*q)->next;
    free(l);
  }

  return true;
}
bool pop(Queue **q) {
  if (*q && !(*q)->next) {
    free(*q);
    *q = NULL;
    return true;
  }

  Queue *l;
  if (*q && (*q)->next) {
    l = *q;
    *q = (*q)->next;
    free(l);
    return true;
  }

  fprintf(stderr, "Queue is already empty");
  return false;
}

bool push(Queue **head, int x, int y) {
  if (!*head) {
    if(!init_queue(head, 0, 0)) {
      fprintf(stderr, "Could not push to queue: queue head not initialized");
      return false;
    }
    return true;
  }

  Queue *q = NULL;
  Queue *l = *head;

  if(!init_queue(&q, x, y)) {
    fprintf(stderr, "Could not push to queue: new node not initialized");
    return false;
  }

  while (l->next != NULL) {
    l = l->next;
  }
  l->next = q;
  return true;
}

