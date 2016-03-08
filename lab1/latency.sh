echo "=== Compilation..."
mpicc -o latency_send latency_send.c
mpicc -o latency_ssend latency_ssend.c
mpicc -o latency_bsend latency_bsend.c

echo "=== MPI_Send latency benchmark ==="
mpiexec -np 2 ./latency_send > latency_send.out
cat latency_send.out

echo "=== MPI_Ssend latency benchmark ==="
mpiexec -np 2 ./latency_ssend > latency_ssend.out
cat latency_ssend.out

echo "=== MPI_Bsend latency benchmark ==="
mpiexec -np 2 ./latency_bsend > latency_bsend.out
cat latency_bsend.out
