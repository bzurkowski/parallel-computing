mpicc -o bandwidth_send bandwidth_send.c
mpicc -o bandwidth_ssend bandwidth_ssend.c
mpicc -o bandwidth_bsend bandwidth_bsend.c

mpiexec -machinefile mpihosts -np 2 ./bandwidth_send > bandwidth_send.out
mpiexec -machinefile mpihosts -np 2 ./bandwidth_ssend > bandwidth_ssend.out
mpiexec -machinefile mpihosts -np 2 ./bandwidth_bsend > bandwidth_bsend.out
