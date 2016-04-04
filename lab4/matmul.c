#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

# define RETRIES_COUNT 10

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

int main(int argc, char **argv)
{
  int world_rank, world_size;
  int ncols_per_proc, ncols_overhead;
  int arows, acols, bcols, asize;
  int i, j, k, retry;
  double **a, **b, **c, **bcol, **ccol;
  void *bpt, *cpt;
  double sum, start_time, total_time;

  MPI_Datatype send_bcol_type0, send_bcol_type;
  MPI_Datatype recv_bcol_type0, recv_bcol_type;

  MPI_Datatype send_ccol_type0, send_ccol_type;
  MPI_Datatype recv_ccol_type0, recv_ccol_type;

  if (argc != 4) {
    fprintf(stderr, "Usage: ./matmul arows, acols, bcols\n");
    exit(1);
  }

  arows = atoi(argv[1]);
  acols = atoi(argv[2]);
  bcols = atoi(argv[3]);

  asize = arows * acols;

  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  ncols_overhead = bcols & world_size;
  ncols_per_proc = bcols / world_size;

  MPI_Type_vector(acols,
    1,
    bcols,
    MPI_DOUBLE,
    &send_bcol_type0
  );

  MPI_Type_create_resized(
    send_bcol_type0,
    0,
    sizeof(double),
    &send_bcol_type
  );

  MPI_Type_vector(
    acols,
    1,
    ncols_per_proc,
    MPI_DOUBLE,
    &recv_bcol_type0
  );

  MPI_Type_create_resized(
    recv_bcol_type0,
    0,
    sizeof(double),
    &recv_bcol_type
  );

  MPI_Type_vector(
    arows,
    1,
    ncols_per_proc,
    MPI_DOUBLE,
    &send_ccol_type0
  );

  MPI_Type_create_resized(
    send_ccol_type0,
    0,
    sizeof(double),
    &send_ccol_type
  );

  MPI_Type_vector(
    arows,
    1,
    bcols,
    MPI_DOUBLE,
    &recv_ccol_type0
  );

  MPI_Type_create_resized(
    recv_ccol_type0,
    0,
    sizeof(double),
    &recv_ccol_type
  );

  MPI_Type_commit(&send_bcol_type);
  MPI_Type_commit(&recv_bcol_type);
  MPI_Type_commit(&send_ccol_type);
  MPI_Type_commit(&recv_ccol_type);

  if (world_rank == 0) {
    a = rand_mat(arows, acols);
    b = rand_mat(acols, bcols);
    bpt = *b;

    c = alloc_mat(arows, bcols);
    cpt = *c;
  } else {
    a = alloc_mat(arows, acols);
    bpt = NULL;
    cpt = NULL;
  }

  bcol = alloc_mat(acols, ncols_per_proc);
  ccol = alloc_mat(arows, ncols_per_proc);

  for (retry = 0; retry < RETRIES_COUNT; retry++) {
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();


    MPI_Bcast(*a, asize, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatter(
      bpt,
      ncols_per_proc,
      send_bcol_type,
      (*bcol),
      ncols_per_proc,
      recv_bcol_type,
      0,
      MPI_COMM_WORLD
    );

    for (i = 0; i < arows; i++) {
      for (j = 0; j < ncols_per_proc; j++) {
        sum = 0;

        for (k = 0; k < acols; k++) {
          sum += a[i][k] * bcol[k][j];
        }

        ccol[i][j] = sum;
      }
    }

    //         0 1 2
    //         3 4 5
    //         6 7 8
    //  0 1 2  o x x
    //  3 4 5  o x x
    //  6 7 8  o x x

    MPI_Gather(
      (*ccol),
      ncols_per_proc,
      send_ccol_type,
      cpt,
      ncols_per_proc,
      recv_ccol_type,
      0,
      MPI_COMM_WORLD
    );

    if (world_rank == 0) {
      for (i = bcols - ncols_overhead; i < bcols; i++) {
        for (j = 0; j < arows; j++) {
          sum = 0;

          for (k = 0; k < acols; k++) {
            sum += a[j][k] * b[k][i];
          }

          c[j][i] = sum;
        }
      }
    }

    total_time += MPI_Wtime() - start_time;
  }

  if (world_rank == 0) {
    printf("%f", total_time / RETRIES_COUNT);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
