#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define MIN 0
#define MAX 1

typedef struct
{
  double *values;
  int    count;
} Bucket;

int doubles_comparator(const void *a, const void *b)
{
    double _a = *(double *) a;
    double _b = *(double *) b;

    if (_a < _b)
      return -1;
    else if (_a == _b)
      return 0;
    else
      return 1;
}

double *rand_array(int size) {
  int i;
  double *array;

  array = (double *) malloc(sizeof(double) * size);

  for (i = 0; i < size; i++) {
    array[i] = (double) rand() / (double) RAND_MAX;
  }

  return array;
}

void print_array(double *array, int size) {
  int i;

  for (i = 0; i < size; i++) {
    printf("%f ", array[i]);
  }
  printf("\n");
}


int main(int argc, char* argv[]) {
  int size, num_threads;
  int i, j;

  double *array;

  int buckets_count, bucket_id;
  double bucket_range, bucket_step;
  double *bucket_boundaries;

  Bucket **buckets;
  Bucket *my_buckets, *results;

  if (argc != 3) {
    fprintf(stderr, "Usage: ./bsort size num_threads\n");
    exit(1);
  }

  srand((unsigned) time(NULL));

  size = atoi(argv[1]);
  num_threads = atoi(argv[2]);

  array = rand_array(size);

  #pragma omp parallel num_threads(num_threads) default(shared) private(i, j, my_buckets, bucket_id)
  {
    #pragma omp single
    {
      // number of buckets = number of available threads
      buckets_count = omp_get_num_threads();

      // calculate range of values
      bucket_range = MAX - MIN;

      // bucket step = bucket size
      bucket_step = bucket_range / buckets_count;

      // calculate bucket boundaries
      bucket_boundaries = (double *) malloc(sizeof(double) * (buckets_count - 1));

      // [0,     0.125)
      // [0.125, 0.250)
      // [0.250, 0.375)
      // ...
      // [0.750, 0.875)
      // [0.875, 1.0]

      for (i = 0; i < buckets_count - 1; i++) {
        bucket_boundaries[i] = MIN + (i + 1) * bucket_step;
      }

      // allocate array of bucket's arrays
      // each thread gets own array of buckets
      buckets = (Bucket **) malloc(sizeof(Bucket *) * buckets_count);
    }

    // allocate array of buckets for each thread
    #pragma omp for schedule(static)
    for (i = 0; i < buckets_count; i++) {
      buckets[i] = (Bucket *) malloc(sizeof(Bucket) * buckets_count);

      my_buckets = buckets[i];

      for (j = 0; j < buckets_count; j++)  {
        my_buckets[j].values = (double *) malloc(sizeof(double) * size);
        my_buckets[j].count = 0;
      }
    }

    // distribute elements into buckets
    #pragma omp for schedule(static)
    for (i = 0; i < size; i++) {
      bucket_id = 0;
      j = 0;

      // calculate bucket for current element
      while (j < buckets_count - 1 && array[i] >= bucket_boundaries[j++]) {
        bucket_id++;
      }

      // add element to bucket
      my_buckets[bucket_id].values[my_buckets[bucket_id].count] = array[i];
      my_buckets[bucket_id].count++;
    }

    #pragma omp single
    {
      results = (Bucket *) malloc(sizeof(Bucket) * buckets_count);
    }

    // merge buckets
    // 1-st thread merges all 1-st buckets
    // 2-nd thread merges all 2-nd buckets
    // ...
    #pragma omp for schedule(static)
    for (i = 0; i < buckets_count; i++) {
      results[i].values = (double *) malloc(sizeof(double) * size);
      results[i].count = 0;

      for (j = 0; j < buckets_count; j++) {
        memcpy(
          &results[i].values[results[i].count],
          buckets[j][i].values,
          buckets[j][i].count * sizeof(double)
        );

        results[i].count += buckets[j][i].count;
      }
      free(buckets[j]);

      // sort merged bucket
      qsort(results[i].values, results[i].count, sizeof(double), &doubles_comparator);
    }

    // collect sorted buckets
    #pragma omp single
    {
      free(buckets);

      j = 0;
      for (i = 0; i < buckets_count; i++) {
        memcpy(
          &array[j],
          results[i].values,
          results[i].count * sizeof(double)
        );

        j += results[i].count;
      }
      free(results);
    }
  }

  // print_array(array, size);

  return 0;
}
