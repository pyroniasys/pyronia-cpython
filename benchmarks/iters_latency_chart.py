import numpy as np
import matplotlib.pyplot as plt
import json
from collections import OrderedDict

def autoscale_y(ax,margin=0.1):
    """This function rescales the y-axis based on the data that is visible given the current xlim of the axis.
    ax -- a matplotlib axes object
    margin -- the fraction of the total height of the y-data to pad the upper and lower ylims"""

    def get_bottom_top(line):
        xd = line.get_xdata()
        yd = line.get_ydata()
        lo,hi = ax.get_xlim()
        y_displayed = yd[((xd>lo) & (xd<hi))]
        h = np.max(y_displayed) - np.min(y_displayed)
        bot = np.min(y_displayed)-margin*h
        top = np.max(y_displayed)+margin*h
        return bot,top

    lines = ax.get_lines()
    bot,top = np.inf, -np.inf

    for line in lines:
        new_bot, new_top = get_bottom_top(line)
        print(str(new_bot)+" "+str(new_top))
        if new_bot < bot: bot = new_bot
        if new_top > top: top = new_top

    ax.set_ylim(bot,top)

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
lgd = None
idx = 0
plot_idx = 1
for a in apps:
    plt.subplot(len(apps), 1, plot_idx)
    nopyr_times = [float(t) for t in nopyr_data[a]['mean']]
    pyr_times = [float(t) for t in pyr_data[a]['mean']]
    color = mem_line_colors[idx % len(mem_line_colors)]
    ax = plt.gca()
    ax.set_xticks(np.arange(0, 101, 10))
    ax.set_xlim([0,101])
    if int(max(pyr_times)) < 100:
        top_marg = 1
    else:
        top_marg = 5
    ax.set_ylim([0, int(max(pyr_times))+top_marg])
    plt.plot(ind, pyr_times, color+'--', label='+Pyronia')
    plt.plot(ind, nopyr_times, color, label='app')
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width*0.8, box.height])
    lgd = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.title(a)
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
plt.ylabel('time (ms)')
fig.savefig('apps-iter-latency.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')

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
