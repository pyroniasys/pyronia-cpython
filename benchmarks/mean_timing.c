/* Measure impact of time measurement in performance benchmarks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ITERS 10000

int main(int argc, char **argv) {

  int i = 0;
  double total = 0;
  struct timespec start, stop, dummy;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &dummy);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
    total += ((stop.tv_sec - start.tv_sec) * 1e6 + (stop.tv_nsec - start.tv_nsec) / 1e3);
  }

  printf("Mean duration of gettime: %.2f us\n", (total/ITERS));
}
