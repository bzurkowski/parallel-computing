import sys, re, urllib
from pyspark import SparkContext, SparkConf


APP_NAME = 'Scalarm Logs Analyzer'


def filter_actions(entry):
    return re.match(r'^(\d+)\.(\d+)\.(\d+)\.(\d+)', entry)

def normalize_action_and_time(entry):
    entry_parts = entry.split()
    action, time = entry_parts[-5], entry_parts[-1]

    action = urllib.unquote(action)
    action = remove_query_params(action)

    action_parts = action.split('/')
    normalized_action_parts = []

    for action_part in action_parts:
        if not action_part:
            continue

        if re.match(r'^[a-zA-z_]+$', action_part):
            normalized_action_parts.append(action_part)
        else:
            normalized_action_parts.append(':normalized')

    normalized_action = '/'.join(normalized_action_parts)

    return normalized_action, float(time)

def remove_query_params(action):
    return re.sub(r'\?(.+)$', '', action)

def analyze(sc, logs_path):
    # initialize logs RDD
    lines = sc.textFile(logs_path)

    # filter out actions from log entries
    actions = lines.filter(filter_actions)

    # extract and normalize actions and times
    normalized_actions = actions.map(normalize_action_and_time)

    # calculate max time for each action
    actions_max = normalized_actions.reduceByKey(
        lambda time1, time2: time1 if time1 > time2 else time2
    )

    # sort acitons by their max times
    sorted_actions_max = actions_max.sortBy((lambda (_, max_time): max_time), ascending=False)
    sorted_actions_max.collect()

    # take 10 actions with highest max time
    highest_actions_max = sorted_actions_max.take(10)

    return list(highest_actions_max)


if __name__ == '__main__':
    conf = SparkConf().setAppName(APP_NAME)
    conf = conf.setMaster('local[*]')
    sc   = SparkContext(conf=conf)

    if len(sys.argv) != 2:
        print 'usage: analyze.py logs_path [output_path]'

    logs_path = sys.argv[1]
    if len(sys.argv) > 2:
        output_path = sys.argv[2]

    results = analyze(sc, logs_path)

    if output_path:
        with open(output_path, 'w+') as output_file:
            output_file.write(str(results))
