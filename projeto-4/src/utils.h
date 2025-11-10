#ifndef _UTILS 
#define _UTILS

#include "defines.h"
#include "math.h"
#include "time.h"


void read_input(const char *path, int **A, int *N, int *M);

bool write_file(const char *path, int **A, int N, int M);

long double *arr_norm(long double *x1, long double *x2, int N);

void print_arr(long double *x, int N);

void print_mat(long double **x, int N, int M);

bool is_every_elem_one(int **A, int N, int M);

bool is_every_elem_zero(int **A, int N, int M);

void measure_fn_time(void *(fn)(int **, int, int), int **A, int N, int M);

#endif
