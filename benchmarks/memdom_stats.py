from statistics import mean, median, stdev
import json
import os
from collections import OrderedDict

data_path = os.path.expanduser('~')+'/Research/lib-isolation/cpython/'

apps = ['alexa', 'hello', 'plant_watering', 'twitterPhoto']

outfile = 'benchmarks/app_memdom-iter_stats.txt'

runs = 25

app_stats = OrderedDict()

for a in apps:
    num_memdoms = []
    total_allocs = []
    total_metadata_allocs = []

    f = open(data_path+'benchmarks/'+a+'/'+a+'-memusage-iter-pyr.data', 'r')
    mem_data = [l.strip() for l in f.readlines()]
    f.close()

    for l in mem_data:
        run_data = l.split(', ')
        total_allocs.append(int(run_data[0]))
        total_metadata_allocs.append(int(run_data[1]))
        num_memdoms.append(int(run_data[2]))

    stats = OrderedDict()
    stats['total usage'] = OrderedDict()
    stats['metadata usage'] = OrderedDict()
    stats['num pages'] = OrderedDict()

    stats['total usage']['min'] = min(total_allocs)
    stats['total usage']['mean'] = mean(total_allocs)
    stats['total usage']['median'] = median(total_allocs)
    stats['total usage']['max'] = max(total_allocs)
    stats['total usage']['stddev'] = stdev(total_allocs)

    stats['metadata usage']['min'] = min(total_metadata_allocs)
    stats['metadata usage']['mean'] = mean(total_metadata_allocs)
    stats['metadata usage']['median'] = median(total_metadata_allocs)
    stats['metadata usage']['max'] = max(total_metadata_allocs)
    stats['metadata usage']['stddev'] = stdev(total_metadata_allocs)

    stats['num pages']['min'] = min(num_memdoms)
    stats['num pages']['mean'] = mean(num_memdoms)
    stats['num pages']['median'] = median(num_memdoms)
    stats['num pages']['max'] = max(num_memdoms)
    stats['num pages']['stddev'] = stdev(num_memdoms)
    app_stats[a] = stats

out = open(data_path+outfile, 'w+')
json.dump(app_stats, out, indent=4)
out.close()
