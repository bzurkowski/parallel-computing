#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MESSAGE_SIZE (1<<26)
#define BYTES_IN_MEGABIT (1<<17)

#define RETRIES_PER_MEASUREMENT 10

int main(int argc, char **argv)
{
  int rank, numprocs, message_size, buffer_size, i;

  char *message, *buffer;

  double start, end, total_time, average_time;
  double message_size_mbits, bandwidth;

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

  if (rank == 0)
  {
    printf("%-20s\t%-20s\t%-20s\n", "MESSAGE SIZE IN B", "RUNTIME IN s", "BANDWIDTH IN Mb/s");
  }

  for (message_size = 1; message_size < MAX_MESSAGE_SIZE; message_size *= 2)
  {
    MPI_Pack_size(message_size, MPI_BYTE, MPI_COMM_WORLD, &buffer_size);
    buffer_size += MPI_BSEND_OVERHEAD;

    message = (char *) malloc(message_size * sizeof(char));
    buffer  = (char *) malloc(buffer_size);

    MPI_Buffer_attach(buffer, buffer_size);

    total_time = 0;

    for (i = 0; i < RETRIES_PER_MEASUREMENT; i++)
    {
      MPI_Barrier(MPI_COMM_WORLD);

      if (rank == 0)
      {
        start = MPI_Wtime();

        MPI_Bsend(message, message_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(message, message_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        end = MPI_Wtime();

        total_time += (end - start) / 2;
      }
      else
      {
        MPI_Recv(message, message_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Bsend(message, message_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
      }
    }

    if (rank == 0)
    {
      average_time = total_time / RETRIES_PER_MEASUREMENT;

      message_size_mbits = message_size * 1.0 / BYTES_IN_MEGABIT;

      bandwidth = message_size_mbits / average_time;

      printf("%-20i\t%-20f\t%-20f\n", message_size, average_time, bandwidth);
    }

    MPI_Buffer_detach(buffer, &buffer_size);

    free(message);
    free(buffer);
  }

  MPI_Finalize();

  return 0;
}
