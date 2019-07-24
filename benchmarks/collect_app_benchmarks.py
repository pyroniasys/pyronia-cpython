# Run 10 trials for a given application, and collect the end-to-end latency for  each run                         

import os
import sys
import subprocess

runs = 25

if len(sys.argv) != 2:
    print('Usage: python collect_app_benchmarks.py <benchmark type>')
    exit(0)
    
bench_type = sys.argv[1]

apps = ['hello', 'twitterPhoto', 'alexa', 'plant_watering']
apps_path = '/home/pyronia/libpyronia/apps'

for a in apps:
    for x in range (0, runs):
        p = subprocess.call(['../pyronia_build/python', apps_path+'/'+a+'/'+a+'.py'])
        print('Finished run '+str(x+1)+' for app '+a)

    os.system('mv '+apps_path+'/'+a+'/'+a+'.py.data '+a+'/'+a+'-'+bench_type+'.data')
