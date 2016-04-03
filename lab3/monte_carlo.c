#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define RETRIES_COUNT 10

int main(int argc, char **argv)
{
  int world_rank, world_size;
  int num_points, points_per_proc;

  double x, y, z;
  double pi;
  int inner_counter = 0;
  int total_inner_counter = 0;
  int i, retry;

  double start_time, total_time;

  if (argc != 2) {
    fprintf(stderr, "Usage: ./monte_carlo num_points\n");
    exit(1);
  }

  num_points = atoi(argv[1]);

  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_rank == 0) {
    points_per_proc = num_points - (world_size - 1) * (num_points / world_size);
  } else {
    points_per_proc = num_points / world_size;
  }

  srand(world_rank);

  for (retry = 0; retry < RETRIES_COUNT; retry++) {
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    for (i = 0; i < points_per_proc; i++) {
      x = (double) rand() / RAND_MAX;
      y = (double) rand() / RAND_MAX;

      z = x * x + y * y;

      if (z <= 1) inner_counter++;
    }

    MPI_Reduce(&inner_counter, &total_inner_counter, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
      pi = 4.f * total_inner_counter / num_points;
      total_time += MPI_Wtime() - start_time;
    }
  }

  if (world_rank == 0) {
    printf("%f", total_time / RETRIES_COUNT);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
