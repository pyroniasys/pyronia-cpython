import json
from collections import OrderedDict
import sys
from statistics import mean

def calc_diff(new, orig):
    return float(new)-float(orig)

def calc_percent(new, orig):
    return ((calc_diff(new, orig))/float(orig))*100.0

def calc_multi(new, orig):
    return int(float(new)/float(orig))

app_path = '/home/marcela/Research/lib-isolation/cpython'

apps = ['alexa', 'plant_watering', 'twitterPhoto']

bench_types = ['e2e-latency', 'syscall-latency', 'iter-latency']

datasets = ['nopyr', 'pyr']

syscalls = ['open', 'fopen', 'connect']

overheads = OrderedDict()
overheads['mean e2e latency overhead'] = OrderedDict()
overheads['mean syscall latency overhead'] = OrderedDict()
overheads['mean per-iter latency overhead'] = OrderedDict()
for bt in bench_types:
    nopyr_data = dict()
    pyr_data = dict()
    with open(app_path+'/benchmarks/app_'+bt+'-pyr_stats.txt', 'r') as fp:
        pyr_data = json.load(fp, object_pairs_hook=OrderedDict)
    with open(app_path+'/benchmarks/app_'+bt+'-nopyr_stats.txt', 'r') as fp:
        nopyr_data = json.load(fp, object_pairs_hook=OrderedDict)
    
    # gather pyr and no_pyr lists
    if bt == 'e2e-latency':
        for a in apps:
            pyr_mean = float(pyr_data[a]['mean'][0])/1000000.0
            nopyr_mean = float(nopyr_data[a]['mean'][0])/1000000.0

            overheads['mean e2e latency overhead'][a] = ("%.2f seconds (%.1fx)" % (pyr_mean, calc_multi(pyr_mean, nopyr_mean)))
    elif bt == 'syscall-latency':
        overheads['mean syscall latency overhead']['open'] = OrderedDict()
        overheads['mean syscall latency overhead']['fopen'] = OrderedDict()
        overheads['mean syscall latency overhead']['connect'] = OrderedDict()
        for a in apps:
            for s in syscalls:
                pyr_mean = float(pyr_data[a][s]['stats']['mean'])
                nopyr_mean = float(nopyr_data[a][s]['stats']['mean'])
                if nopyr_mean > 0.0:
                    overheads['mean syscall latency overhead'][s][a] = ("%.1fx (%.2f us)" % (calc_multi(pyr_mean, nopyr_mean), calc_diff(pyr_mean, nopyr_mean)))
    elif bt == 'iter-latency':
        for a in apps:
            pyr_mean = [float(t) for t in pyr_data[a]['mean']]
            pyr_first = pyr_mean[0]
            pyr_mean = mean(pyr_mean[5:])
            nopyr_mean = [float(t) for t in nopyr_data[a]['mean']]
            nopyr_first = nopyr_mean[0]
            nopyr_mean = mean(nopyr_mean[5:])
            overheads['mean per-iter latency overhead'][a] = ("First iter %.1fx, avg iter %.2f ms (%.1fx)" % (calc_multi(pyr_first, nopyr_first), pyr_mean, calc_multi(pyr_mean, nopyr_mean)))

out = open(app_path+'/benchmarks/pyronia_overheads.txt', 'w+')
json.dump(overheads, out, indent=4)
out.close()
