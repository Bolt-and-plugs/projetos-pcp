#ifndef _OMP_J_H_
#define _OMP_J_H_

#include "omp.h"
#include "stdio.h"
#include "stdlib.h"

float **parallel_matrix_mul(float **A, float **B);

void omp_process(float **A, float *b, int n);

#endif
