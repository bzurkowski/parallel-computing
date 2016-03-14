echo "=== Compilation..."
mpicc -o scatter scatter.c
mpicc -o my_scatter my_scatter.c

echo "=== MPI_Scatter and MPI_Gather"
echo "=== Computing dot product of 400-element vectors accross 4 nodes ==="
mpiexec -np 4 ./scatter 100 > scatter.out
cat scatter.out

echo "=== MY_Scatter and MY_Gather"
echo "=== Computing dot product of 400-element vectors accross 4 nodes ==="
mpiexec -np 4 ./my_scatter 100 > my_scatter.out
cat my_scatter.out
