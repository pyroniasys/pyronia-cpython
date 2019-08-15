from statistics import mean, median, stdev
import json
from collections import OrderedDict

app_path = '/home/marcela/Research/lib-isolation/cpython'

apps = ['hello', 'alexa', 'plant_watering', 'twitterPhoto']

bench_types = ['e2e-latency', 'syscall-latency', 'iter-latency']

datasets = ['nopyr', 'pyr']

def get_e2e_latency_stats(latencies):
    exec_times = []
    max_dep_depth = []
    for l in latencies:
        run_data = l.split(' ')
        exec_times.append(float(run_data[0].strip()))
        max_dep_depth.append(int(run_data[1].strip()[:-1]))

    stats = OrderedDict()
    stats['min'] = (("%.4f" % min(exec_times)), ("%d" % min(max_dep_depth)))
    stats['mean'] = (("%.4f" % mean(exec_times)), ("%.2f" % mean(max_dep_depth)))
    stats['median'] = (("%.4f" % median(exec_times)), ("%.2f" % median(max_dep_depth)))
    stats['max'] = (("%.4f" % max(exec_times)), ("%d" % max(max_dep_depth)))
    stats['stddev'] = (("%.4f" % stdev(exec_times)), ("%.4f" % stdev(max_dep_depth)))
    return stats

syscalls = ['open', 'fopen', 'connect']

def get_syscall_latency_stats(latencies):
    stats = OrderedDict()
    for s in syscalls:
        stats[s] = OrderedDict()
        stats[s]['num'] = []
        stats[s]['times'] = []
    for l in latencies:
        run_data = l.split(',')
        idx = 0
        if len(run_data) != len(syscalls):
            exit(-1)
        for s in syscalls:
            bench = run_data[idx].split(' ')
            num = float(bench[0].strip())
            stats[s]['num'].append(num)
            if num > 0.0:
                stats[s]['times'].append(float(bench[1].strip())/num)
            idx += 1

    for s in syscalls:
        if len(stats[s]['times']) == 0:
            stats[s]['times'] = [0.0, 0.0]

    for s in stats:
        stats[s]['stats'] = OrderedDict()
        stats[s]['stats']['min'] = "%.2f" % min(stats[s]['times'])
        stats[s]['stats']['mean'] = "%.2f" % mean(stats[s]['times'])
        stats[s]['stats']['median'] = "%.2f" % median(stats[s]['times'])
        stats[s]['stats']['max'] = "%.2f" % max(stats[s]['times'])
        stats[s]['stats']['stddev'] = "%.2f" % stdev(stats[s]['times'])
        stats[s]['stats']['median num'] = "%.2f" % median(stats[s]['num'])
    return stats

def get_iters_latency_stats(latencies):
    iter_times = OrderedDict()
    for i in range(0, 100):
        iter_times[str(i)] = []
    for l in latencies:
        run_data = l.split(' ')
        for i in range(0, len(run_data)):
            iter_times[str(i)].append(float(run_data[i].strip())*1000.0)
            
    iter_means = []
    iter_medians = []
    iter_stddev = []
    iter_mins = []
    iter_max = []
    for i,times in iter_times.items():
        iter_mins.append("%.4f" % min(times))
        iter_means.append("%.4f" % mean(times))
        iter_medians.append("%.4f" % median(times))
        iter_max.append("%.4f" % max(times))
        iter_stddev.append("%.4f" % stdev(times))
            
    stats = OrderedDict()
    stats['min'] = iter_mins
    stats['mean'] = iter_means
    stats['median'] = iter_medians
    stats['max'] = iter_max
    stats['stddev'] = iter_stddev
    return stats

for d in datasets:
    for bt in bench_types:
        app_stats = OrderedDict()
        for a in apps[1:]:
            f = open(app_path+'/benchmarks/'+a+'/'+a+'-'+bt+'-'+d+'.data', 'r')
            latencies = f.readlines()
            f.close()
            stats = OrderedDict()
            if bt == 'e2e-latency':
                stats = get_e2e_latency_stats(latencies)
            elif bt == 'syscall-latency':
                stats = get_syscall_latency_stats(latencies)
            elif bt == 'iter-latency':
                stats = get_iters_latency_stats(latencies)

            app_stats[a] = stats

        out = open(app_path+'/benchmarks/app_'+bt+'-'+d+'_stats.txt', 'w+')
        json.dump(app_stats, out, indent=4)
        out.close()
