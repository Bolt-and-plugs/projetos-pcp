#include "omp_j.h"
#include "seq_j.h"
#include "utils.h"

int main() {
  int N = 3;

  long double *response = malloc(N * sizeof(long double));
  long double **coeficients = malloc(N * sizeof(long double *));

  if (coeficients == NULL) {
    perror("Failed to allocate memory for rows");
    return 1;
  }

  for (int i = 0; i < N; i++) {
    coeficients[i] = malloc(N * sizeof(long double));
    if (!coeficients[i]) {
      perror("Failed to allocate memory for a column");
      free(coeficients);
      free(response);
      return 1;
    }
  }


  read_input("inputs/linear3.dat", coeficients, response, N);

  measure_fn_time(seq_process, coeficients, response, N);
  measure_fn_time(omp_process, coeficients, response, N);

  free(coeficients);
  free(response);
}
