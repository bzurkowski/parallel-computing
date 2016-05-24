#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cuda.h>
#include "helper_functions.h"


__global__ void add_vectors_gpu(int *a, int *b, int *c, unsigned long long int size);

void add_vectors_cpu(int *a, int *b, int *c, unsigned long long int size);

int *alloc_vector(unsigned long long int size);

int * random_vector(unsigned long long int size);

int check(int *c1, int *c2, unsigned long long int size);

void print_vector(int *v, unsigned long long int size);


int main(int argc, char **argv) {
  unsigned long long int size;
  int *a, *b, *c_cpu, *c_gpu;
  int *da, *db, *dc;
  int num_blocks, num_threads;
  double cpu_time, gpu_time;
  clock_t cpu_begin, cpu_end;

  if (argc != 3) {
    printf("usage: add_vectors SIZE NUM_THREADS_PER_BLOCK");
    exit(1);
  }

  srand(time(NULL));

  size = atoi(argv[1]);
  num_threads = atoi(argv[2]);
  num_blocks = (size + num_threads - 1) / num_threads;

  // generate random input vectors
  a = random_vector(size);
  b = random_vector(size);

  // allocate memory for results
  c_cpu = alloc_vector(size);
  c_gpu = alloc_vector(size);

  // add vectors on host
  cpu_begin = clock();
  add_vectors_cpu(a, b, c_cpu, size);
  cpu_end = clock();

  cpu_time = (double) (cpu_end - cpu_begin) / CLOCKS_PER_SEC;

  // alloc device memory
  cudaMalloc((void**) &da, size * sizeof(int));
  cudaMalloc((void**) &db, size * sizeof(int));
  cudaMalloc((void**) &dc, size * sizeof(int));

  // copy input vectors to device
  cudaMemcpy(da, a, size * sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(db, b, size * sizeof(int), cudaMemcpyHostToDevice);

  // start device timer
  StopWatchInterface *timer = NULL;
  sdkCreateTimer(&timer);
  sdkResetTimer(&timer);
  sdkStartTimer(&timer);

  add_vectors_gpu<<<num_blocks, num_threads>>>(da, db, dc, size);

  // stop device timer
  cudaThreadSynchronize();
  sdkStopTimer(&timer);
  gpu_time = (double) sdkGetTimerValue(&timer) / 1000;
  sdkDeleteTimer(&timer);

  // copy results from device to host
  cudaMemcpy(c_gpu, dc, size * sizeof(int), cudaMemcpyDeviceToHost);

  // clean up device memory
  cudaFree(da);
  cudaFree(db);
  cudaFree(dc);

  // print results
  // printf("CPU time: %f\n", cpu_time);
  // printf("GPU time: %f\n", gpu_time);
  // printf("Check: %d\n", check(c_cpu, c_gpu, size));

  printf("%d, %d, %f,%f,%d\n", num_blocks, num_threads, cpu_time, gpu_time, check(c_cpu, c_gpu, size));

  // clean up host memory
  free(a);
  free(b);
  free(c_cpu);
  free(c_gpu);

  return 0;
}

__global__ void add_vectors_gpu(int *a, int *b, int *c, unsigned long long int size) {
  unsigned long long int index = blockIdx.x * blockDim.x + threadIdx.x;

  if(index < size) {
    c[index] = a[index] + b[index];
  }
}

void add_vectors_cpu(int *a, int *b, int *c, unsigned long long int size) {
  unsigned long long int i;
  for (i = 0; i < size; i++) {
    c[i] = a[i] + b[i];
  }
}

int *alloc_vector(unsigned long long int size) {
  return (int *) malloc(sizeof(int) * size);
}

int * random_vector(unsigned long long int size) {
  unsigned long long int i;
  int *vector;
  vector = alloc_vector(size);

  for (i = 0; i < size; i++) {
    vector[i] = rand() % 100;
  }

  return vector;
}

int check(int *c1, int *c2, unsigned long long int size) {
  unsigned long long int i;
  for (i = 0; i < size; i++) {
    if (c1[i] != c2[i]) return 0;
  }
  return 1;
}

void print_vector(int *v, unsigned long long int size) {
  unsigned long long int i;
  for (i = 0; i < size; i++) {
    printf("%d ", v[i]);
  }
  printf("\n");
}
