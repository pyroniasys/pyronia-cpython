from statistics import mean, median, stdev
import json
from collections import OrderedDict

app_path = '/home/marcela/Research/lib-isolation/cpython'

apps = ['hello', 'twitterPhoto', 'alexa', 'plant_watering']

bench_types = ['e2e-latency', 'syscall-latency']

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
    open_times = []
    fopen_times = []
    connect_times = []
    open_idx = 0
    fopen_idx = 1
    connect_idx = 2
    for l in latencies:
        run_data = l.split(',')
        open_bench = run_data[open_idx].split(' ')
        fopen_bench = run_data[fopen_idx].split(' ')
        connect_bench = run_data[connect_idx].split(' ')
        open_num = float(open_bench[0].strip())
        fopen_num = float(fopen_bench[0].strip())
        connect_num = float(connect_bench[0].strip())
        if open_num > 0.0:
            open_times.append(float(open_bench[1].strip())/open_num)
        if fopen_num > 0.0:
            fopen_times.append(float(fopen_bench[1].strip())/fopen_num)
        if connect_num > 0.0:
            connect_times.append(float(connect_bench[1].strip())/connect_num)

    if len(open_times) == 0:
        open_times = [0.0, 0.0]
    if len(fopen_times) == 0:
        fopen_times = [0.0, 0.0]
    if len(connect_times) == 0:
        connect_times = [0.0, 0.0]

    stats = OrderedDict()
    stats['open'] = OrderedDict()
    stats['fopen'] = OrderedDict()
    stats['connect'] = OrderedDict()
    stats['open']['times'] = open_times;
    stats['fopen']['times'] = fopen_times;
    stats['connect']['times'] = connect_times

    for s in stats:
        stats[s]['stats'] = OrderedDict()
        stats[s]['stats']['min'] = "%.2f" % min(stats[s]['times'])
        stats[s]['stats']['mean'] = "%.2f" % mean(stats[s]['times'])
        stats[s]['stats']['median'] = "%.2f" % median(stats[s]['times'])
        stats[s]['stats']['max'] = "%.2f" % max(stats[s]['times'])
        stats[s]['stats']['stddev'] = "%.2f" % stdev(stats[s]['times'])
    return stats

def get_iters_latency_stats(latencies):
    iter_times = OrderedDict()
    for i in range(0, 100):
        iter_times[str(i)] = []
    for l in latencies:
        run_data = l.split(' ')
        for i in range(0, len(run_data)):
            iter_times[str(i)].append(float(run_data[i].strip()))

    iter_means = []
    iter_medians = []
    iter_stddev = []
    iter_mins = []
    iter_max = []
    for i,times in iter_times.items():
        iter_mins.append("%.2f" % min(times))
        iter_means.append("%.2f" % mean(times))
        iter_medians.append("%.2f" % median(times))
        iter_max.append("%.2f" % max(times))
        iter_stddev.append("%.2f" % stdev(times))
            
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
        for a in apps:
            f = open(app_path+'/benchmarks/'+a+'/'+a+'-'+bt+'-'+d+'.data', 'r')
            latencies = f.readlines()
            f.close()
            stats = OrderedDict()
            if bt == 'e2e-latency':
                stats = get_e2e_latency_stats(latencies)
            elif bt == 'syscall-latency':
                stats = get_syscall_latency_stats(latencies)
            elif bt == 'iters-latency':
                stats = get_iters_latency_stats(latencies)

            app_stats[a] = stats

        out = open(app_path+'/benchmarks/app_'+bt+'-'+d+'_stats.txt', 'w+')
        json.dump(app_stats, out, indent=4)
        out.close()
