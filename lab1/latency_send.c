#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define RETRIES_PER_MEASUREMENT 10

int main(int argc, char **argv)
{
  int rank, numprocs, i;

  char message;

  double start, end, total_time;
  double latency;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

  if (numprocs != 2)
  {
      if(rank == 0)
      {
          fprintf(stderr, "This test requires exactly two processes!\n");
      }

      MPI_Finalize();
      return EXIT_FAILURE;
  }

  message = 'a';

  for (i = 0; i < RETRIES_PER_MEASUREMENT; i++)
  {
    MPI_Barrier(MPI_COMM_WORLD);

    start = MPI_Wtime();

    if (rank == 0)
    {
      MPI_Send(&message, 1, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(&message, 1, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
      MPI_Recv(&message, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(&message, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    end = MPI_Wtime();

    total_time += (end - start) / 2;
  }

  latency = total_time * 1000 / RETRIES_PER_MEASUREMENT;

  if (rank == 0)
  {
    printf("latency: %fms\n", latency);
  }

  MPI_Finalize();

  return 0;
}
