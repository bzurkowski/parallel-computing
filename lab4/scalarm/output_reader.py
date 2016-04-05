#!/usr/bin/env python2.6
import json

# load results
with open('output.txt', 'r') as output_file:
    time = float(output_file.read())

# prepare results dict
results = {
    'status': 'ok',
    'results': {
        'time': time
    }
}

# write results to json output file
with open('output.json', 'wb+') as output_json:
    output_json.write(json.dumps(results))
