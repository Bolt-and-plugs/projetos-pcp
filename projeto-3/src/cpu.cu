#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>
#include "utils.h" 

#define ITERATION_END 404
#define ITERATION_CONTINUE 101

#define CURADA 1
#define CONTAMINADA -1
#define MORTA -2
#define NINGUEM 0


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

    // printing the input matrix for verification
    //print_matrix(*A, *N, *M);

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


__host__ void execute_iter_serial(int *A_current, int *A_next, int N, int M, int *total_dead) {
    int total_elements = N * M;
    
    // Processa todos os elementos sequencialmente
    for (int index = 0; index < total_elements; index++) {
        int r = index / M; 
        int c = index % M; 

        int current_state = A_current[index];
        int next_state = current_state; 

        if (current_state == CURADA) { 
            int neighbor_up    = GET_STATE(r - 1, c);
            int neighbor_down  = GET_STATE(r + 1, c);
            int neighbor_left  = GET_STATE(r, c - 1);
            int neighbor_right = GET_STATE(r, c + 1);

            if (neighbor_up < 0 || neighbor_down < 0 || neighbor_left < 0 || neighbor_right < 0) {
                next_state = CONTAMINADA; 
            }
        }
        else if (current_state == CONTAMINADA) {


            
            int x = rand()% 10000;
            
            if (x <= 999) { // 0.1 
                next_state = CURADA; 
            } else if (x <= 3999) { // 0.3
                next_state = CONTAMINADA; 
            } else { // 0.6
                next_state = MORTA;
                (*total_dead)++;
            }
        }
        else if(current_state == MORTA){
            next_state = NINGUEM;
        }

        A_next[index] = next_state;
    }
}

enum { NS_PER_SECOND = 1000000000 };

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
    if(argc < 2){
        puts("Invalid Number of arguments: <file_path> <num_threads> <num_blocks>");
        return -1;
    }

    const char *file_path = argv[1];
    // const char *device = argv[4]; 
    const char *output_file = "cpu_output.txt"; 

    int **space, N, M; 
    read_input(file_path, &space, &N, &M);
    int max_iter = N*M;
    int size = N*M*sizeof(int);

    int *h_current_flat = (int*)malloc(size); //matriz interpretada como vetor
    int *h_next_flat = (int*)malloc(size); 
    int total_dead = 0;
    for(int i=0;i<N;i++){
        for(int j=0;j<M;j++){
            if(space[i][j]==MORTA) total_dead++;
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