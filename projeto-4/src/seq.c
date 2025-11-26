#include "seq.h"

char input_path[BUFF_SIZE], output_path[BUFF_SIZE];
int **buff_write;
int g_iteration = 0;

bool is_every_elem_one(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 1) return false;

  return true;
}

bool is_every_elem_zero(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 0) return false;

  return true;
}

void seq_process(int **A, int N, int M) {
  int rand_int;

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      buff_write[i][j] = A[i][j];
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      
      if (A[i][j] == empty)
        continue;

      if (A[i][j] == dead) {
        buff_write[i][j] = dead_twice;
        continue;
      }

      if (A[i][j] == dead_twice) {
        buff_write[i][j] = empty;
        continue;
      }

      rand_int = rand() % 10000;

      bool has_infetected_nearby = (
          (i > 0 && A[i - 1][j] == infected) ||
          (i < N - 1 && A[i + 1][j] == infected) ||
          (j > 0 && A[i][j - 1] == infected) ||
          (j < M - 1 && A[i][j + 1] == infected)
      );

      bool has_dead_nearby = (
          (i > 0 && A[i - 1][j] == dead) ||
          (i < N - 1 && A[i + 1][j] == dead) ||
          (j > 0 && A[i][j - 1] == dead) ||
          (j < M - 1 && A[i][j + 1] == dead)
      );

      if (A[i][j] == healthy && (has_infetected_nearby || has_dead_nearby)) {
        buff_write[i][j] = infected;
        continue;
      }

      if (rand_int - 999 <= 0) {
        buff_write[i][j] = healthy;
      } else if (rand_int >= 4000) {
        buff_write[i][j] = dead;
      } else {
        buff_write[i][j] = infected;
      }
    }
  }


  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      A[i][j] = buff_write[i][j];
    }
  }

  g_iteration++;
}

void *seq_worker(int **A, int N, int M) {
  int limit = N * M;
  
  while(true) {
    if (is_every_elem_one(A, N, M) || is_every_elem_zero(A, N, M) || g_iteration >= limit) {
      break;
    }

    seq_process(A, N, M);
  }
  
  printf("Sequential completed %d iterations\\n", g_iteration);
  return NULL;
}

bool handle_args(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr,
            "Usage: %s --input_path <input_path> --output_path <output_path>\n",
            argv[0]);
    return false;
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--input_path") == 0) {
      if (!argv[i + 1]) {
        fprintf(stderr, "Input path missing\n");
        return false;
      }
      strncpy(input_path, argv[i + 1], BUFF_SIZE - 1);
      input_path[BUFF_SIZE - 1] = '\0';
    } else if (strcmp(argv[i], "--output_path") == 0) {
      if (!argv[i + 1]) {
        fprintf(stderr, "Output path missing\n");
        return false;
      }
      strncpy(output_path, argv[i + 1], BUFF_SIZE - 1);
      output_path[BUFF_SIZE - 1] = '\0';
    }
  }

  return true;
}

int main(int argc, char **argv) {
  int N, M, **A;
  
  if (!handle_args(argc, argv)) {
    return EXIT_FAILURE;
  }

  printf("Sequential module initialized.\n");

  // Allocate matrix A
  A = malloc(sizeof(int *) * N);
  for (int i = 0; i < N; i++) {
    A[i] = malloc(sizeof(int) * M);
  }

  // input
  read_input(input_path, A, &N, &M);

  // init buff_write
  buff_write = malloc(sizeof(int *) * N);
  for (int i = 0; i < N; i++) {
    buff_write[i] = malloc(sizeof(int) * M);
  }

  // logic
  measure_fn_time(seq_worker, A, N, M, &g_iteration);

  // cleanup buff_write
  for (int i = 0; i < N; i++) {
    free(buff_write[i]);
  }
  free(buff_write);

  // output
  // write_file(output_path, A, N, M);
  
  // cleanup A
  for (int i = 0; i < N; i++) {
    free(A[i]);
  }
  free(A);
  
  return 0;
}
