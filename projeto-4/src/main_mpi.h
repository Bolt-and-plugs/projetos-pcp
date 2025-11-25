#ifndef _MPI
#define _MPI

#include "defines.h"
#include "utils.h"
#include "mpi.h"
#include "time.h"
#include "stdlib.h"

typedef enum _status { dead_twice = -3, dead = -2, infected = -1, empty = 0, healthy = 1 } status;

int main(int argc, char **argv);

#endif
