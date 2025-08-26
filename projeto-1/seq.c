#include "utils.h"

int main() {
  int N = 2000;

  float *coeficients = malloc(N * sizeof(float));
  float **response = malloc(N * sizeof(float *));
  if (response == NULL) {
    perror("Failed to allocate memory for rows");
    return 1;
  }

  for (int i = 0; i < N; i++) {
    response[i] = malloc(N * sizeof(float));
    if (response[i] == NULL) {
      perror("Failed to allocate memory for a column");
      for (int k = 0; k < i; k++) {
        free(response[k]);
      }
      free(response);
      return 1;
    }
  }

  read_input("inputs/dados.txt", response, coeficients);

  free(coeficients);
  free(response);
}
