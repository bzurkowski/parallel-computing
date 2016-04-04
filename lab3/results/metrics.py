import argparse
import matplotlib.pyplot as plt

from loader   import load_results
from operator import itemgetter
from string   import capitalize
from copy     import deepcopy

def plot_metric(metric_arg, metric_val, results, **kwargs):
    plot_opts   = kwargs.pop('plot_opts', {}).copy()
    legend_opts = kwargs.pop('legend_opts', {}).copy()

    # series label
    label_series = None
    if plot_opts.has_key('label') and callable(plot_opts['label']):
        label_series = plot_opts.pop('label')
    else:
        label_series = lambda val: val

    # title
    if plot_opts.has_key('title'):
        plt.title(plot_opts.pop('title'), size=24)
    else:
        plt.title(kwargs.get('metric_val_name', capitalize(metric_val)), size=24)

    # axis labels
    plt.xlabel(kwargs.get('metric_arg_name', capitalize(metric_arg)))
    plt.ylabel(kwargs.get('metric_val_name', capitalize(metric_val)))

    # first n rows offset
    nskip = kwargs.get('nskip', 0)

    # plotting
    for series_subject, series_results in results:
        # sort series results by metric arg
        sorted_series_results = sorted(series_results, key=itemgetter(metric_arg))[nskip:]

        # fetch metric args and values
        metric_args = collect_key_values(metric_arg, sorted_series_results)
        metric_vals = collect_key_values(metric_val, sorted_series_results)

        # label series
        series_label = label_series(series_subject)

        # plot series
        plt.plot(metric_args, metric_vals, label=series_label, **plot_opts)

    # draw legend
    legend = plt.legend(**legend_opts)
    legend.get_frame().set_alpha(0.7)

def calculate_metrics(results, scalable=False, key_mappings={}):
    results_copy = deepcopy(results)

    # resolve key mappings
    time_key   = key_mappings.get('time', 'time')
    ncores_key = key_mappings.get('ncores', 'ncores')

    for _, series_results in results_copy:
        # sort series results by metric arg
        sorted_series_results = sorted(series_results, key=itemgetter(ncores_key))

        # calculate execution time for single core
        single_core_time = sorted_series_results[0][time_key]

        # select metrics function
        perform_metrics_calculation = None
        if scalable:
            perform_metrics_calculation = calculate_scalable_metrics
        else:
            perform_metrics_calculation = calculate_nonscalable_metrics

        # calculate metrics for each series entry
        for results_entry in sorted_series_results[1:]:
            time   = results_entry[time_key]
            ncores = results_entry[ncores_key]

            metrics = perform_metrics_calculation(single_core_time, time, ncores)

            results_entry['speedup']         = metrics['speedup']
            results_entry['efficiency']      = metrics['efficiency']
            results_entry['serial_fraction'] = metrics['serial_fraction']

    return results_copy

def calculate_nonscalable_metrics(single_core_time, time, ncores):
    speedup = single_core_time / time

    return {
        'speedup':         speedup,
        'efficiency':      speedup / ncores,
        'serial_fraction': (1 / speedup - 1 / ncores) / (1 - 1 / ncores)
    }

def calculate_scalable_metrics(single_core_time, time, ncores):
    speedup = single_core_time / time * ncores

    return {
        'speedup':         speedup,
        'efficiency':      speedup / ncores,
        'serial_fraction': (1 / speedup - 1 / ncores) / (1 - 1 / ncores)
    }

def collect_key_values(key, results):
    return map(itemgetter(key), results)
