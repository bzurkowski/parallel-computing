import argparse
import matplotlib.pyplot as plt

from loader   import load_results
from operator import itemgetter
from metrics  import plot_metric

if __name__ == '__main__':
    # parse args
    parser = argparse.ArgumentParser()
    parser.add_argument('results_path', action='store', help='path to file containing results in csv format')
    parser.add_argument('--output',     action='store', help='path to output plot file')
    args = parser.parse_args()

    # load results
    results = load_results(args.results_path, grouping_col='ncores')
    results = sorted(results.iteritems())

    # plot performance
    plt.figure(figsize=(12, 8), dpi=80)

    plot_opts = {
        'label': lambda ncores: '%s cores' % ncores,
        'title': 'Monte Carlo approximation of PI number - performance of parallel computation'
    }
    legend_opts = {'loc': 'upper left', 'ncol': 1}

    plot_metric('npoints', 'time', results,
        metric_arg_name='Number of points',
        metric_val_name='Walltime [s]',
        plot_opts=plot_opts,
        legend_opts=legend_opts
    )

    # plt.show()
    output_path = args.output if args.output else 'output.png'
    plt.savefig(output_path, dpi=100)
