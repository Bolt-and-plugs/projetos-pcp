#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void read_input(const char *path, float **A, float *b, const int n);

bool write_file(const char *path, char *buffer);

void measure_fn_time(void (fn)(float **, float *, int), float **A, float *b, int N);

bool stop_test(float *x1, float *x2, float precision, int N);

float *arr_norm(float *x1, float *x2, int N);

void print_arr(float *x, int N);

void print_mat(float **x, int N, int M);

#endif
