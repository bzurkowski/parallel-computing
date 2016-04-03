#!/usr/bin/env python2.6
import os, subprocess, sys, tarfile, json, shlex

def get_json(file_name):
    with open(file_name, 'rb+') as input_file:
        h = json.JSONDecoder().decode(input_file.read())
    return h

def make_tarfile(file_name, output_filename):
    from contextlib import closing
    with closing(tarfile.open(output_filename, "w:gz")) as tar:
        tar.add(file_name)

current_dir = os.path.dirname(os.path.realpath(sys.argv[0]))

input_json = get_json('input.json')

num_points = input_json['num_points']
num_procs  = input_json['num_procs']

mpicc_cmd   = "mpicc -o monte_carlo %s/monte_carlo.c" % current_dir
mpiexec_cmd = "mpiexec -np %s ./monte_carlo %s" % (num_procs, num_points)

commands = [
    'module load mvapich2',
    'module load mpiexec',
    mpicc_cmd,
    mpiexec_cmd
]

command = ' && '.join(commands)

out = None
try:
    out = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).stdout.read()
except subprocess.CalledProcessError as e:
    out = e.output

with open('output.txt', 'wb+') as output_file:
    output_file.write(out)

with open('output.json', 'wb+') as output_file:
    output_file.write(json.JSONEncoder().encode({'status': 'ok', 'results': {'walltime': float(out)}}))

make_tarfile('output.txt', 'output.tar.gz')
