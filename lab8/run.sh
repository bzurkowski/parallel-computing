#!/usr/bin/env bash

SIZE=50000000
MAX_THREADS=1024
WARP_SIZE=32
START_THREADS_NUM=768
OUTPUT="$SIZE.out"

echo "===> Benchmarking..."
> $OUTPUT

for num_threads in $(seq $START_THREADS_NUM $WARP_SIZE $MAX_THREADS); do
  echo $num_threads
  ./add_vectors $SIZE $num_threads >> $OUTPUT
done

echo "===> Done."
echo "===> Plotting..."

gnuplot

set term png size 1024,768
set output "1000000.png"

set autoscale
set grid

set title "CUDA - Vectors addition" font ",24 "

set xlabel "Number of threads"

set ylabel "Execution time [s]" tc lt 1
set ytics autofreq tc lt 1

set y2label "Number of blocks" tc lt 2
set y2tics autofreq tc lt 2

set format x "%.0f"

set datafile separator ","

set key right bottom box

plot "1000000.out" using 2:4 every ::1 title "GPU: 1000000" with lines, \
     "1000000.out" using 2:1 every ::1 title "GPU: 1000000" with lines axes x1y2

q

echo "===> Done."
