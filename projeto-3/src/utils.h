#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include <time.h>

#include <cuda_runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ITERATION_END 404
#define ITERATION_CONTINUE 101

#define HEALTHY 1
#define INFECTED -1
#define DEAD -2
#define EMPTY 0

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);

void read_input(const char *path, int ***A, int *N, int *M);

void write_output(const char *output_path, int **A, int N, int M, int total_dead);

void measure_fn_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,int N);

void init_clock(struct timespec *start);

void end_clock(struct timespec start, int N, int M, int threads, int blocks);

#ifdef __cplusplus
}
#endif

#endif
