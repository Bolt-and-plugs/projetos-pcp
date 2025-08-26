#ifndef _UTILS
#define _UTILS

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

void read_input(const char *path, float **A, float *b);

bool write_file(const char *path, char *buffer);

#endif
