#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define RETRIES_PER_MEASUREMENT 10

int main(int argc, char **argv)
{
  int rank, numprocs, buffer_size, i;

  char message, *buffer;

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

  MPI_Pack_size(1, MPI_BYTE, MPI_COMM_WORLD, &buffer_size);
  buffer_size += MPI_BSEND_OVERHEAD;

  buffer = (char *) malloc(buffer_size);

  MPI_Buffer_attach(buffer, buffer_size);

  message = 'a';

  for (i = 0; i < RETRIES_PER_MEASUREMENT; i++)
  {
    MPI_Barrier(MPI_COMM_WORLD);

    start = MPI_Wtime();

    if (rank == 0)
    {
      MPI_Bsend(&message, 1, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(&message, 1, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
      MPI_Recv(&message, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Bsend(&message, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    end = MPI_Wtime();

    total_time += (end - start) / 2;
  }

  latency = total_time * 1000 / RETRIES_PER_MEASUREMENT;

  if (rank == 0)
  {
    printf("latency: %fms\n", latency);
  }

  MPI_Buffer_detach(buffer, &buffer_size);
  MPI_Finalize();

  free(buffer);

  return 0;
}
