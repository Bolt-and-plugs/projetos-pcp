#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void read_input(const char *path, float **A, float *b, const int n);

bool write_file(const char *path, char *buffer);

void measure_fn_time(void (fn)(float **, float *, int), float **A, float *b, int N);

#endif
