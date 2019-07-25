import numpy as np
import matplotlib.pyplot as plt
import json
from collections import OrderedDict

nopyr_data = dict()
pyr_data = dict()
with open('app_iter-latency-pyr_stats.txt', 'r') as fp:
    pyr_data = json.load(fp, object_pairs_hook=OrderedDict)

with open('app_iter-latency-nopyr_stats.txt', 'r') as fp:
    nopyr_data = json.load(fp, object_pairs_hook=OrderedDict)

apps = ['alexa', 'plant_watering', 'twitterPhoto']

ind = np.arange(1.0, 101.0, 1.0)  # the x locations for the groups
fig, ax = plt.subplots()

mem_line_colors = ("k", "b", "r", "g", "c", "m", "y")

# gather pyr and no_pyr lists
idx = 0
plot_idx = 1
for a in apps:
    plt.subplot(len(apps), 1, plot_idx)
    nopyr_times = [float(t) for t in nopyr_data[a]['mean']]
    pyr_times = [float(t) for t in pyr_data[a]['mean']]
    is_millis = False
    if min(nopyr_times) > 1000.0:
        nopyr_times = [t/1000.0 for t in nopyr_times]
        pyr_times = [t/1000.0 for t in pyr_times]
        is_millis = True
    color = mem_line_colors[idx % len(mem_line_colors)]
    plt.plot(ind, pyr_times, color, nopyr_times, color+'--')
    plt.title(a)
    ax.set_yticks(np.arange(int(min(nopyr_times)), int(max(pyr_times)), 5.0))
    ax.set_xticks(np.arange(1.0, 100.0, 1.0))
    if is_millis == True:
        plt.ylabel('millisecs')
    else:
        plt.ylabel('microsecs')
        # Add some text for labels, title and custom x-axis tick labels, etc.
    #ax.set_yscale('log')
    idx += 1
    plot_idx += 1

#plt.show()
fig.tight_layout()
fig.add_subplot(111, frameon=False)
plt.tick_params(labelcolor='none', top=False, bottom=False, left=False, right=False)
plt.grid(False)
plt.xlabel('number of iterations')
fig.savefig('apps-iter-latency.pdf')

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
