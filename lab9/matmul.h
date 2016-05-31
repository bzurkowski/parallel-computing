#include <stdio.h>
#include <time.h>
#include <cublas_v2.h>
#include <helper_functions.h>

#define BLOCK_SIZE 32

typedef struct
{
  int width;
  int height;
  float *elements;
} Matrix;

// matrix GPU multiplication
float matmul_gpu(const Matrix, const Matrix, Matrix);

// matrix GPU multiplication kernel
__global__ void matmul_gpu_kernel(const Matrix, const Matrix, Matrix);

// matrix GPU CUBLAS multiplication
float matmul_gpu_cublas(const Matrix, const Matrix, Matrix);

// matrix CPU multiplication
float matmul_cpu(const Matrix, const Matrix, Matrix);

// verifies if product matrices calculated by cpu and GPU are equal
int check(const Matrix A, const Matrix B);

Matrix cudaMatrixCopy(const Matrix);
