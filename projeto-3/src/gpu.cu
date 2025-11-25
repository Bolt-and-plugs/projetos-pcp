#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <curand.h>        
#include <curand_kernel.h> 
#include "utils.h" 

enum { NS_PER_SECOND = 1000000000 };

#define GET_STATE(A_current_ptr, R, C) \
    (((R) >= 0 && (R) < N && (C) >= 0 && (C) < M) ? A_current_ptr[(R) * M + (C)] : 0)

__global__ void verify_end_kernel(int *d_current, int N, int M, int *d_end_condition) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int i = idx; i < N * M; i += stride) {
        // Se a condição de CONTINUAR já foi sinalizada por outra thread, pare.
        if (*d_end_condition == ITERATION_CONTINUE) {
            return;
        }

        // Verifica a condição que REQUER CONTINUIDADE: presença de INFECTED
        if (d_current[i] == INFECTED) {
            // Sinaliza que a iteração DEVE CONTINUAR
            atomicExch(d_end_condition, ITERATION_CONTINUE);
            return; // Otimização: se achou um, não precisa verificar o resto desta thread
        }
    }
}    

__global__ void execute_iter(int *A_current, int *A_next, int N, int M, curandState_t *states, int *total_dead) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int index = idx; index < N * M; index += stride) {
        int r = index / M;
        int c = index % M;

        int current_state = A_current[index];
        int next_state = current_state; 

        if (current_state == HEALTHY) { 
            int neighbor_up    = GET_STATE(A_current, r - 1, c);
            int neighbor_down  = GET_STATE(A_current, r + 1, c);
            int neighbor_left  = GET_STATE(A_current, r, c - 1);
            int neighbor_right = GET_STATE(A_current, r, c + 1);
            if (neighbor_up < 0 || neighbor_down < 0 || neighbor_left < 0 || neighbor_right < 0) {
                next_state = INFECTED; 
            }
        }
        else if (current_state == INFECTED) {
            // Usa o estado do gerador correspondente à célula
            curandState_t local = states[index];
            unsigned int random_val = curand(&local);
            states[index] = local; // Atualiza o estado na memória global
            
            int x = random_val % 10000;
            
            if (x <= 999) { // 0.1 
                next_state = HEALTHY; 
            } else if (x <= 3999) { // 0.3
                next_state = INFECTED; 
            } else { // 0.6 
                next_state = DEAD;
                // Incrementa o contador de mortos atomicamente (thread-safe)
                atomicAdd(total_dead, 1);
            }
        }
        else if(current_state == DEAD){
            next_state = EMPTY;
        }
        A_next[index] = next_state;
    }
}

__global__ void setup_kernel(curandState_t *state, unsigned long seed, int N, int M) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int id = idx; id < N * M; id += stride) {
        curand_init(seed, id, 0, &state[id]);
    }
}

int main(int argc, char **argv){
    
    srand(time(NULL)); 
    
    if(argc < 4){
        puts("Usage: ./gpu <file_path> <num_threads> <num_blocks>");
        return -1;
    }

    const char *file_path = argv[1];
    int num_threads = atoi(argv[2]);
    int num_blocks = atoi(argv[3]);
    const char *output_file = "outputs/gpu_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);

    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); // host current matrix as flat array
    int *h_next_flat = (int*)malloc(size); // host next matrix as flat array
    
    if (h_current_flat == NULL || h_next_flat == NULL) {
        fprintf(stderr, "Host memory allocation failed\n");
        return -1;
    }
    
    int total_dead = 0;
    int *d_total_dead;

    // conta total de mortos iniciais
    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            if(space[i][j]==DEAD) total_dead++;
        }
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
        cudaMalloc((void **)&d_next, size) != cudaSuccess || 
        cudaMalloc((void **)&d_total_dead, (int) sizeof(int))!= cudaSuccess) {
        printf("Error at allocating memory on GPU\n");
        return -1;
    }
    
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice); // copia matriz inicial para a GPU
    cudaMemcpy(d_next, d_current, size, cudaMemcpyDeviceToDevice); // copia current para next na GPU

    cudaMemcpy(d_total_dead, &total_dead, sizeof(int), cudaMemcpyHostToDevice); // copia total_dead inicial para a GPU
    cudaMemcpy(d_current, h_current_flat, size, cudaMemcpyHostToDevice); //copiando o conteúdo para a GPU

    curandState_t *d_states;
    size_t state_size = N*M * sizeof(curandState_t);

    int i = 0;
    if (cudaMalloc((void **)&d_states, state_size) != cudaSuccess) {
        printf("Error at allocating cuRAND states on GPU\n");
        return -1;
    }   
    
    unsigned long initial_seed = 0; 
    
    dim3 threadsPerBlock;
    dim3 numBlocksGrid;
    int total_elements = N * M;

    // ============ MODO MANUAL (1D) ============
    
    // Lógica para interpretar casos como "1024 16" significando 1024 threads totais em 16 blocos
    // (ou seja, 64 threads por bloco), ao invés de 1024 threads POR bloco.
    
    int threads_arg = num_threads;
    int blocks_arg = num_blocks;
    
    // Se threads_arg > 1024 (limite de hardware por bloco) OU
    // se threads_arg é divisível por blocks_arg e o resultado é um tamanho de bloco válido...
    // Vamos assumir a interpretação "Total de Threads" vs "Blocos"
    
    if (threads_arg * blocks_arg != N * M && (threads_arg > 1024 || (threads_arg % blocks_arg == 0 && threads_arg / blocks_arg <= 1024 && threads_arg / blocks_arg > 0))) {
            // Interpretação: threads_arg é o TOTAL de threads
            int t_per_b = threads_arg / blocks_arg;
            if (t_per_b == 0) t_per_b = 1; // segurança
            
            threadsPerBlock = dim3(t_per_b);
            numBlocksGrid = dim3(blocks_arg);
            
            printf("========================================\n");
            printf("MODO 1D MANUAL (Interpretado: Total Threads / Blocos)\n");
            printf("  Argumentos: %d threads totais, %d blocos\n", threads_arg, blocks_arg);
            printf("  Configuração Real: %d threads/bloco × %d blocos\n", t_per_b, blocks_arg);
    } else {
            // Interpretação Padrão: threads_arg é threads POR bloco
            threadsPerBlock = dim3(threads_arg);
            numBlocksGrid = dim3(blocks_arg);
            
            printf("========================================\n");
            printf("MODO 1D MANUAL (Interpretado: Threads por Bloco × Blocos)\n");
            printf("  Configuração: %d threads/bloco × %d blocos\n", threads_arg, blocks_arg);
    }

    int total_threads_configured = threadsPerBlock.x * numBlocksGrid.x;
    printf("  Matriz: %d × %d = %d elementos\n", N, M, total_elements);
    printf("  Total de Threads na Grid: %d\n", total_threads_configured);
    
    if (total_threads_configured < total_elements) {
        printf("  ⚠ Threads insuficientes para cobrir a matriz em paralelo.\n");
        printf("  ✓ Grid-Stride Loop ativado: cada thread processará múltiplos elementos.\n");
    } else {
        printf("  ✓ Cobertura total em paralelo.\n");
    }
    printf("========================================\n\n");
    
    // Inicializa estados cuRAND
    setup_kernel<<<numBlocksGrid, threadsPerBlock>>>(d_states, initial_seed, N, M);
    cudaDeviceSynchronize(); 

    //init
    struct timespec start;
    init_clock(&start);
    int h_end_condition;
    for(i = 0; i < max_iter; i++){
        
        // Usa configuração 2D
        execute_iter<<<numBlocksGrid, threadsPerBlock>>>(d_current, d_next, N, M, d_states, d_total_dead);
        
        h_end_condition = ITERATION_END; // 0
        cudaMemcpy(d_end_condition, &h_end_condition, sizeof(int), cudaMemcpyHostToDevice);
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess) {
            fprintf(stderr, "CUDA Kernel launch failed: %s\n", cudaGetErrorString(err));
            break; 
        }
        verify_end_kernel<<<numBlocksGrid, threadsPerBlock>>>(d_next, N, M, d_end_condition);
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

    end_clock(start, N, M, threadsPerBlock.x, numBlocksGrid.x);
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