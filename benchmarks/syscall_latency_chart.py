import numpy as np
import matplotlib.pyplot as plt
import json
from collections import OrderedDict
from statistics import mean

nopyr_data = dict()
pyr_data = dict()
with open('app_syscall-latency-pyr_stats.txt', 'r') as fp:
    pyr_data = json.load(fp, object_pairs_hook=OrderedDict)

with open('app_syscall-latency-nopyr_stats.txt', 'r') as fp:
    nopyr_data = json.load(fp, object_pairs_hook=OrderedDict)

apps = ['alexa', 'plant_watering', 'twitterPhoto']
syscalls = ['open', 'fopen', 'connect']

ind = np.arange(len(syscalls))  # the x locations for the groups
width = 0.34  # the width of the bars

pyr_means_d = dict()
pyr_stddev_d = dict()
nopyr_means_d = dict()
nopyr_stddev_d = dict()

for s in syscalls:
    pyr_means_d[s] = []
    pyr_stddev_d[s] = []
    nopyr_means_d[s] = []
    nopyr_stddev_d[s] = []

# gather pyr and no_pyr lists
for a in apps:
    for s in syscalls:
        pyr_means_d[s].append(float(pyr_data[a][s]['stats']['mean']))
        pyr_stddev_d[s].append(float(pyr_data[a][s]['stats']['stddev']))

        nopyr_means_d[s].append(float(nopyr_data[a][s]['stats']['mean']))
        nopyr_stddev_d[s].append(float(nopyr_data[a][s]['stats']['stddev']))

pyr_means = []
pyr_stddev = []
nopyr_means = []
nopyr_stddev = []
for s in syscalls:
    pyr_means.append(mean(pyr_means_d[s]))
    pyr_stddev.append(mean(pyr_stddev_d[s]))
    nopyr_means.append(mean(nopyr_means_d[s]))
    nopyr_stddev.append(mean(nopyr_stddev_d[s]))

fig, ax = plt.subplots()
rects1 = ax.bar(ind - width/2, nopyr_means, width, yerr=nopyr_stddev, color='IndianRed', label='syscall')
rects2 = ax.bar(ind + width/2, pyr_means, width, yerr=pyr_stddev,
                color='SkyBlue', label='+Pyronia')

# Add some text for labels, title and custom x-axis tick labels, etc.
#ax.set_yscale('log')
ax.set_ylabel('time (us)')
ax.set_xticks(ind)
ax.set_xticklabels(syscalls)

plt.legend(loc='upper left')
fig.savefig('pyr-syscall-latency.pdf')

'''
def autolabel(rects, xpos='center'):
    """
    Attach a text label above each bar in *rects*, displaying its height.

    *xpos* indicates which side to place the text w.r.t. the center of
    the bar. It can be one of the following {'center', 'right', 'left'}.
    """

    xpos = xpos.lower()  # normalize the case of the parameter
    ha = {'center': 'center', 'right': 'left', 'left': 'right'}
    offset = {'center': 0.5, 'right': 0.57, 'left': 0.43}  # x_txt = x + w*off

    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()*offset[xpos], 1.01*height,
                '{}'.format(height), ha=ha[xpos], va='bottom')


autolabel(rects1, "left")
autolabel(rects2, "right")
'''
