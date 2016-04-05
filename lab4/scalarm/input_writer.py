#!/usr/bin/env python2.6
import sys, json

# extract input params from json
data = None
with open(sys.argv[1], 'rb') as input_json:
    data = json.load(input_json)

ncores   = data['ncores']
size     = data['size']
scalable = data['scalable']

# write params to input file
with open('input.txt', 'wb+') as input_file:
    input_file.write('{0} {1} {2}'.format(ncores, size, scalable))
