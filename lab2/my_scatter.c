// Program that computes the dot product of an array of elements in parallel using
// custom implementation of collective communication

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// generates a vector of random numbers from range 0 - 1
float *gen_rand_nums(int num_elements)
{
  float *rand_nums = (float *) malloc(sizeof(float) * num_elements);
  int i;

  for (i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float) RAND_MAX);
  }

  return rand_nums;
}

// computes the dot product of given vectors
float calculate_dot(float *a, float *b, int num_elements)
{
  float result = 0.f;
  int i;

  for (i = 0; i < num_elements; i++) {
    result += a[i] * b[i];
  }

  return result;
}

// sums elements of array
float sum(float *a, int num_elements)
{
  float result = 0.f;
  int i;

  for (i = 0; i < num_elements; i++) {
    result += a[i];
  }

  return result;
}

// sends data from one process to all other processes in a communicator
MY_Scatter(void *send_data, int send_count, MPI_Datatype send_datatype,
  void *recv_data, int recv_count, MPI_Datatype recv_datatype,
  int root, MPI_Comm communicator)
{
  int world_rank, world_size, proc_num;
  int chunk_size;
  void *chunk, *send_data_ptr;

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_rank == root)
  {
    MPI_Type_size(send_datatype, &chunk_size);
    chunk_size *= send_count;

    chunk = (char *) malloc(chunk_size);

    send_data_ptr = (char *) send_data;

    for (proc_num = 0; proc_num < world_size; proc_num++) {
      memcpy(chunk, send_data_ptr, chunk_size);

      if (proc_num == root) {
        memcpy(recv_data, chunk, chunk_size);
      } else {
        MPI_Send(chunk, send_count, send_datatype, proc_num, 0, communicator);
      }

      send_data_ptr += chunk_size;
    }
  }
  else
  {
    MPI_Recv(recv_data, send_count, send_datatype, root, 0, communicator, MPI_STATUS_IGNORE);
  }
}

// gathers together values from a group of processes
MY_Gather(void* send_data, int send_count, MPI_Datatype send_datatype,
  void* recv_data, int recv_count, MPI_Datatype recv_datatype,
  int root, MPI_Comm communicator)
{
  int world_rank, world_size, proc_num;
  int send_data_size;
  void *recv_data_ptr;

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_rank == root) {
    MPI_Type_size(send_datatype, &send_data_size);
    send_data_size *= send_count;

    recv_data_ptr = (char *) recv_data;

    for (proc_num = 0; proc_num < world_size; proc_num++) {
      if (proc_num == root) {
        memcpy(recv_data_ptr, send_data, send_data_size);
      } else {
        MPI_Recv(recv_data_ptr, recv_count, recv_datatype, proc_num, 0, communicator, MPI_STATUS_IGNORE);
      }

      recv_data_ptr += send_data_size;
    }
  } else {
    MPI_Send(send_data, send_count, send_datatype, root, 0, MPI_COMM_WORLD);
  }

  MPI_Barrier(communicator);
}

int main(int argc, char **argv)
{
  int world_rank, world_size;
  int num_elements_per_proc;

  float *a, *b, *sa, *sb;
  float total_dot, sdot;
  float *sdots;

  double start_time, total_time;

  if (argc != 2) {
    fprintf(stderr, "usage: my_scatter num_elements_per_proc\n");
    exit(1);
  }

  num_elements_per_proc = atoi(argv[1]);

  srand(time(NULL));

  // initialize MPI world
  MPI_Init(NULL, NULL);

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);


  // generate input vectors of random numbers on the root process
  if (world_rank == 0) {
    a = gen_rand_nums(world_size * num_elements_per_proc);
    b = gen_rand_nums(world_size * num_elements_per_proc);
  }

  // for each process, allocate buffers that will hold subsets of input vectors
  sa = (float *) malloc(sizeof(float) * num_elements_per_proc);
  sb = (float *) malloc(sizeof(float) * num_elements_per_proc);

  if (world_rank == 0) {
    printf("computing dot product of two %i-element vectors on %i nodes...\n",
      world_size * num_elements_per_proc, world_size);

    start_time = MPI_Wtime();
  }

  // scatter input vectors from the root process to all processes in the MPI world
  MY_Scatter(a, num_elements_per_proc, MPI_FLOAT,
    sa, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

  MY_Scatter(b, num_elements_per_proc, MPI_FLOAT,
    sb, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // compute the dot product of current subset
  sdot = calculate_dot(sa, sb, num_elements_per_proc);

  // allocate a buffer that will hold gathered results from all processes
  if (world_rank == 0) {
    sdots = (float *) malloc(sizeof(float) * world_size);
  }

  // gather all partial dot products to the root process
  MY_Gather(&sdot, 1, MPI_FLOAT, sdots, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // compute total dot product
  if (world_rank == 0) {
    total_time = MPI_Wtime() - start_time;

    total_dot = sum(sdots, world_size);

    printf("dot product: %f\n", total_dot);
    printf("total execution time: %f\n", total_time);
  }

  // clean up
  if (world_rank == 0) {
    free(a);
    free(b);
    free(sdots);
  }

  free(sa);
  free(sb);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
