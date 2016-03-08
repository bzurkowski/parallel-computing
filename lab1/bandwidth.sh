echo "=== Compilation..."
mpicc -o bandwidth_send bandwidth_send.c
mpicc -o bandwidth_ssend bandwidth_ssend.c
mpicc -o bandwidth_bsend bandwidth_bsend.c

echo "=== MPI_Send bandwidth benchmark ==="
mpiexec -np 2 ./bandwidth_send > bandwidth_send.out
cat bandwidth_send.out

echo "=== MPI_Ssend bandwidth benchmark ==="
mpiexec -np 2 ./bandwidth_ssend > bandwidth_ssend.out
cat bandwidth_ssend.out

echo "=== MPI_Bsend bandwidth benchmark ==="
mpiexec -np 2 ./bandwidth_bsend > bandwidth_bsend.out
cat bandwidth_bsend.out

echo "=== Plotting..."

gnuplot

set term png size 1024,768
set output "bandwidth.png"

set autoscale
set grid

set title "MPI P2P Communication Modes - Benchmark" font ",24 "

set xlabel "Message size [B]"
set ylabel "Bandwidth [Mbit/s]"

set format x "%.0f"

set datafile separator "\t"

set key right bottom box

plot "bandwidth_send.out"  using 1:3 every ::1 title "standard send"    with lines, \
     "bandwidth_ssend.out" using 1:3 every ::1 title "synchronous send" with lines, \
     "bandwidth_bsend.out" using 1:3 every ::1 title "buffered send"    with lines

set output "bandwidth_log.png"

set logscale x 2

plot "bandwidth_send.out"  using 1:3 every ::1 title "standard send"    with lines, \
     "bandwidth_ssend.out" using 1:3 every ::1 title "synchronous send" with lines, \
     "bandwidth_bsend.out" using 1:3 every ::1 title "buffered send"    with lines

q

echo "=== Done."
