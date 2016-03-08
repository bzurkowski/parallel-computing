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
