import csv
from collections import defaultdict
from operator import itemgetter

def normalize_entry(entry):
    normalized_entry = dict()
    for key, val in entry.iteritems():
        normalized_entry[key] = normalize_value(val)
    return normalized_entry

def normalize_value(value):
    normalized_value = None
    try:
        normalized_value = int(value)
    except ValueError:
        normalized_value = float(value)
    return normalized_value

def load_results(results_path, grouping_col=None):
    results = defaultdict(list)
    with open(results_path, 'rb') as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for entry in csv_reader:
            normalized_entry = normalize_entry(entry)
            results[normalized_entry[grouping_col]].append(normalized_entry)
    return results
