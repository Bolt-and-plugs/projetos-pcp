#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

// parse N "long double's" from path to A
void read_input(const char *path, int **A, const int n);

bool write_file(const char *path, char *buffer);

void measure_fn_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,int N);
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
