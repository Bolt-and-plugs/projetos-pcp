#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <time.h>
#include <curand.h>        
#include <curand_kernel.h> 
#include "utils.h" 

#define ITERATION_END 404
#define ITERATION_CONTINUE 101

#define CURADA 1
#define CONTAMINADA -1
#define MORTA -2
#define NINGUEM 0

enum { NS_PER_SECOND = 1000000000 };

#define GET_STATE(A_current_ptr, R, C) \
    (((R) >= 0 && (R) < N && (C) >= 0 && (C) < M) ? A_current_ptr[(R) * M + (C)] : 0)

void print_matrix(int **A, int N, int M) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }
}

__host__ void h_print_flat_matrix(int *A, int N, int M) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            printf("%d ", A[i * M + j]);
        }
        printf("\n");
    }
}

__device__ void d_print_flat_matrix(int *A, int N, int M) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            printf("%d ", A[i * M + j]);
        }
        printf("\n");
    }
}

void read_input(const char *path, int ***A, int *N, int *M) {
    FILE *fp = fopen(path, "r");


    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    if (fscanf(fp, "%d %d", N, M) != 2) {
        fprintf(stderr, "Error reading N, M\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    *A = (int**)malloc(sizeof(int*) * (*N));
    for (int i = 0; i < *N; i++) {
        (*A)[i] = (int*)malloc(sizeof(int) * (*M));
        if ((*A)[i] == NULL) {
            perror("Could not allocate A row");
            // free previously allocated rows
            for (int j = 0; j < i; j++) {
                free((*A)[j]);
            }
            free((*A));
            exit(1);
        }
    }

    for (int i = 0; i < *N; i++) {
        for (int j = 0; j < *M; j++) {
            if (fscanf(fp, "%d", &(*A)[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
                fclose(fp);
                exit(EXIT_FAILURE);
            }
        }
    }

    // printing the input matrix for verification
    //print_matrix(*A, *N, *M);

    fclose(fp);
}
__global__ void verify_end_kernel(int *d_current, int N, int M, int *d_end_condition) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = N * M;

    if (idx < total_elements) {
        // Se a condição de CONTINUAR já foi sinalizada por outra thread, pare.
        if (*d_end_condition == ITERATION_CONTINUE) {
            return;
        }

        // Verifica a condição que REQUER CONTINUIDADE: presença de CONTAMINADA
        if (d_current[idx] == CONTAMINADA) {
            // Sinaliza que a iteração DEVE CONTINUAR
            atomicExch(d_end_condition, ITERATION_CONTINUE);
        }
    }
}
void write_output(const char *output_path, int **A, int N, int M, int total_dead) {
    long total_survivors = 0; 
    long total_nobody = 0;
    int total_deaths = 0;
    
    // printing the final matrix for verification
    //print_matrix(A, N, M);
    
    // Conta apenas sobreviventes (vivos curados ou contaminados)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            if (A[r][c] == CURADA || A[r][c] == CONTAMINADA) {
                total_survivors++;
            }
            else if (A[r][c] == NINGUEM) {
                total_nobody++;
            }
            else {
                total_deaths++;
            }
        }
    }

    FILE *fp = fopen(output_path, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }

    fprintf(fp, "%d %ld\n", total_dead, total_survivors);
    
    fclose(fp);
    printf("Resultados escritos em: %s\n", output_path);
}    

__global__ void execute_iter(int *A_current, int *A_next, int N, int M, curandState_t *states, int *total_dead) {
    int total_elements = N * M;
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    
    if (index >= total_elements) return;

    int r = index / M; 
    int c = index % M; 

    int current_state = A_current[index];
    int next_state = current_state; 

    if (current_state == CURADA) { 
        int neighbor_up    = GET_STATE(A_current, r - 1, c);
        int neighbor_down  = GET_STATE(A_current, r + 1, c);
        int neighbor_left  = GET_STATE(A_current, r, c - 1);
        int neighbor_right = GET_STATE(A_current, r, c + 1);
        if (neighbor_up < 0 || neighbor_down < 0 || neighbor_left < 0 || neighbor_right < 0) {
            next_state = CONTAMINADA; 
        }
    }
    else if (current_state == CONTAMINADA) {
        int element_id = r * M + c;

        curandState_t local = states[element_id];
        unsigned int random_val = curand(&local);
        states[element_id] = local;
        
        int x = random_val % 10000;
        
        if (x <= 999) { // 0.1 
            next_state = CURADA; 
        } else if (x <= 3999) { // 0.3
            next_state = CONTAMINADA; 
        } else { // 0.6 
            next_state = MORTA;
            // Incrementa o contador de mortos atomicamente (thread-safe)
            atomicAdd(total_dead, 1);
        }
    }
    else if(current_state == MORTA){
    next_state = NINGUEM;
    }
    A_next[index] = next_state;
}

__global__ void setup_kernel(curandState_t *state, unsigned long seed, int N_total) {
    int id = threadIdx.x + blockIdx.x * blockDim.x;
    if (id < N_total) {
        curand_init(seed, id, 0, &state[id]);
    }
}

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
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


int main(int argc, char **argv){
    
    srand(time(NULL)); 
    if(argc < 4){
        puts("Invalid Number of arguments: <file_path> <num_threads> <num_blocks>");
        return -1;
    }

    const char *file_path = argv[1];
    int num_threads = atoi(argv[2]);
    int num_blocks = atoi(argv[3]);
    // const char *device = argv[4]; 
    const char *output_file = "outputs/gpu_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);
    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); //matriz interpretada como vetor
    int *h_next_flat = (int*)malloc(size); 
    int total_dead = 0;
    int *d_total_dead;
    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            if(space[i][j]==MORTA) total_dead++;
        }
    }

    if (h_current_flat == NULL || h_next_flat == NULL) {
        fprintf(stderr, "Host memory allocation failed\n");
        return -1;
    }

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            h_current_flat[r * M + c] = space[r][c]; //atribuindo valores para a matriz vetorizada
        }
    }

    int *d_current = NULL; 
    int *d_next = NULL;    
    int *d_end_condition;
    if (cudaMalloc((void **)&d_end_condition, sizeof(int)) != cudaSuccess) {
        printf("Error at allocating end condition memory on GPU\n");
        return -1;
}
    if (cudaMalloc((void **)&d_current, size) != cudaSuccess || 
        cudaMalloc((void **)&d_next, size) != cudaSuccess || cudaMalloc((void **)&d_total_dead, (int)sizeof(int))!= cudaSuccess) {
        printf("Error at allocating memory on GPU\n");
        return -1;
    }
    
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_next, d_current, size, cudaMemcpyDeviceToDevice);


    cudaMemcpy(d_total_dead, &total_dead, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice); //copiando o conteúdo para a GPU

    curandState_t *d_states;
    size_t state_size = N*M * sizeof(curandState_t);

    int i = 0;
    if (cudaMalloc((void **)&d_states, state_size) != cudaSuccess) {
        printf("Error at allocating cuRAND states on GPU\n");
        return -1;
    }   

    
    unsigned long initial_seed = 0; 
    
    // Calcula o total de threads que serão lançadas
    int total_threads = num_blocks * num_threads;
    int total_elements = N * M;
    
    // Verifica se a configuração é insuficiente
    if (total_threads < total_elements) {
        printf("AVISO: Configuração insuficiente!\n");
        printf("  Total de elementos: %d\n", total_elements);
        printf("  Total de threads: %d blocos × %d threads = %d threads\n", 
               num_blocks, num_threads, total_threads);
        printf("  %d elementos NÃO seriam processados!\n", total_elements - total_threads);
    } else {
        printf("Configuração: %d blocos × %d threads = %d threads (suficiente para %d elementos)\n",
               num_blocks, num_threads, total_threads, total_elements);
    }
    
    // Usa num_blocks do usuário para setup_kernel
    setup_kernel<<<num_blocks, num_threads>>>(d_states, initial_seed, N*M);
    cudaDeviceSynchronize(); 

    //init
    FILE *time_file;
    const char *file_name = "time_related/gpu_time.dat";
    time_file = fopen(file_name, "a");
    struct timespec start, end, _time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int h_end_condition;
    for(i = 0; i < max_iter; i++){
        
        execute_iter<<<num_blocks, num_threads>>>(d_current, d_next, N, M, d_states, d_total_dead);
        
        h_end_condition = ITERATION_END; // 0
        cudaMemcpy(d_end_condition, &h_end_condition, sizeof(int), cudaMemcpyHostToDevice);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            fprintf(stderr, "CUDA Kernel launch failed: %s\n", cudaGetErrorString(err));
            break; 
        }
        verify_end_kernel<<<num_blocks, num_threads>>>(d_next, N, M, d_end_condition);
        cudaDeviceSynchronize(); 


        cudaMemcpy(&h_end_condition, d_end_condition, sizeof(int), cudaMemcpyDeviceToHost);

        // 5. Verifica a condição de parada no Host
        if(h_end_condition == ITERATION_END) { 
            printf("Simulation ended after %d iterations due to population stability.\n", i + 1);
            break; 
        }

        int *temp = d_current; //atualização para iteração atual na GPU. 
        d_current = d_next;
        d_next = temp; 
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    sub_timespec(start, end, &_time);
     
    printf("Time elapsed: %d.%.9ld | Matrix Size: %d x %d | Threads: %d | Blocks %d\n" , (int)_time.tv_sec, _time.tv_nsec, N, M, num_threads, num_blocks);
    fprintf(time_file,"Time elapsed: %d.%.9ld | Matrix Size:%d x %d | Threads: %d | Blocks %d\n" , (int)_time.tv_sec, _time.tv_nsec, N, M, num_threads, num_blocks);

    fclose(time_file);
    cudaMemcpy(h_next_flat, d_current, size, cudaMemcpyDeviceToHost); //Copiando a matriz de iteração atual para o Host

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            space[r][c] = h_next_flat[r * M + c];
        }
    }
    if (i == max_iter) {
        printf("Simulation ended after %d iterations (maximum limit reached).\n", max_iter);
    }

    // Copia o total de mortos acumulado durante todas as iterações
    cudaMemcpy(&total_dead, d_total_dead, sizeof(int), cudaMemcpyDeviceToHost);
    printf("Total de mortes durante a simulação: %d\n", total_dead);

    write_output(output_file, space, N, M, total_dead);
    

    for (int r = 0; r < N; r++) free(space[r]);
    free(space);
    free(h_current_flat);
    free(h_next_flat);

    cudaFree(d_current);
    cudaFree(d_next);
    cudaFree(d_states);
    cudaFree(d_total_dead);
    return 0;
}