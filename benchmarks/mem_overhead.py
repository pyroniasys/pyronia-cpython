from statistics import mean, median, stdev
from collections import OrderedDict
import os
import json

def read_mprofile_file(filename):
    """Read an mprofile file and return its content.

    Returns
    =======
    content: dict
        Keys:

        - "mem_usage": (list) memory usage values, in MiB
        - "timestamp": (list) time instant for each memory usage value, in
            second
        - "func_timestamp": (dict) for each function, timestamps and memory
            usage upon entering and exiting.
        - 'cmd_line': (str) command-line ran for this profile.
    """
    func_ts = {}
    mem_usage = []
    timestamp = []
    children  = dict()
    cmd_line = None
    f = open(filename, "r")
    for l in f:
        if l == '\n':
            raise ValueError('Sampling time was too short')
        field, value = l.split(' ', 1)
        if field == "MEM":
            # mem, timestamp
            values = value.split(' ')
            mem_usage.append(float(values[0]))
            timestamp.append(float(values[1]))

        elif field == "FUNC":
            values = value.split(' ')
            f_name, mem_start, start, mem_end, end = values[:5]
            ts = func_ts.get(f_name, [])
            ts.append([float(start), float(end),
                       float(mem_start), float(mem_end)])
            func_ts[f_name] = ts

        elif field == "CHLD":
            values = value.split(' ')
            chldnum = values[0]
            children[chldnum].append(
                (float(values[1]), float(values[2]))
            )

        elif field == "CMDLINE":
            cmd_line = value.strip()
        else:
            pass
    f.close()

    return {"mem_usage": mem_usage, "timestamp": timestamp,
            "func_timestamp": func_ts, 'filename': filename,
            'cmd_line': cmd_line, 'children': children}

def calc_diff(new, orig):
    return float(new)-float(orig)

def calc_percent(new, orig):
    return ((calc_diff(new, orig))/float(orig))*100.0

runs = 5

data_path = os.path.expanduser('~')+'/Research/lib-isolation/cpython/'

apps = ['alexa', 'plant_watering', 'twitterPhoto']
btypes = ['nopyr', 'pyr']

stats = dict()
stats['min'] = []
stats['mean'] = []
stats['median'] = []
stats['max'] = []
stats['stddev'] = []
stats['pct'] = []
for b in btypes:
    for a in apps:
        if b == 'pyr' and a == 'alexa':
            continue
        points = OrderedDict()
        timestamps = []
        cmd_line = ""
        data_dict = dict()
        for i in range(1, runs+1):
            data_dict = read_mprofile_file(data_path+'benchmarks/'+a+'/'+a+'-mprofile-'+str(i)+'-'+b+'.data')
            ctr = 0
            for u in data_dict['mem_usage']:
                if points.get(str(ctr)) == None:
                    points[str(ctr)] = []
                points[str(ctr)].append(float(u))
                ctr += 1
            if len(data_dict['mem_usage']) != ctr:
                print("Mem usage data points mismatch: "+len(data_dict['mem_usage'])+" vs "+ctr)
            if len(data_dict['timestamp']) > len(timestamps):
                timestamps = data_dict['timestamp']
        outfile = data_path+'/benchmarks/'+a+'/'+a+'-mprofile-means-'+b+'.data'
        out = open(outfile, 'w+')
        out.write("CMDLINE "+data_dict['cmd_line']+"\n")
        for c, p in points.items():
            out.write("MEM %.6f %s\n" % (mean(p), timestamps[int(c)]))
        out.close()

        outfile = data_path+'/benchmarks/'+a+'/'+a+'-mprofile-medians-'+b+'.data'
        out = open(outfile, 'w+')
        out.write("CMDLINE "+data_dict['cmd_line']+"\n")
        for c, p in points.items():
            out.write("MEM %.6f %s\n" %(median(p), timestamps[int(c)]))
        out.close()

overhd_dict = OrderedDict()
for a in apps[1:]:
    overhd_dict[a] = OrderedDict()
    for b in btypes:
        pyr_mem_dict = read_mprofile_file(a+'/'+a+'-mprofile-medians-'+b+'.data')
        overhd_dict[a][b] = max(pyr_mem_dict['mem_usage'])
    overhd_dict[a]['overhead'] = "%.2f %%" % calc_percent(overhd_dict[a]['pyr'], overhd_dict[a]['nopyr'])

overhd_dict['alexa'] = OrderedDict()
pyr_mem_dict = read_mprofile_file('alexa/alexa-mprofile-medians-nopyr.data')
overhd_dict['alexa']['nopyr'] = max(pyr_mem_dict['mem_usage'])

outfile = data_path+'/benchmarks/mprofile_overhead.txt'
f = open(outfile, 'w+')
json.dump(overhd_dict, f, indent=4)
f.close()
