#include "matmul.h"

int main(int argc, char * const argv[])
{
  int size;

  Matrix A;
  Matrix B;
  Matrix C_gpu;
  Matrix C_cpu;

  double gpu_time, cpu_time;

  if (argc != 2) {
    printf("usage: matmul SIZE\n");
    exit(1);
  }

  size = atoi(argv[1]);

  A.width = size;
  B.width = size;

  C_gpu.width = size;
  C_cpu.width = size;

  A.height = size;
  B.height = size;

  C_gpu.height = size;
  C_cpu.height = size;

  A.elements = new float[size*size];
  B.elements = new float[size*size];

  C_gpu.elements = new float[size*size];
  C_cpu.elements = new float[size*size];

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      A.elements[i * size + j] = rand() % 100;
      B.elements[i * size + j] = rand() % 100;
    }
  }

  gpu_time = matmul_gpu(A, B, C_gpu);
  cpu_time = matmul_cpu(A, B, C_cpu);

  // printf("GPU time: %f\n", gpu_time);
  // printf("CPU time: %f\n", cpu_time);
  // printf("Check: %d\n", check(C_gpu, C_cpu));
  printf("%d,%f,%f\n", size, cpu_time, gpu_time);
}

// matrix gpu multiplication
double matmul_gpu(const Matrix A, const Matrix B, Matrix C)
{
  Matrix d_A, d_B, d_C;
  size_t size;
  double time;

  d_A.width = A.width;
  d_B.width = B.width;
  d_C.width = C.width;

  d_A.height = A.height;
  d_B.height = B.height;
  d_C.height = C.height;

  // load input matrices into device memory
  size = A.width * A.height * sizeof(float);
  cudaMalloc((void**) &d_A.elements, size);
  cudaMemcpy(d_A.elements, A.elements, size, cudaMemcpyHostToDevice);

  size = B.width * B.height * sizeof(float);
  cudaMalloc((void**) &d_B.elements, size);
  cudaMemcpy(d_B.elements, B.elements, size, cudaMemcpyHostToDevice);

  // allocate device memory for matrix product
  size = C.width * C.height * sizeof(float);
  cudaMalloc((void**) &d_C.elements, size);

  // set block size
  dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);

  // set grid size
  dim3 dimGrid(
    (B.width + dimBlock.x - 1) / dimBlock.x,
    (B.height + dimBlock.y - 1) / dimBlock.y
  );

  // start timer
  StopWatchInterface *timer = NULL;
  sdkCreateTimer(&timer);
  sdkResetTimer(&timer);
  sdkStartTimer(&timer);

  // invoke kernel
  matmul_gpu_kernel<<<dimGrid, dimBlock>>>(d_A, d_B, d_C);

  // synchronize threads
  cudaThreadSynchronize();

  // stop timer, calculate total time
  sdkStopTimer(&timer);
  time = (double) sdkGetTimerValue(&timer) / 1000;
  sdkDeleteTimer(&timer);

  // copy results from device to host
  cudaMemcpy(C.elements, d_C.elements, size, cudaMemcpyDeviceToHost);

  // clean up device memory
  cudaFree(d_A.elements);
  cudaFree(d_B.elements);
  cudaFree(d_C.elements);

  return time;
}

// matrix gpu multiplication kernel
__global__ void matmul_gpu_kernel(Matrix A, Matrix B, Matrix C)
{
  // each thread computes one element of product C
  // results are being accumulated in C_value

  float C_value = 0;
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  // terminate thread if out of matrix bounds
  if (row >= A.height || col >= B.width) return;

  for (int i = 0; i < A.width; i++) {
    C_value += A.elements[row * A.width + i] * B.elements[i * B.width + col];
  }

  C.elements[row * C.width + col] = C_value;
}

// matrix cpu multiplication
double matmul_cpu(const Matrix A, const Matrix B, Matrix C) {
  int i, j, k;
  float sum;
  clock_t start_time, end_time;

  start_time = clock();

  for (i = 0; i < A.height; i++) {
    for (j = 0; j < B.width; j++) {
      sum = 0;
      for (k = 0; k < A.width; k++) {
        sum += A.elements[i * A.width + k] * B.elements[k * B.width + j];
      }

      C.elements[i * B.width + j] = sum;
    }
  }

  end_time = clock();
  return (double) (end_time - start_time) / CLOCKS_PER_SEC;
}

// verifies if product matrices calculated by cpu and gpu are equal
int check(const Matrix A, const Matrix B) {
  if (A.width != B.width || A.height != B.height) return 0;
  for (int i = 0; i < A.width; i++) {
    for (int j = 0; j < A.height; j++) {
      if (A.elements[j * A.width + i] != B.elements[j * A.width + i])
        return 0;
    }
  }
  return 1;
}
