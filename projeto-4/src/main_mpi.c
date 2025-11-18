#include "main_mpi.h"

char input_path[BUFF_SIZE], output_path[BUFF_SIZE];
int block_x, block_y, num_cli;
int **buff_write;
Queue *head;
int *it;
int g_iteration = 0;

static bool is_every_elem_one(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 1) return false;

  return true;
}

static bool is_every_elem_zero(int **A, int N, int M) {
  if (!A)
    return false;

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      if (A[i][j] != 0) return false;

  return true;
}

void mpi_process_block(int **A, int N, int M, int rank, int start_x,
                       int start_y, int block_x, int block_y) {
  int rand_int;
  while(it[rank] != g_iteration)
    ;

  for (int i = start_x; i < start_x + block_x && i < N; i++) {
    for (int j = start_y; j < start_y + block_y && j < M; j++) {
      
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

      srand(time(NULL));
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

  for (int i = start_x; i < start_x + block_x && i < N; i++) {
    for (int j = start_y; j < start_y + block_y && j < M; j++) {
      A[i][j] = buff_write[i][j];
    }
  }

  it[rank]++;
  if (rank == 0) {
    g_iteration++;
  }
}

void *mpi_worker(int **A, int N, int M, int rank) {
  int limit = N * M;
  while(true) {
    if (is_every_elem_one(A, N, M) || is_every_elem_zero(A, N, M) || g_iteration >= limit) {
      break;
    }

    // logica 
    mpi_process_block(A, N, M, rank, head->entry.bound_x,
                      head->entry.bound_y, block_x, block_y);
    pop(&head);

    // barreira de sincronismo
    MPI_Barrier(MPI_COMM_WORLD);

  }
  return NULL;
}

bool handle_args(int argc, char **argv) {
  if (argc < 5) {
    fprintf(stderr,
            "Usage: %s --input_path <input_path> --output_path <output_path> "
            "--num_cli <num_clients> --block_x <block_size> --block_y "
            "<block_size>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--input_path") == 0) {
      if (!argv[i + 1]) {
        fprintf(stderr, "Input path missing");
        return false;
      }
      strncpy(input_path, argv[i + 1], BUFF_SIZE - 1);
      input_path[BUFF_SIZE - 1] = '\0';
    } else if (strcmp(argv[i], "--output_path") == 0) {
      if (!argv[i + 1]) {
        fprintf(stderr, "Output path missing");
        return false;
      }
      strncpy(output_path, argv[i + 1], BUFF_SIZE - 1);
      output_path[BUFF_SIZE - 1] = '\0';
    } else if (strcmp(argv[i], "--num_cli") == 0) {
      const int local_num_cli = atoi(argv[i + 1]);
      if (local_num_cli <= 0 || local_num_cli >= 8) {
        num_cli = 8;
        continue;
      }
      num_cli = local_num_cli;
    } else if (strcmp(argv[i], "--block_x")==0) {
      const int local_block_size = atoi(argv[i + 1]);
      if (local_block_size <= 0 || local_block_size >= 8) {
        block_x = 8;
        continue;
      }
      block_x = local_block_size;
    } else if (strcmp(argv[i], "--block_y") == 0) {
      const int local_block_size = atoi(argv[i + 1]);
      if (local_block_size <= 0 || local_block_size >= 8) {
        block_y = 8;
        continue;
      }
      block_y = local_block_size;
    }
  }

  return true;
}

int main(int argc, char **argv) {
  // handle arguments
  int N, M, **A, rank, delta_time = 0;
  char mpi_buff[BUFF_SIZE];
  MPI_Status status;

  if (!handle_args(argc, argv)) {
    return EXIT_FAILURE;
  }

  if (!init_queue(&head, 0, block_x * block_y * sizeof(int))) {
    return 0;
  }

  // input
  read_input(input_path, A, &N, &M);

  // init buff_write 
  buff_write = malloc(sizeof(int *) * N);
  for (int i = 0; i < N; i++) {
    buff_write[i] = malloc(sizeof(int) * M);
  }

  for (int i = 0; i < num_cli; i++)
    for (int j = 0; j < num_cli; j++)
      push(&head, i * block_x, j * block_y);

  it = malloc(sizeof(int) * num_cli);
  for (int i = 0; i < num_cli; i++)
    it[i] = 0;

  // logic
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  measure_fn_mpi_time(mpi_worker, A, N, M, rank);
  MPI_Finalize();

  // output

  write_file(output_path, A, N, M);
  return 0;
}
