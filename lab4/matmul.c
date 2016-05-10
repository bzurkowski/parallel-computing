#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

# define RETRIES_COUNT 1


double **alloc_mat(int nrows, int ncols);
double **rand_mat(int nrows, int ncols);
void print_mat(double **matrix, int nrows, int ncols);


int main(int argc, char **argv)
{
  int world_rank, world_size;
  int ncols_per_proc;
  int size;
  int i, j, k, retry;
  double **a, **b, **c;
  double sum;
  double start_time, total_time;

  MPI_Datatype column_type0, column_type;

  if (argc != 2) {
    fprintf(stderr, "Usage: ./matmul size\n");
    exit(1);
  }

  size = atoi(argv[1]);

  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  ncols_per_proc = size / world_size;

  c = alloc_mat(size, size);

  if (world_rank == 0) {
      a = rand_mat(size, size);
      b = rand_mat(size, size);
  } else {
      a = alloc_mat(size, size);
      b = alloc_mat(size, size);
  }

  MPI_Type_vector(size,
    1,
    size,
    MPI_DOUBLE,
    &column_type0
  );

  MPI_Type_create_resized(
    column_type0,
    0,
    sizeof(double),
    &column_type
  );

  MPI_Type_commit(&column_type);

  MPI_Barrier(MPI_COMM_WORLD);
  start_time = MPI_Wtime();

  for (retry = 0; retry < RETRIES_COUNT; retry++) {
    MPI_Bcast(*a, size * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatter(
      (*b),
      ncols_per_proc,
      column_type,
      (*b),
      ncols_per_proc,
      column_type,
      0,
      MPI_COMM_WORLD
    );

    for (i = 0; i < size; i++) {
      for (j = 0; j < ncols_per_proc; j++) {
        sum = 0;

        for (k = 0; k < size; k++) {
          sum += a[i][k] * b[k][j];
        }

        c[i][j] = sum;
      }
    }

    MPI_Gather(
      (*c),
      ncols_per_proc,
      column_type,
      (*c),
      ncols_per_proc,
      column_type,
      0,
      MPI_COMM_WORLD
    );
  }

  total_time = MPI_Wtime() - start_time;

  if (world_rank == 0) {
    printf("%f", total_time / RETRIES_COUNT);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}

double **alloc_mat(int nrows, int ncols) {
  int mat_size, i;
  double *mat, **mat_pts;

  mat_size = nrows * ncols;

  mat = (double *) malloc(sizeof(double) * mat_size);
  mat_pts = (double **) malloc(sizeof(double *) * nrows);

  for (i = 0; i < nrows; i++) {
    mat_pts[i] = mat + i * ncols;
  }

  return mat_pts;
}

double **rand_mat(int nrows, int ncols) {
  int i, j, val;
  double **mat;

  mat = alloc_mat(nrows, ncols);

  val = 0;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      mat[i][j] = val++;
    }
  }

  return mat;
}

void print_mat(double **matrix, int nrows, int ncols) {
  int i, j;

  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      printf("%f ", matrix[i][j]);
    }
    printf("\n");
  }
}
