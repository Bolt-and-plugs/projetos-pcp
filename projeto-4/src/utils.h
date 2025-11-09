#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void read_input(const char *path, int **A, int *N, int *M);

bool write_file(const char *path, int **A, int N, int M);

void measure_fn_time(long double *(fn)(long double **, long double *, int), long double **A, long double *b,int N);

void print_arr(long double *x, int N);

void print_mat(long double **x, int N, int M);

#endif
