import argparse
import matplotlib.pyplot as plt

from loader  import load_results
from metrics import calculate_metrics, plot_metric

if __name__ == '__main__':
    # parse args
    parser = argparse.ArgumentParser()
    parser.add_argument('results_path', action='store',      help='path to file containing results in csv format')
    parser.add_argument('--scalable',   action='store_true', help='switch to scalable metrics')
    parser.add_argument('--output',     action='store',      help='path to output plot file')
    args = parser.parse_args()

    # load results
    results = load_results(args.results_path, grouping_col='size')
    results = sorted(results.iteritems())

    # calculate metrics
    results = calculate_metrics(results, scalable=args.scalable)

    # plot metrics
    plt.figure(figsize=(22, 16), dpi=80)

    metrics_type = 'scalable' if args.scalable else 'non-scalable'
    plot_title = 'Matri multiplication - %s metrics of parallel computation' % metrics_type
    plt.suptitle(plot_title, size=24)

    plot_opts   = {'label': lambda size: 'Size: %s' % size}
    legend_opts = {'loc': 'upper center', 'ncol': 3}

    plt.subplot(2, 2, 1)
    plot_metric('ncores', 'speedup', results,
        metric_arg_name='Number of cores',
        metric_val_name='Speedup',
        nskip=1,
        plot_opts=plot_opts,
        legend_opts=legend_opts
    )

    plt.subplot(2, 2, 2)
    plot_metric('ncores', 'efficiency', results,
        metric_arg_name='Number of cores',
        metric_val_name='Efficiency',
        nskip=1,
        plot_opts=plot_opts,
        legend_opts=legend_opts
    )

    plt.subplot(2, 2, 3)
    plot_metric('ncores', 'serial_fraction', results,
        metric_arg_name='Number of cores',
        metric_val_name='Serial fraction',
        nskip=1,
        plot_opts=plot_opts,
        legend_opts=legend_opts
    )

    # plt.show()
    output_path = args.output if args.output else 'output.png'
    plt.savefig(output_path, dpi=100)
