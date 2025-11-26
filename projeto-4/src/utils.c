#include "utils.h"

sem_t mutex;
bool initilized = false;

enum { NS_PER_SECOND = 1000000000 };

static void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
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

bool read_input(const char *path, int **A, int *N, int *M) {
  assert(path != NULL && strlen(path) > 0);

  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    fclose(fp);
    return false;
  }

  if (fscanf(fp, "%d %d\n", N, M) != 2) {
    fprintf(stderr, "Error reading N and M at [%d][%d]\n", i, j);
    fclose(fp);
    return false;
  }

  for (int i = 0; i < *N; i++) {
    for (int j = 0; j < *M; j++) {
      if (fscanf(fp, "%d", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        return false;
      }
    }
  }

  return true;
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



void print_mat(long double **x, int N, int M) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      printf("[%Lf]\t", x[i][j]);
    }
    puts("");
  }
}



void measure_fn_time(void *(fn)(int **, int, int), int **A, int N,
                     int M, int *iterations) {
  FILE *file;
  puts("Initializing sequential execution");
  const char *file_name = "assets/output/time_measure.txt";
  file = fopen(file_name, "a");
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, N, M);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  fprintf(file,"Tipo: Sequencial | Processos: 1 | Tempo: %d.%.9ld s | Tamanho Matriz: %d | Iterações: %d\n" , (int)_time.tv_sec, _time.tv_nsec, N, *iterations);
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

  if (!initilized) {
    sem_init(&mutex, 0, 1);
    initilized = true;
  }
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
  assert(q != NULL);

  sem_wait(&mutex);
  if (*q && !(*q)->next) {
    free(*q);
    *q = NULL;
    sem_post(&mutex);
    return true;
  }

  Queue *l;
  if (*q && (*q)->next) {
    l = *q;
    *q = (*q)->next;
    free(l);
    sem_post(&mutex);
    return true;
  }

  fprintf(stderr, "Queue is already empty");
  sem_post(&mutex);
  return false;
}

bool push(Queue **head, int x, int y) {
  assert(head != NULL);

  sem_wait(&mutex);
  Queue *q = NULL;
  Queue *l = *head;

  if(!init_queue(&q, x, y)) {
    fprintf(stderr, "Could not push to queue: new node not initialized");
    sem_post(&mutex);
    return false;
  }

  while (l->next != NULL) {
    l = l->next;
  }
  l->next = q;
  sem_post(&mutex);
  return true;
}
