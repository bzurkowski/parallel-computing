#include <stdio.h>
#include <time.h>
#include "helper_timer.h"

#define BLOCK_SIZE 16

typedef struct
{
  int width;
  int height;
  float *elements;
} Matrix;

__global__ void matmul_gpu_kernel(const Matrix, const Matrix, Matrix);

double matmul_gpu(const Matrix, const Matrix, Matrix);

double matmul_cpu(const Matrix, const Matrix, Matrix);

int check(const Matrix A, const Matrix B);
