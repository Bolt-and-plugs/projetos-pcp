#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <time.h>
#include <curand.h>        
#include <curand_kernel.h> 
#include "utils.h" 

#define ITERATION_END 404
#define ITERATION_CONTINUE 101



#define GET_STATE(R, C) \
    (((R) >= 0 && (R) < N && (C) >= 0 && (C) < M) ? A_current[(R) * M + (C)] : 0)


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
    fclose(fp);
}

int verify_end_population(int **A, int N, int M) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            if (A[i][j] != 0) {
                return ITERATION_CONTINUE;
            }
        }
    }
    return ITERATION_END;
}


void write_output(const char *output_path, int **A, int N, int M, int total_dead) {
    long total_survivors = 0; 
    

    
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            if (A[r][c] == 1) {
                total_survivors++;
            } else if (A[r][c] == -1) {
                total_survivors++; 
            } else if (A[r][c] == -2) {
                total_dead++; 
            }
        }
    }

    FILE *fp = fopen(output_path, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }
    

    
    fprintf(fp, "Total de Sobreviventes (Saudáveis ou Contaminados): %ld\n", total_survivors);

    fprintf(fp, "Total de Pessoas Mortas na Matriz Final: %d\n", total_dead); 
    
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

    if (current_state == 1) { 
        int neighbor_up    = GET_STATE(r - 1, c);
        int neighbor_down  = GET_STATE(r + 1, c);
        int neighbor_left  = GET_STATE(r, c - 1);
        int neighbor_right = GET_STATE(r, c + 1);

        if (neighbor_up <= -1 || neighbor_down <= -1 ||
            neighbor_left <= -1 || neighbor_right <= -1) {
            
            next_state = -1; 
        }
    }
    else if (current_state == -1) {
        int element_id = r * M + c;   
        curandState_t local = states[element_id];
        unsigned int random_val = curand(&local);
        states[element_id] = local;
        int x = random_val % 10000;
        if (x <= 999) { // 0.1 
            next_state = 1; 
        } else if (x <= 3999) { // 0.3
            next_state = -1; 
        } else { // 0.6 
            next_state = -2; 
        }
    }
    else if (current_state == -2) {
        atomicAdd(total_dead, 1);
        next_state = 0; 
    }

    A_next[index] = next_state;
}

__global__ void setup_kernel(curandState_t *state, unsigned long seed, int N_total) {
        int id = threadIdx.x + blockIdx.x * blockDim.x;
        if (id < N_total) {
            curand_init(seed, id, 0, &state[id]);
        }
    }

int main(int argc, char **argv){
    if(argc < 5){
        puts("Invalid Number of arguments: <file_path> <num_threads> <num_blocks> <device>");
        return -1;
    }


    const char *file_path = argv[1];
    int num_threads = atoi(argv[2]);
    int num_blocks = atoi(argv[3]);
    // const char *device = argv[4]; 
    const char *output_file = "simulation_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);
    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); //matriz interpretada como vetor
    int *h_next_flat = (int*)malloc(size); 
    int total_dead = 0;
    int *d_total_dead;
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

    if (cudaMalloc((void **)&d_current, size) != cudaSuccess || 
        cudaMalloc((void **)&d_next, size) != cudaSuccess || cudaMalloc((void **)&d_total_dead, (int)sizeof(int))!= cudaSuccess) {
        printf("Error at allocating memory on GPU\n");
        return -1;
    }

    cudaMemset(d_total_dead, 0, sizeof(int));
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice); //copiando o conteúdo para a GPU
    curandState_t *d_states;
    size_t state_size = N*M * sizeof(curandState_t);

    int i = 0;
    unsigned int seed;
    if (cudaMalloc((void **)&d_states, state_size) != cudaSuccess) {
        printf("Error at allocating cuRAND states on GPU\n");
        return -1;
    }


    
    unsigned long initial_seed = 42; 
    
    int blocks_setup = (N*M + num_threads - 1) / num_threads;
    
    setup_kernel<<<blocks_setup, num_threads>>>(d_states, initial_seed, N*M);

    for(i = 0; i < max_iter; i++){
        
        execute_iter<<<num_blocks, num_threads>>>(d_current, d_next, N, M, d_states, d_total_dead);
        
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            fprintf(stderr, "CUDA Kernel launch failed: %s\n", cudaGetErrorString(err));

            break; 
        }

        cudaDeviceSynchronize(); 
        

        cudaMemcpy(h_next_flat, d_next, size, cudaMemcpyDeviceToHost); //Copiando a matriz de iteração atual para o Host
        

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < M; c++) {
                space[r][c] = h_next_flat[r * M + c];
            }
        }
        cudaMemcpy(&total_dead, d_total_dead, sizeof(int), cudaMemcpyDeviceToHost); //Copiando a matriz de iteração atual para o Host
        if(verify_end_population(space, N, M) == ITERATION_END) { //verificação se acabou ou não as iterações
            printf("Simulation ended after %d iterations due to population stability.\n", i + 1);
            break; 
        }

        int *temp = d_current; //atualização para iteração atual na GPU. 
        d_current = d_next;
        d_next = temp; 
    }

    if (i == max_iter) {
        printf("Simulation ended after %d iterations (maximum limit reached).\n", max_iter);
    }


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