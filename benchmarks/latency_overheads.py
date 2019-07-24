import json
from collections import OrderedDict
import sys

def calc_diff(new, orig):
    return float(new)-float(orig)

def calc_percent(new, orig):
    return ((calc_diff(new, orig))/float(orig))*100.0

def calc_multi(new, orig):
    return int(float(new)/float(orig))

app_path = '/home/marcela/Research/lib-isolation/cpython'

apps = ['hello', 'twitterPhoto', 'alexa', 'plant_watering']

bench_types = ['e2e-latency', 'syscall-latency']

datasets = ['nopyr', 'pyr']

syscalls = ['open', 'fopen', 'connect']

overheads = OrderedDict()
overheads['mean e2e latency overhead'] = OrderedDict()
overheads['mean syscall latency overhead'] = OrderedDict()
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

            overheads['mean e2e latency overhead'][a] = ("%.2f seconds (%.1f%%)" % (calc_diff(pyr_mean, nopyr_mean), calc_percent(pyr_mean, nopyr_mean)))
    elif bt == 'syscall-latency':
        overheads['mean syscall latency overhead']['open'] = OrderedDict()
        overheads['mean syscall latency overhead']['fopen'] = OrderedDict()
        overheads['mean syscall latency overhead']['connect'] = OrderedDict()
        for a in apps:
            for s in syscalls:
                pyr_mean = float(pyr_data[a][s]['stats']['mean'])
                nopyr_mean = float(nopyr_data[a][s]['stats']['mean'])
                if nopyr_mean > 0.0:
                    overheads['mean syscall latency overhead'][s][a] = ("%.1f%% (%.2f us)" % (calc_percent(pyr_mean, nopyr_mean), calc_diff(pyr_mean, nopyr_mean)))

out = open(app_path+'/benchmarks/pyronia_overheads.txt', 'w+')
json.dump(overheads, out, indent=4)
out.close()
