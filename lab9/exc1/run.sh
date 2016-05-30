#!/usr/bin/env bash

> results.out

for size in $(seq 16 16 512); do
  echo $size
  ./matmul $size >> results.out
done

gnuplot results.p
