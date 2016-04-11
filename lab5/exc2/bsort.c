#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

double *rand_array(int size) {
  int i;
  double *array;

  array = (double *) malloc(sizeof(double) * size);

  for (i = 0; i < size; i++) {
    array[i] = (double) rand() / (double) RAND_MAX;
  }

  return array;
}

int is_asc_sorted(double *array, int size) {
  int i;
  double prev;

  if (!size) return 1;

  prev = array[0];
  for (i = 1; i < size; i++) {
    if (array[i] < prev) return 0;
    prev = array[i];
  }

  return 1;
}

void print_array(double *array, int size) {
  int i;

  for (i = 0; i < size; i++) {
    printf("%f ", array[i]);
  }
  printf("\n");
}


int main(int argc, char* argv[]) {
  int size;
  double *array;

  if (argc != 2) {
    fprintf(stderr, "Usage: ./bsort size\n");
    exit(1);
  }

  srand((unsigned) time(NULL));

  size = atoi(argv[1]);

  array = rand_array(size);

  print_array(array, size);

  if (is_asc_sorted(array, size)) {
    printf("asc sorted\n");
  } else {
    printf("asc unsorted\n");
  }

  return 0;
}
