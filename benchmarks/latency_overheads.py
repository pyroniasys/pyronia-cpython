import json
from collections import OrderedDict
import sys

def calc_diff(new, orig):
    return float(new)-float(orig)

def calc_percent(new, orig):
    return (calc_diff(new, orig))/float(orig)

def calc_multi(new, orig):
    return int(float(new)/float(orig))

app_path = '/Users/marcela/Research/lib-isolation/cpython'

apps = ['hello', 'twitterPhoto', 'alexa', 'plant_watering']

bench_types = ['e2e-latency', 'syscall-latency']

datasets = ['nopyr', 'pyr']

overheads = OrderedDict()
overheads['mean e2e latency overhead'] = OrderedDict()
overheads['mean syscall latency overhead'] = OrderedDict()
for bt in bench_types:
    nopyr_data = dict()
    pyr_data = dict()
    with open(app_path+'/app_'+bt+'-pyr_stats.txt', 'r') as fp:
        pyr_data = json.load(fp, object_pairs_hook=OrderedDict)
    with open(app_path+'/app_'+bt+'-nopyr_stats.txt', 'r') as fp:
        nopyr_data = json.load(fp, object_pairs_hook=OrderedDict)
    
    for a in apps:
        # gather pyr and no_pyr lists
        if bt == 'e2e-latency':
            pyr_mean = float(pyr_data[a]['mean'][0])/1000000.0
            nopyr_mean = float(nopyr_data[a]['mean'][0])/1000000.0

            overheads['mean e2e latency overhead'][a] = ("%.2f seconds (%.1f %)" % (calc_diff(pyr_mean, nopyr_mean), calc_percent(pyr_mean, nopyr_mean)))
        elif bt == 'syscall-latency':
            
            overheads['mean module-only overhead'][a] = ("%.2f seconds (%dx)" % (calc_diff(pyr_mean, nopyr_mean), calc_multi(pyr_mean, nopyr_mean)))

    pyr_mem_dict = read_mprofile_file(a+'/mprofile_'+a+'-medians-pyr.dat')
    nopyr_mem_dict = read_mprofile_file(a+'/mprofile_'+a+'-medians-nopyr.dat')
    overheads['max median mem overhead'][a] = ("%.2f MB (%dx)" % (calc_diff(max(pyr_mem_dict['mem_usage']), max(nopyr_mem_dict['mem_usage'])), calc_multi(max(pyr_mem_dict['mem_usage']), max(nopyr_mem_dict['mem_usage']))))

json.dump(overheads, sys.stdout, indent=4)
