#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

// parse N "long double's" from path to A
void read_input(const char *path, int **A, const int n);

bool write_file(const char *path, int **buffer, const int n);

void measure_fn_time(long double *(fn)(int, int, int), int N, int W, int H);
bool stop_test(long double *x1, long double *x2, long double precision, int N);

long double *arr_norm(long double *x1, long double *x2, int N);

void print_arr(long double *x, int N);

void print_mat(long double **x, int N, int M);

typedef struct {
    int queue_x[8];
    int queue_y[8];
    int head;
    int tail;
} queue;

void init_queue(queue* q);
void enqueue(queue *q, int x, int y);
void dequeue(queue *q);


#endif
