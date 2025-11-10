#include "seq.h"

int main(int argc, char **argv) {
  int N, M, **A;
  const char *input_path = argv[1];
  const char *output_path = argv[2];
  printf("MPI module initialized.\n");

  // input
  read_input(input_path, A, &N, &M);

  // logic

  // output

  write_file(output_path, A, N, M);
  return 0;
}
