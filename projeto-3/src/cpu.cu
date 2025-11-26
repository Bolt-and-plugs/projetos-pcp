#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>
#include "utils.h" 

#define GET_STATE(R, C) \
    (((R) >= 0 && (R) < N && (C) >= 0 && (C) < M) ? A_current[(R) * M + (C)] : 0)

int verify_end_population(int **A, int N, int M) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            if (A[i][j] == INFECTED || A[i][j] == DEAD) {
                return ITERATION_CONTINUE;
            }
        }
    }
    return ITERATION_END;
}

__host__ void execute_iter_serial(int *A_current, int *A_next, int N, int M, int *total_dead) {
    int total_elements = N * M;
    
    // Processa todos os elementos sequencialmente
    for (int index = 0; index < total_elements; index++) {
        int r = index / M; 
        int c = index % M; 

        int current_state = A_current[index];
        int next_state = current_state; 

        if (current_state == HEALTHY) { 
            int neighbor_up    = GET_STATE(r - 1, c);
            int neighbor_down  = GET_STATE(r + 1, c);
            int neighbor_left  = GET_STATE(r, c - 1);
            int neighbor_right = GET_STATE(r, c + 1);

            if (neighbor_up < 0 || neighbor_down < 0 || neighbor_left < 0 || neighbor_right < 0) {
                next_state = INFECTED; 
            }
        }
        else if (current_state == INFECTED) {


            
            int x = rand()% 10000;
            
            if (x <= 999) { // 0.1 
                next_state = HEALTHY; 
            } else if (x <= 3999) { // 0.3
                next_state = INFECTED; 
            } else { // 0.6
                next_state = DEAD;
                (*total_dead)++;
            }
        }
        else if(current_state == DEAD){
            next_state = EMPTY;
        }

        A_next[index] = next_state;
    }
}

enum { NS_PER_SECOND = 1000000000 };

int main(int argc, char **argv){
    
    srand(time(NULL)); 
    if(argc < 2){
        puts("Invalid Number of arguments: <file_path> <num_threads> <num_blocks>");
        return -1;
    }

    const char *file_path = argv[1];
    // const char *device = argv[4]; 
    const char *output_file = "outputs/cpu_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);
    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); //matriz interpretada como vetor
    int *h_next_flat = (int*)malloc(size); 
    int total_dead = 0;
    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            if(space[i][j]==DEAD) total_dead++;
        }
    }

    if (h_current_flat == NULL || h_next_flat == NULL ) {
        fprintf(stderr, "Host memory allocation failed\n");
        return -1;
    }

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            h_current_flat[r * M + c] = space[r][c]; //atribuindo valores para a matriz vetorizada
        }
    }

    int *d_current = (int*)malloc(size); 
    int *d_next = (int*)malloc(size);    

    if (d_current == NULL || d_next == NULL) {
        printf("Error at allocating memory\n");
        return -1;
    }
    
    memcpy(d_current, h_current_flat, size);
    memcpy(d_next, d_current, size);

    memcpy(d_current, h_current_flat, size); //copiando o conteúdo para a GPU

    FILE *time_file;
    const char *file_name = "time_related/cpu_time.dat";
    time_file = fopen(file_name, "a");
    struct timespec start, end, _time;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int i;
    for(i = 0; i < max_iter; i++){
        
        execute_iter_serial(d_current, d_next, N, M, &total_dead);



        memcpy(h_next_flat, d_next, size);
        

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < M; c++) {
                space[r][c] = h_next_flat[r * M + c];
            }
        }
        
        if(verify_end_population(space, N, M) == ITERATION_END) { //verificação se acabou ou não as iterações
            printf("Simulation ended after %d iterations due to population stability.\n", i + 1);
            break; 
        }

        int *temp = d_current; //atualização para iteração atual na GPU. 
        d_current = d_next;
        d_next = temp; 
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    sub_timespec(start, end, &_time);
     
    printf("Time elapsed: %d.%.9ld | Matrix Size: %d x %d\n", (int)_time.tv_sec, _time.tv_nsec, N, M);
    fprintf(time_file,"Time elapsed: %d.%.9ld | Matrix Size:%d x %d\n" , (int)_time.tv_sec, _time.tv_nsec, N, M);

    fclose(time_file);


    if (i == max_iter) {
        printf("Simulation ended after %d iterations (maximum limit reached).\n", max_iter);
    }

    // Copia o total de mortos acumulado durante todas as iterações
    printf("Total de mortes durante a simulação: %d\n", total_dead);

    write_output(output_file, space, N, M, total_dead);
    

    for (int r = 0; r < N; r++) free(space[r]);
    free(space);
    free(h_current_flat);
    free(h_next_flat);

    free(d_current);
    free(d_next);

    return 0;
}