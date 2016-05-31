#include "matmul.h"

int main(int argc, char * const argv[])
{
  int size;
  Matrix A, B, C_gpu, C_gpu_blas, C_cpu;
  float gpu_time, gpu_blas_time, cpu_time;

  if (argc != 2) {
    printf("usage: matmul SIZE\n");
    exit(1);
  }

  size = atoi(argv[1]);

  A.width = size;
  A.height = size;
  A.elements = new float[size * size];

  B.width = size;
  B.height = size;
  B.elements = new float[size * size];

  C_gpu.width = size;
  C_gpu.height = size;
  C_gpu.elements = new float[size * size];

  C_gpu_blas.width = size;
  C_gpu_blas.height = size;
  C_gpu_blas.elements = new float[size * size];

  C_cpu.width = size;
  C_cpu.height = size;
  C_cpu.elements = new float[size * size];

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      A.elements[i * size + j] = rand() % 100;
      B.elements[i * size + j] = rand() % 100;
    }
  }

  gpu_time = matmul_gpu(A, B, C_gpu);
  gpu_blas_time = matmul_gpu_cublas(A, B, C_gpu_blas);
  cpu_time = matmul_cpu(A, B, C_cpu);

  // printf("GPU time: %f\n", gpu_time);
  // printf("GPU CUBLAS time: %f\n", gpu_blas_time);
  // printf("CPU time: %f\n", cpu_time);
  //
  // printf("Check GPU: %d\n", check(C_gpu, C_cpu));
  // printf("Check GPU CUBLAS: %d\n", check(C_gpu_blas, C_cpu));

  printf("%d,%f,%f,%f\n", size, gpu_time, gpu_blas_time, cpu_time);
}

// matrix GPU multiplication
float matmul_gpu(const Matrix A, const Matrix B, Matrix C)
{
  Matrix d_A, d_B, d_C;
  size_t size;
  float time;

  d_A = cudaMatrixCopy(A);
  d_B = cudaMatrixCopy(B);
  d_C = cudaMatrixCopy(C);

  // set block size
  dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);

  // set grid size
  dim3 dimGrid(
    (C.width + dimBlock.x - 1) / dimBlock.x,
    (C.height + dimBlock.y - 1) / dimBlock.y
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
  time = (float) sdkGetTimerValue(&timer) / 1000;
  sdkDeleteTimer(&timer);

  // copy results from device to host
  size = C.width * C.height * sizeof(float);
  cudaMemcpy(C.elements, d_C.elements, size, cudaMemcpyDeviceToHost);

  // clean up device memory
  cudaFree(d_A.elements);
  cudaFree(d_B.elements);
  cudaFree(d_C.elements);

  return time;
}

// matrix GPU multiplication kernel
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

// matrix GPU CUBLAS multiplication
float matmul_gpu_cublas(const Matrix A, const Matrix B, Matrix C) {
  Matrix d_A, d_B, d_C;
  size_t size;
  float time;

  cudaEvent_t start, stop;
  cublasHandle_t handle;

  const float alpha = 1.0;
  const float beta = 0.0;

  // load input matrices into device memory
  d_A = cudaMatrixCopy(A);
  d_B = cudaMatrixCopy(B);
  d_C = cudaMatrixCopy(C);

  // set block size
  dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);

  // set grid size
  dim3 dimGrid(
    (C.width + dimBlock.x - 1) / dimBlock.x,
    (C.height + dimBlock.y - 1) / dimBlock.y
  );

  // create CUBLAS handle
  cublasCreate(&handle);

  cudaEventCreate(&start);
  cudaEventCreate(&stop);

  // record start event
  cudaEventRecord(start, NULL);

  // perform matrix multiplication
  cublasSgemm(
    handle,
    CUBLAS_OP_N,
    CUBLAS_OP_N,
    d_B.width,
    d_A.height,
    d_A.width,
    &alpha,
    d_B.elements,
    d_B.width,
    d_A.elements,
    d_A.width,
    &beta,
    d_C.elements,
    d_A.width
  );

  // record stop event
  cudaEventRecord(stop, NULL);
  cudaEventSynchronize(stop);

  // calculate time between events
  cudaEventElapsedTime(&time, start, stop);
  time /= 1000;

  // destroy handle
  cublasDestroy(handle);

  // copy results from device to host
  size = C.width * C.height * sizeof(float);
  cudaMemcpy(C.elements, d_C.elements, size, cudaMemcpyDeviceToHost);

  // clean up device memory
  cudaFree(d_A.elements);
  cudaFree(d_B.elements);
  cudaFree(d_C.elements);

  return time;
}

// matrix CPU multiplication
float matmul_cpu(const Matrix A, const Matrix B, Matrix C) {
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
  return (float) (end_time - start_time) / CLOCKS_PER_SEC;
}

// verifies if product matrices calculated by cpu and GPU are equal
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

Matrix cudaMatrixCopy(const Matrix A) {
  Matrix d_A;
  size_t size = A.width * A.height * sizeof(float);

  d_A.width = A.width;
  d_A.height = A.height;

  cudaMalloc((void**) &d_A.elements, size);
  cudaMemcpy(d_A.elements, A.elements, size, cudaMemcpyHostToDevice);

  return d_A;
}
