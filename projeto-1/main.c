#include "omp_j.h"
#include "seq_j.h"
#include "utils.h"
#include "string.h"

int main(int argc, char **argv) {
  // treat args 
  // can receive N as argument
  // if not, use default value of 3
  // can receive omp threads as argument
  // if not, use default value of 4
  // can receive input file as argument
  // if not, use default value of inputs/linear%d.dat
  // where %d is the value of N
  // example: ./main --N 5 --num_threads 8 --input inputs/linear5.dat
  // give also --help
  


  int N = 3;
  int num_threads = 4;
  char *input =  malloc(256 * sizeof(char));

  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "--N") == 0 || strcmp(argv[i], "-n") == 0) && i + 1 < argc) {
      N = atoi(argv[i + 1]);
      i++;
    } else if ((strcmp(argv[i], "--num_threads") == 0 || strcmp(argv[i], "-nt") ==0) && i + 1 < argc) {
      num_threads = atoi(argv[i + 1]);
      i++;
    } else if ((strcmp(argv[i], "--input") == 0 || strcmp(argv[i], "-i") == 0) && i + 1 < argc) {
      strncpy(input, argv[i + 1], 255);
      input[255] = '\0';
      i++;
    }
    else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [--N <matrix_size>] [--num_threads <number_of_threads>] [--input <input_file>]\n", argv[0]);
      exit(0);
    }
  }

  sprintf(input, "inputs/linear%d.dat", N);

  omp_set_num_threads(num_threads);
  printf("Matrix size: %d\n", N);
  printf("Number of threads: %d\n", num_threads);
  printf("Input file: %s\n", input);

  long double *response = malloc(N * sizeof(long double));
  long double **coeficients = malloc(N * sizeof(long double *));

  if (coeficients == NULL) {
    perror("Failed to allocate memory for rows");
    return 1;
  }

  for (int i = 0; i < N; i++) {
    coeficients[i] = malloc(N * sizeof(long double));
    if (!coeficients[i]) {
      perror("Failed to allocate memory for a column");
      free(coeficients);
      free(response);
      return 1;
    }
  }


  read_input(input, coeficients, response, N);


  measure_fn_time(seq_process, coeficients, response, N);
  measure_fn_time(omp_process, coeficients, response, N);

  free(coeficients);
  free(response);
}
