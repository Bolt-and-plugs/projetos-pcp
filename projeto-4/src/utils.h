#ifndef _UTILS 
#define _UTILS

#include "defines.h"
#include "math.h"
#include "time.h"
#include <semaphore.h>
#include "assert.h"

typedef struct __queue_entry {
  int bound_x, bound_y;
} queue_entry;

typedef struct __queue {
  struct __queue* next;
  queue_entry entry;
} Queue;

bool read_input(const char *path, int **A, int *N, int *M);

bool write_file(const char *path, int **A, int N, int M);

long double *arr_norm(long double *x1, long double *x2, int N);

void print_arr(long double *x, int N);

void print_mat(long double **x, int N, int M);

void measure_fn_time(void *(fn)(int **, int, int), int **A, int N, int M, int *iterations);


bool init_queue(Queue **q, int x, int y);
bool clear_queue(Queue **q);
bool pop(Queue **q);
bool push(Queue **head, int x, int y);

#endif
