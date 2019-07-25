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

out = open(app_path+'/benchmarks/app_pyrops_latency_stats.txt', 'w+')
json.dump(app_stats, out, indent=4)
out.close()
