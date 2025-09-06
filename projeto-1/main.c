#include "omp_j.h"
#include "seq_j.h"
#include "utils.h"
#include <string.h>
int execute_test(char *dir, int num_threads, char *scheduler_method,int chunk_size){
  int N = 2000;
  char path[256] = "inputs/";
  strcat(path, dir);
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


  read_input(path, coeficients, response, N);

  measure_fn_seq_time(seq_process, coeficients, response, N);
  measure_fn_time(omp_process, coeficients, response, N, num_threads, scheduler_method, chunk_size);

  free(coeficients);
  free(response);

}
int main(int argc, char* argv[]) {

  if(argc<=3){
    puts("Error with arguments!");
    return -1;
  }
  if(argc==4){
    execute_test(argv[1],atoi(argv[2]), argv[3], 0);
  }
  if(argc==5){
    execute_test(argv[1],atoi(argv[2]), argv[3], atoi(argv[4]));
  }

}
