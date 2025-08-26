#include "utils.h"

void print_arr(float **vec, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      printf("%f | ", vec[i][j]);
}

void read_input(const char *path, float **A, float *b) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  int n = 2000;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (fscanf(fp, "%f", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }

  print_arr(A, 2000);

  for (int i = 0; i < n; i++) {
    if (fscanf(fp, "%f", &b[i]) != 1) {
      fprintf(stderr, "Error reading vector data at b[%d]\n", i);
      fclose(fp);
      exit(EXIT_FAILURE);
    }
  }
}

bool write_file(const char *path, char *buffer) {
  return true;
}
