#include "src/omp_j.h"
#include "src/seq_j.h"
#include "src/utils.h"
#include <string.h>

int execute_seq_test(char *dir) {
  int N = 2000;
  char path[256] = "inputs/";
  strcat(path, dir);
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
  read_input(path, coeficients, response, N);
  measure_fn_seq_time(seq_process, coeficients, response, N);
  free(coeficients);
  free(response);
  return 0;
}

int execute_omp_test(char *dir, int num_threads, char *scheduler_method,int chunk_size){
  int N = 2000;
  char path[256] = "inputs/";
  strcat(path, dir);
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

  read_input(path, coeficients, response, N);

  measure_fn_omp_time(omp_process, coeficients, response, N, num_threads, scheduler_method, chunk_size);

  free(coeficients);
  free(response);
  return 0;
}

int main(int argc, char* argv[]) {
  int num_threads, chunk_size;
  char* scheduler_method;
  char *input_file = malloc(256 * sizeof(char));
  bool seq = false;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage:\n");
      printf("%s --input_path <file_path> --schedule <scheduler_method> --chunk <chunk_size> --threads <num_threads>\n", argv[0]);
      printf("%s --input_path <file_path> --seq\n", argv[0]);
      printf("\n");
      printf("Additional arguments for parallel mode:\n");
      printf("--num_threads      Run parallel version with specified number of threads\n");
      printf("--schedule         Scheduling method (static, dynamic, guided)\n");
      printf("--chunk_size       Chunk size for scheduling (optional)\n");
      return 0;
    }
    if (strcmp(argv[i], "--seq") == 0) {
        seq = true;
    }

    if (strcmp(argv[i], "--schedule") == 0) {
        scheduler_method = argv[i + 1];
    }
    if (strcmp(argv[i], "--input_path") == 0) {
        input_file = argv[i + 1];
    }
    if (strcmp(argv[i], "--num_threads") == 0) {
        num_threads = atoi(argv[i + 1]);
    }
    if (strcmp(argv[i], "--chunk") == 0) {
        chunk_size = atoi(argv[i + 1]);
    }
  }

  if (seq) {
    execute_seq_test(input_file);
    return 0;
  }
  if (num_threads <= 0) {
    printf("Please provide a valid number of threads using --threads <num_threads>\n");
    return 1;
  }
  if (chunk_size <= 0) {
    chunk_size = 1;
  }
  execute_omp_test(input_file, num_threads, scheduler_method, chunk_size);

  return 0;
}
