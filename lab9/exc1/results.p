set term png size 1024,768
set output "results.png"

set autoscale
set grid

set title "CUDA - Matrix multiplication" font ",24 "

set xlabel "Matrix size"
set ylabel "Execution time [s]"

set format x "%.0f"

set datafile separator ","

set key left top box

plot "results.out" using 1:3 title "GPU computation" with lines, \
     "results.out" using 1:2 title "CPU computation" with lines
