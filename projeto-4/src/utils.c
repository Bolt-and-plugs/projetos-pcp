#include "utils.h"
#include "math.h"
#include "time.h"

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

static void write_x_to_file(long double *x, int N, bool omp) {
  char buffer[256];
  if(omp)
    sprintf(buffer, "outputs/omp-mat-%d-%d.dat", N, N);
  else 
    sprintf(buffer, "outputs/seq-mat-%d-%d.dat", N, N);
  FILE *fp = fopen(buffer, "w");

  if (!fp) {
    printf("Arquivo mal formado %s\n", buffer);
    return;
  }

  for (int i = 0; i < N; i++)
    fprintf(fp, "[%.4Lf]\t", x[i]);

  fclose(fp);
}


void read_input(const char *path, int **A, int *N, int *M) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    
    exit(EXIT_FAILURE);
  }

  if (fscanf(fp, "%d %d", N, M) != 1) {
    fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      if (fscanf(fp, "%d", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }

}


bool write_file(const char *path, int **A, int N, int M) {
  FILE *fp = fopen(path, "w");

  if (!fp) {
    fprintf(stderr, "Could not write file down");
    fclose(fp);
    return false;
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      if(fprintf(fp, "%d", A[i][j]) < 0) {
        fprintf(stderr, "Could not write file down");
        fclose(fp);
        return false;
      }
    }
    if(fprintf(fp, "\n") < 0) {

      fprintf(stderr, "Could not write file down");
      fclose(fp);
      return false;
    }
  }

 return true;
}

long double *arr_norm(long double *x1, long double *x2, int N) {
  long double *x3 = malloc(sizeof(long double) * N);
  for (int i = 0; i < N; i++) {
    x3[i] = fabsl(x2[i] - x1[i]);
  }

  return x3;
}

void print_arr(long double *x, int N) {
  for (int i = 0; i < N; i++) {
    printf("[%Lf]\t", x[i]);
  }
  puts("");
}

void print_mat(long double **x, int N, int M) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      printf("[%Lf]\t", x[i][j]);
    }
    puts("");
  }
}
