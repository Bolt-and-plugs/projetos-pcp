#include "main_mpi.h"
#include <stdio.h>

char input_path[BUFF_SIZE], output_path[BUFF_SIZE];
int block_x, block_y, num_cli;

void *mpi_worker(int **A, int N, int M, int rank) {
  while(true) {
    if (is_every_elem_one(A, N, M) || is_every_elem_zero(A, N, M)) {
      break;
    }

    // logica 
    for (int i = 0; i < num_cli; i++) {
      for (int j = 0; j < num_cli; j++) {
        int start_x = i * block_x;
        int start_y = j * block_y;



      }
    }


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
  Queue *head;

  if (!handle_args(argc, argv)) {
    return EXIT_FAILURE;
  }

  if (!init_queue(&head, 0, block_x * block_y * sizeof(int))) {
    return 0;
  }

  // input
  read_input(input_path, A, &N, &M);

  for (int i = 0; i < num_cli; i++)
    for (int j = 0; j < num_cli; j++)
      push(&head, i * block_x, j * block_y);

  // logic
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  measure_fn_mpi_time(mpi_worker, A, N, M, rank);
  MPI_Finalize();

  // output

  write_file(output_path, A, N, M);
  return 0;
}
