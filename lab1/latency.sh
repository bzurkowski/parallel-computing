mpicc -o latency_send latency_send.c
mpicc -o latency_ssend latency_ssend.c
mpicc -o latency_bsend latency_bsend.c

mpiexec -machinefile mpihosts -np 2 ./latency_send > latency_send.out
mpiexec -machinefile mpihosts -np 2 ./latency_ssend > latency_ssend.out
mpiexec -machinefile mpihosts -np 2 ./latency_bsend > latency_bsend.out
