#ifndef _OMP_J_H_
#define _OMP_J_H_

#include "omp.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "unistd.h"
#include "utils.h"

long double **parallel_matrix_mul(long double **A, long double **B);

long double *omp_process(long double **A, long double *b, int N, int num_threads, char *schedule, int chunk);

#endif
