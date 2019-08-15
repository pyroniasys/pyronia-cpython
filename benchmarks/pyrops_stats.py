from statistics import mean, median, stdev
import json
from collections import OrderedDict

app_path = '/home/marcela/Research/lib-isolation/cpython'

apps = ['hello', 'twitterPhoto', 'alexa', 'plant_watering']

ops = ['alloc', 'free', 'grant', 'revoke', 'priv_add', 'priv_del', 'new_page', 'callstack_gen', 'callstack_hash']

def get_ops_latencies(latencies):
    stats = OrderedDict()
    for o in ops:
        stats[o] = OrderedDict()
        stats[o]['num'] = []
        stats[o]['times'] = []

    for l in latencies:
        run_data = l.split(',')
        idx = 0
        if len(run_data) != len(ops):
            exit(-1)
        for o in ops:
            bench = run_data[idx].split(' ')
            num = float(bench[0].strip())
            stats[o]['num'].append(num)
            if num > 0.0:
                stats[o]['times'].append(float(bench[1].strip())/num)
            idx += 1

    for o in ops:
        if len(stats[o]['times']) == 0:
            stats[o]['times'] = [0.0, 0.0]

    for o in stats:
        stats[o]['stats'] = OrderedDict()
        stats[o]['stats']['min'] = "%.2f" % min(stats[o]['times'])
        stats[o]['stats']['mean'] = "%.2f" % mean(stats[o]['times'])
        stats[o]['stats']['median'] = "%.2f" % median(stats[o]['times'])
        stats[o]['stats']['max'] = "%.2f" % max(stats[o]['times'])
        stats[o]['stats']['stddev'] = "%.2f" % stdev(stats[o]['times'])
        stats[o]['stats']['median num'] = "%.2f" % median(stats[o]['num'])
    return stats

app_stats = OrderedDict()
for a in apps:
    f = open(app_path+'/benchmarks/'+a+'/'+a+'-pyrops-latency.data', 'r')
    latencies = f.readlines()
    f.close()
    app_stats[a] = get_ops_latencies(latencies)

app_stats_agg = OrderedDict()
for o in ops:
    app_stats_agg[o] = OrderedDict()
    for a in apps:
        if o in app_stats[a]:
            app_stats_agg[o][a] = app_stats[a][o]['stats']

most_calls = OrderedDict()
for a in apps:
    app_max = 0.0
    max_op = ""
    for o in ops:
        if float(app_stats_agg[o][a]['median num']) > app_max:
            app_max = float(app_stats_agg[o][a]['median num'])
            max_op = o
    most_calls[a] = OrderedDict()
    most_calls[a]['op'] = max_op
    most_calls[a]['num calls'] = app_max

out = open(app_path+'/benchmarks/app_pyrops_latency_stats.txt', 'w+')
json.dump(app_stats, out, indent=4)
out.close()

out = open(app_path+'/benchmarks/app_pyrops_latency_agg_stats.txt', 'w+')
json.dump(app_stats_agg, out, indent=4)
out.close()

out = open(app_path+'/benchmarks/app_pyrops_latency_max.txt', 'w+')
json.dump(most_calls, out, indent=4)
out.close()
