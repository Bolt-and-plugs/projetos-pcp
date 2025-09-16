#ifndef _SEQ_J_H_
#define _SEQ_J_H_

#include "stdio.h"
#include "stdlib.h"
#include "utils.h"

long double **matrix_mul(long double **A, long double **B, int n);

long double *seq_process(long double **A, long double *b, int N);

#endif
