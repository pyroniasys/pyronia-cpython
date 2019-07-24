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
        max_dep_depth.append(int(run_data[1].strip()))

    stats = OrderedDict()
    stats['min'] = (("%.4f" % min(exec_times)), ("%d" % min(max_dep_depth)))
    stats['mean'] = (("%.4f" % mean(exec_times)), ("%.2f" % mean(max_dep_depth)))
    stats['median'] = (("%.4f" % median(exec_times)), ("%.2f" % median(max_dep_depth)))
    stats['max'] = (("%.4f" % max(exec_times)), ("%d" % max(max_dep_depth)))
    stats['stddev'] = (("%.4f" % stdev(exec_times)), ("%.4f" % stdev(max_dep_depth)))
    return stats

def get_syscall_latency_stats(latencies):
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

def get_iters_latency_stats(latencies):
    exec_times = []
    max_dep_depth = []
    for l in latencies:
        run_data = l.split(' ')
        exec_times.append(float(run_data[0].strip()))
        max_dep_depth.append(int(run_data[1].strip()))

    stats = OrderedDict()
    stats['min'] = (("%.4f" % min(exec_times)), ("%d" % min(max_dep_depth)))
    stats['mean'] = (("%.4f" % mean(exec_times)), ("%.2f" % mean(max_dep_depth)))
    stats['median'] = (("%.4f" % median(exec_times)), ("%.2f" % median(max_dep_depth)))
    stats['max'] = (("%.4f" % max(exec_times)), ("%d" % max(max_dep_depth)))
    stats['stddev'] = (("%.4f" % stdev(exec_times)), ("%.4f" % stdev(max_dep_depth)))

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

        out = open(app_path+'/app_'+bt+'-'+d+'_stats.txt', 'w+')
        json.dump(app_stats, out, indent=4)
        out.close()
