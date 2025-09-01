#include "utils.h"
#include "time.h"

void print_arr(float **vec, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      printf("%f | ", vec[i][j]);
}

void read_input(const char *path, float **A, float *b, const int n) {
  FILE *fp = fopen(path, "r");
  bool ex = false;
  int i = 0, j = 0;

  if (fp == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (fscanf(fp, "%f", &A[i][j]) != 1) {
        fprintf(stderr, "Error reading matrix data at A[%d][%d]\n", i, j);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }

  for (int i = 0; i < n; i++) {
    if (fscanf(fp, "%f", &b[i]) != 1) {
      fprintf(stderr, "Error reading vector data at b[%d]\n", i);
      fclose(fp);
      exit(EXIT_FAILURE);
    }
  }
}

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

bool write_file(const char *path, char *buffer) { return true; }

void measure_fn_time(void (fn)(float **, float *, int), float **A, float *b, int N) {
  struct timespec start, end, _time;
  clock_gettime(CLOCK_MONOTONIC, &start);
  fn(A, b, N);
  clock_gettime(CLOCK_MONOTONIC, &end);
  sub_timespec(start, end, &_time);
  printf("Time elapsed: %d.%.9ld\n", (int)_time.tv_sec, _time.tv_nsec);
}
