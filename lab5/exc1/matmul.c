#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_THREADS 16

double *rand_vec(int dim) {
  int i;
  double *vec;

  vec = (double *) malloc(sizeof(double) * dim);

  for (i = 0; i < dim; i++) {
    vec[i] = (double) rand() / (double) RAND_MAX;
  }

  return vec;
}

double **rand_mat(int dim1, int dim2) {
  int i;
  double **mat;

  mat = (double **) malloc(sizeof(double *) * dim1);

  for (i = 0; i < dim1; i++) {
    mat[i] = rand_vec(dim2);
  }

  return mat;
}

double *mat_mul_vec(double **mat, double *vec, int dim1, int dim2) {
  int i, j;
  double *result;

  result = (double *) malloc(sizeof(double) * dim1);

  #pragma omp parallel shared(mat, vec, result) private(i, j)
  {
    #pragma omp for schedule(static)
    for (i = 0; i < dim1; i++) {
      result[i] = 0;

      for (j = 0; j < dim2; j++) {
        result[i] += mat[i][j] * vec[i];
      }
    }
  }

  return result;
}

void print_vec(double *vec, int dim) {
  int i;
  for (i = 0; i < dim; i++) {
    printf("%f ", vec[i]);
  }
  printf("\n");
}

void print_mat(double **mat, int dim1, int dim2) {
  int i;
  for (i = 0; i < dim1; i++) {
    print_vec(mat[i], dim2);
  }
}

int main(int argc, char* argv[]) {
  int dim, nthreads;
  double **a, *b;
  double start_time;
  double total_time;

  if (argc != 2) {
    fprintf(stderr, "Usage: ./matmul dim\n");
    exit(1);
  }

  dim = atoi(argv[1]);

  a = rand_mat(dim, dim);
  b = rand_vec(dim);

  omp_set_dynamic(0);

  printf("dim,nthreads,time\n");
  for (nthreads = 1; nthreads <= MAX_THREADS; nthreads++) {
    start_time = omp_get_wtime();

    #pragma omp parallel num_threads(nthreads)
    {
      mat_mul_vec(a, b, dim, dim);
    }

    total_time = omp_get_wtime() - start_time;
    printf("%d,%d,%f\n", dim, nthreads, total_time);
  }

  return 0;
}
