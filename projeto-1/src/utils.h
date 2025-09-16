#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void read_input(const char *path, long double **A, long double *b, const int n);

bool write_file(const char *path, char *buffer);

void measure_fn_omp_time(long double *(fn)(long double **, long double *, int, int, char*, int), long double **A, long double *b,int N, int num_threads, char* schedule, int chunk);

void measure_fn_seq_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,int N);
bool stop_test(long double *x1, long double *x2, long double precision, int N);

long double *arr_norm(long double *x1, long double *x2, int N);

void print_arr(long double *x, int N);

void print_mat(long double **x, int N, int M);

#endif
