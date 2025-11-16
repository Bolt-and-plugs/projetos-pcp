#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h" 

#define ITERATION_END 404
#define ITERATION_CONTINUE 101

#define CURADA 1
#define CONTAMINADA -1
#define MORTA -2
#define NINGUEM 0



#define GET_STATE(R, C) \
    (((R) >= 0 && (R) < N && (C) >= 0 && (C) < M) ? A_current[(R) * M + (C)] : 0)

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
    print_matrix(*A, *N, *M);

    fclose(fp);
}

int verify_end_population(int **A, int N, int M) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            if (A[i][j] == CONTAMINADA || A[i][j] == MORTA) {
                return ITERATION_CONTINUE;
            }
        }
    }
    return ITERATION_END;
}


void write_output(const char *output_path, int **A, int N, int M) {
    long total_dead = 0; 
    long total_survivors = 0; 
    
    // printing the final matrix for verification
    print_matrix(A, N, M);
    
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            if (A[r][c] == CURADA || A[r][c] == CONTAMINADA) {
                total_survivors++;
            } else if (A[r][c] == MORTA || A[r][c] == NINGUEM) {
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

    fprintf(fp, "Total de Pessoas Mortas na Matriz Final: %ld\n", total_dead); 
    
    fclose(fp);
    printf("Resultados escritos em: %s\n", output_path);
}


__device__ int simple_lcg_rand(int index, unsigned int seed) {
    unsigned long long state = (unsigned long long)index * 1103515245 + seed;
    state = state * 1103515566 + 12345;
    return (int)((state >> 16) % 10000);
}


__global__ void execute_iter(int *A_current, int *A_next, int N, int M, unsigned int seed) {
    int total_elements = N * M;
    int index = threadIdx.x + blockIdx.x * blockDim.x;

    // printf("Thread %d in Block %d computing index %d\n", threadIdx.x, blockIdx.x, index);

    // printing the current state for debugging
    // d_print_flat_matrix(A_current, N, M);

    if (index >= total_elements) return;

    int r = index / M; 
    int c = index % M; 

    int current_state = A_current[index];
    int next_state = current_state; 

    if (current_state == CURADA) { 
        int neighbor_up    = GET_STATE(r - 1, c);
        int neighbor_down  = GET_STATE(r + 1, c);
        int neighbor_left  = GET_STATE(r, c - 1);
        int neighbor_right = GET_STATE(r, c + 1);

        if (neighbor_up < 0 || neighbor_down < 0 ||
            neighbor_left < 0 || neighbor_right < 0) {
            
            next_state = CONTAMINADA; 
        }
    }
    else if (current_state == -1) {
        int x = simple_lcg_rand(index, seed);

        if (x <= 999) { // 0.1 para a pessoa se curar
            next_state = CURADA; 
        } else if (x <= 3999) { // 0.3 para continuar contaminada
            next_state = CONTAMINADA; 
        } else { // 0.6 para a pessoa morrer    
            next_state = MORTA; 
        }
    }
    else if (current_state == MORTA) { 
        next_state = NINGUEM; 
    }

    A_next[index] = next_state;

    //printf("print d_next after kernel execution:\n");
    //d_print_flat_matrix(A_next, N, M);
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
    const char *output_file = "simulation_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);
    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); 
    int *h_next_flat = (int*)malloc(size); 
    
    if (h_current_flat == NULL || h_next_flat == NULL) {
        fprintf(stderr, "Host memory allocation failed\n");
        return -1;
    }

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            h_current_flat[r * M + c] = space[r][c];
        }
    }

    int *d_current = NULL; 
    int *d_next = NULL;    

    if (cudaMalloc((void **)&d_current, size) != cudaSuccess || 
        cudaMalloc((void **)&d_next, size) != cudaSuccess) {
        printf("Error at allocating memory on GPU\n");
        return -1;
    }
    
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_next, d_current, size, cudaMemcpyDeviceToDevice);

    int i = 0;
    unsigned int seed;
    
    for(i = 0; i < max_iter; i++){
        seed = (unsigned int)rand(); 
        
        
        execute_iter<<<num_blocks, num_threads>>>(d_current, d_next, N, M, seed);

        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            fprintf(stderr, "CUDA Kernel launch failed: %s\n", cudaGetErrorString(err));

            break; 
        }

        cudaDeviceSynchronize(); 

        cudaMemcpy(h_next_flat, d_next, size, cudaMemcpyDeviceToHost);

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < M; c++) {
                space[r][c] = h_next_flat[r * M + c];
            }
        }
        
        if(verify_end_population(space, N, M) == ITERATION_END) {
            printf("Simulation ended after %d iterations due to population stability.\n", i + 1);
            break; 
        }

        int *temp = d_current;
        d_current = d_next;
        d_next = temp; 
    }

    if (i == max_iter) {
        printf("Simulation ended after %d iterations (maximum limit reached).\n", max_iter);
    }

    write_output(output_file, space, N, M);
    
    for (int r = 0; r < N; r++) free(space[r]);
    free(space);
    free(h_current_flat);
    free(h_next_flat);

    cudaFree(d_current);
    cudaFree(d_next);

    return 0;
}