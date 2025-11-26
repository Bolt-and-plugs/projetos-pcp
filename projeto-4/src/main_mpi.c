#include "main_mpi.h"

char input_path[BUFF_SIZE], output_path[BUFF_SIZE];
int block_x, block_y, num_cli;
int **buff_write;
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


enum { NS_PER_SECOND = 1000000000 };

static void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
  td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
  td->tv_sec = t2.tv_sec - t1.tv_sec;
  if (td->tv_sec > 0 && td->tv_nsec < 0) {
    td->tv_nsec += NS_PER_SECOND;
    td->tv_sec--;
  } else if (td->tv_sec < 0 && td->tv_nsec > 0) {
    td->tv_nsec -= NS_PER_SECOND;
    td->tv_sec++;
  }
}

void measure_fn_mpi_time(void *(fn)(int **, int, int, int, int), int **A, int N,
                     int M, int rank, int size, int num_processes) {
  FILE *file;
  const char *file_name = "assets/output/time_measure.txt";
  struct timespec start, end, _time;

  if (rank == 0) {
    puts("Initializing mpi execution");
  }
  
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, N, M, rank, size);
  
  // Wait for all processes to finish before measuring end time
  MPI_Barrier(MPI_COMM_WORLD);
  
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  
  if (rank == 0) {
    file = fopen(file_name, "a");
    if (file) {
      fprintf(file,"Tipo: Paralelo | Processos: %d | Tempo: %d.%.9ld s | Tamanho Matriz: %d | Iterações: %d\n" , num_processes, (int)_time.tv_sec, _time.tv_nsec, N, g_iteration);
      fclose(file);
    } else {
      perror("Error opening time measure file");
    }
  }
}

void mpi_process_block(int **A, int **local_buff, int N, int M, int start_x,
                       int start_y, int block_x, int block_y) {
  int rand_int;

  // Process block
  for (int i = start_x; i < start_x + block_x && i < N; i++) {
    for (int j = start_y; j < start_y + block_y && j < M; j++) {
      
      if (A[i][j] == empty)
        continue;

      if (A[i][j] == dead) {
        local_buff[i][j] = dead_twice;
        continue;
      }

      if (A[i][j] == dead_twice) {
        local_buff[i][j] = empty;
        continue;
      }

      rand_int = rand() % 10000;

      bool has_infetected_nearby = (
          (A[i - 1][j] == infected) ||
          (A[i + 1][j] == infected) ||
          (j > 0 && A[i][j - 1] == infected) ||
          (j < M - 1 && A[i][j + 1] == infected)
      );

      bool has_dead_nearby = (
          (A[i - 1][j] == dead) ||
          (A[i + 1][j] == dead) ||
          (j > 0 && A[i][j - 1] == dead) ||
          (j < M - 1 && A[i][j + 1] == dead)
      );

      if (A[i][j] == healthy && (has_infetected_nearby || has_dead_nearby)) {
        local_buff[i][j] = infected;
        continue;
      }

      if (rand_int - 999 <= 0) {
        local_buff[i][j] = healthy;
      } else if (rand_int >= 4000) {
        local_buff[i][j] = dead;
      } else {
        local_buff[i][j] = infected;
      }
    }
  }
}

void *mpi_worker(int **A, int N, int M, int rank, int size) {
  int limit = N * M;
  int rows_per_proc = N / size;
  
  if (N % size != 0) {
    if (rank == 0) fprintf(stderr, "Error: N must be divisible by number of processes\n");
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  // Allocate local matrix with 2 ghost rows (0 and rows_per_proc+1)
  int local_N = rows_per_proc;
  int *local_data = malloc(sizeof(int) * (local_N + 2) * M);
  int **local_A = malloc(sizeof(int *) * (local_N + 2));
  for (int i = 0; i < local_N + 2; i++) {
    local_A[i] = &local_data[i * M];
  }

  // Allocate local write buffer
  int *buff_data = malloc(sizeof(int) * (local_N + 2) * M);
  int **local_buff = malloc(sizeof(int *) * (local_N + 2));
  for (int i = 0; i < local_N + 2; i++) {
    local_buff[i] = &buff_data[i * M];
  }

  // Scatter data from Rank 0 to all processes
  // Send N/size rows. Rank 0 sends from A[0]. Receivers put into local_A[1]
  int *sendbuf = (rank == 0) ? A[0] : NULL;
  MPI_Scatter(sendbuf, rows_per_proc * M, MPI_INT, 
              local_A[1], rows_per_proc * M, MPI_INT, 
              0, MPI_COMM_WORLD);

  // Initialize ghost rows to empty/safe values (optional but good practice)
  memset(local_A[0], 0, M * sizeof(int));
  memset(local_A[local_N + 1], 0, M * sizeof(int));

  if (rank == 0) {
    printf("Starting MPI worker: N=%d, M=%d, rows_per_proc=%d\n", N, M, rows_per_proc);
    fflush(stdout);
  }
  
  while(true) {
    // 1. Exchange Ghost Cells
    MPI_Request reqs[4];
    int req_count = 0;

    // Send row 1 to Top (rank-1), Recv row 0 from Top
    if (rank > 0) {
      MPI_Irecv(local_A[0], M, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
      MPI_Isend(local_A[1], M, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
    } else {
      // Top boundary (Rank 0): Ghost row 0 is dead/empty
      // Already zeroed out, or we can enforce it if needed
    }

    // Send row local_N to Bottom (rank+1), Recv row local_N+1 from Bottom
    if (rank < size - 1) {
      MPI_Irecv(local_A[local_N + 1], M, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &reqs[req_count++]);
      MPI_Isend(local_A[local_N], M, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &reqs[req_count++]);
    } else {
      // Bottom boundary (Rank size-1): Ghost row local_N+1 is dead/empty
    }

    MPI_Waitall(req_count, reqs, MPI_STATUSES_IGNORE);

    // 2. Compute Local
    // Initialize local_buff with current state (copy)
    for(int i=1; i<=local_N; i++) {
        memcpy(local_buff[i], local_A[i], M * sizeof(int));
    }

    // Process rows 1 to local_N
    // Note: mpi_process_block expects global indices if we use them for logic, 
    // but here we pass local pointers.
    // The logic inside uses A[i-1] etc, which works with our ghost rows.
    // We pass start_x=1, block_x=local_N.
    mpi_process_block(local_A, local_buff, local_N + 2, M, 1, 0, local_N, M);

    // 3. Check Termination (Distributed)
    bool local_all_one = true;
    bool local_all_zero = true;
    
    // Check only valid data rows (1 to local_N)
    for (int i = 1; i <= local_N; i++) {
        for (int j = 0; j < M; j++) {
            if (local_buff[i][j] != 1) local_all_one = false;
            if (local_buff[i][j] != 0) local_all_zero = false;
        }
    }

    bool global_all_one, global_all_zero;
    MPI_Allreduce(&local_all_one, &global_all_one, 1, MPI_C_BOOL, MPI_LAND, MPI_COMM_WORLD);
    MPI_Allreduce(&local_all_zero, &global_all_zero, 1, MPI_C_BOOL, MPI_LAND, MPI_COMM_WORLD);

    if (global_all_one || global_all_zero || g_iteration >= limit) {
      if (rank == 0) {
        printf("Terminating: all_one=%d, all_zero=%d, iter=%d\n", global_all_one, global_all_zero, g_iteration);
      }
      break;
    }

    // 4. Swap Buffers
    int **temp = local_A;
    local_A = local_buff;
    local_buff = temp;
    
    // Also swap underlying data pointers if needed? 
    // Since we allocated contiguous blocks and set pointers, swapping the double pointers 
    // is enough IF we access via local_A[i].
    // But wait, local_A[i] points to &local_data[i*M].
    // If we swap local_A and local_buff, local_A[i] will point to &buff_data[i*M].
    // This is correct.

    g_iteration++;
  }
  
  // Gather results back to Rank 0
  int *recvbuf = (rank == 0) ? A[0] : NULL;
  MPI_Gather(local_A[1], rows_per_proc * M, MPI_INT,
             recvbuf, rows_per_proc * M, MPI_INT,
             0, MPI_COMM_WORLD);

  if (rank == 0) {
    printf("MPI completed %d iterations\\n", g_iteration);
  }

  free(local_data);
  free(local_A);
  free(buff_data);
  free(local_buff);
  
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
      if (local_num_cli <= 0) {
        num_cli = 8;
        continue;
      }
      num_cli = local_num_cli;
    } else if (strcmp(argv[i], "--block_x")==0) {
      const int local_block_size = atoi(argv[i + 1]);
      if (local_block_size <= 0) {
        block_x = 8;
        continue;
      }
      block_x = local_block_size;
    } else if (strcmp(argv[i], "--block_y") == 0) {
      const int local_block_size = atoi(argv[i + 1]);
      if (local_block_size <= 0) {
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
  int N, M, **A = NULL, rank, size;

  if (!handle_args(argc, argv)) {
    return EXIT_FAILURE;
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    // First, read N and M from file header
    FILE *fp = fopen(input_path, "r");
    if (fp == NULL) {
      perror("Error opening input file");
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    if (fscanf(fp, "%d %d\n", &N, &M) != 2) {
      fprintf(stderr, "Error reading matrix dimensions\n");
      fclose(fp);
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    fclose(fp);

    // Allocate matrix A contiguously for MPI_Scatter
    int *data = malloc(sizeof(int) * N * M);
    A = malloc(sizeof(int *) * N);
    for (int i = 0; i < N; i++) {
      A[i] = &data[i * M];
    }

    // input
    read_input(input_path, A, &N, &M);
    
    // init buff_write (not used in main, but kept if needed for logic outside worker)
    // Actually, buff_write is global and used in mpi_process_block, 
    // but mpi_worker will allocate its own local buffers.
    // We can leave the global buff_write unused or remove it later.
  }

  // Broadcast dimensions to all processes
  MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  measure_fn_mpi_time(mpi_worker, A, N, M, rank, size, size);
  MPI_Finalize();

  if (rank == 0 && A) {
    free(A[0]); // free data
    free(A);    // free pointers
  }
  return 0;
}
