#include "main_mpi.h"

int main(int argc, char **argv) {
  int N, M, **A, rank, delta_time = 0;
  const char *input_path = argv[1];
  const char *output_path = argv[2];
  const int num_cli = atoi(argv[3]);
  char mpi_buff[BUFF_SIZE];

  MPI_Status status;
  
  // input
  read_input(input_path, A, &N, &M);

  // logic
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("MPI module initialized.\n");

  // root
  if (is_every_elem_one(A, N, M) || is_every_elem_zero(A, N, M)) {
    // stop
  }

  MPI_Finalize();

  // output

  write_file(output_path, A, N, M);
  return 0;
}
