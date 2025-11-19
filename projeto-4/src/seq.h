#ifndef _SEQ
#define _SEQ

#include "defines.h"
#include "utils.h"
#include "math.h"
#include "time.h"
#include "stdlib.h"

typedef enum _status { 
  dead_twice = -3, 
  dead = -2, 
  infected = -1, 
  empty = 0, 
  healthy = 1 
} status;

bool handle_args(int argc, char **argv);
bool is_every_elem_one(int **A, int N, int M);
bool is_every_elem_zero(int **A, int N, int M);
void seq_process(int **A, int N, int M);
void *seq_worker(int **A, int N, int M);

int main(int argc, char **argv);

#endif
