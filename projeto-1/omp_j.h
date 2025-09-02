#ifndef _OMP_J_H_
#define _OMP_J_H_

#include "omp.h"
#include "stdio.h"
#include "stdlib.h"

long double **parallel_matrix_mul(long double **A, long double **B);

void omp_process(long double **A, long double *b, int n);

#endif
