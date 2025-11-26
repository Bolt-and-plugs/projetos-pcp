#define _POSIX_C_SOURCE 199309L
#include "utils.h"
#include "math.h"
#include "time.h"
#include <cuda_runtime.h>
enum { NS_PER_SECOND = 1000000000 };


void write_output(const char *output_path, int **A, int N, int M, int total_dead) {
    long total_survivors = 0; 
    long total_nobody = 0;
    int total_deaths = 0;
        
    // Conta apenas sobreviventes (vivos curados ou contaminados)
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            if (A[r][c] == HEALTHY || A[r][c] == INFECTED) {
                total_survivors++;
            }
            else if (A[r][c] == EMPTY) {
                total_nobody++;
            }
            else {
                total_deaths++;
            }
        }
    }

    FILE *fp = fopen(output_path, "a");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }

    fprintf(fp, "%d %ld\n", total_dead, total_survivors);
    
    fclose(fp);
    printf("Resultados escritos em: %s\n", output_path);
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

    fclose(fp);
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

void init_clock(struct timespec *start) {
    clock_gettime(CLOCK_MONOTONIC, start);
}

void end_clock(struct timespec start, int N, int M, int threads, int blocks) {
    struct timespec end, _time;
    clock_gettime(CLOCK_MONOTONIC, &end);
    sub_timespec(start, end, &_time);

    FILE *time_file = fopen("time_related/gpu_time.dat", "a");
    if (time_file != NULL) {
        printf("Time elapsed: %d.%.9ld | Matrix Size: %d x %d | Block Config: %d threads, %d blocks\n" , 
            (int)_time.tv_sec, _time.tv_nsec, N, M, threads, blocks);
        fprintf(time_file,"Time elapsed: %d.%.9ld | Matrix Size:%d x %d | Block Config: %d threads, %d blocks\n" , 
                (int)_time.tv_sec, _time.tv_nsec, N, M, threads, blocks);
        fclose(time_file);
    }
}