#!/usr/bin/env python2.6
import os, subprocess

current_dir = os.path.dirname(os.path.realpath(__file__))

# load input params
with open('input.txt', 'r') as input_file:
    input_data = input_file.read().split()
    ncores   = int(input_data[0])
    npoints  = int(input_data[1])
    scalable = True if input_data[2] in ['True', 'true', '1'] else False

# scale problem if for purpose of scalable metrics
if scalable and ncores > 1:
    npoints *= ncores

# prepare exec commands
mpicc_command   = "mpicc -o {dir}/monte_carlo {dir}/monte_carlo.c".format(dir=current_dir)
mpiexec_command = "mpiexec -np {ncores} {dir}/monte_carlo {npoints}".format(ncores=ncores, dir=current_dir, npoints=npoints)

commands = [
    'module load mvapich2',
    'module load mpiexec',
    mpicc_command,
    mpiexec_command
]

command = ' && '.join(commands)

# run exec commands
output = None
try:
    output = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).stdout.read()
except subprocess.CalledProcessError as e:
    output = e.output

# write output to file
with open('output.txt', 'wb+') as output_file:
    output_file.write('{0}'.format(output))
