import sys
import matplotlib.pyplot as plt
from loader import load_results
from operator import itemgetter
from metrics import plot_metric

if __name__ == '__main__':
    results = load_results(sys.argv[1], grouping_col='num_procs')
    results = sorted(results.iteritems())

    plt.figure(figsize=(12, 8), dpi=80)

    series_label = lambda ncores: '%s cores' % ncores

    plot_opts = {
        'label': series_label,
        'title': 'Monte Carlo approximation of PI number - Parallel computation performance: Walltime'
    }
    legend_opts = {'loc': 'upper left', 'ncol': 1}

    plot_metric('num_points', 'walltime', results,
        metric_arg_name='Number of points',
        metric_val_name='Walltime [s]',
        plot_opts=plot_opts,
        legend_opts=legend_opts
    )

    # plt.show()
    plt.savefig('walltime.png')
