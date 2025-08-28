#include "utils.h"
#include "omp_j.h"
#include "seq_j.h"

int main() {
  int N = 2000;

  float *response = malloc(N * sizeof(float));
  float **coeficients = malloc(N * sizeof(float *));

  if (coeficients == NULL) {
    perror("Failed to allocate memory for rows");
    return 1;
  }

  for (int i = 0; i < N; i++) {
    coeficients[i] = malloc(N * sizeof(float));
    if (coeficients[i] == NULL) {
      perror("Failed to allocate memory for a column");
      for (int k = 0; k < i; k++) {
        free(coeficients[k]);
      }
      free(response);
      return 1;
    }
  }

  read_input("inputs/dados.txt", coeficients, response);

  seq_process(coeficients, response);
  omp_process(coeficients, response);

  free(coeficients);
  free(response);
}
