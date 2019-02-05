# Run 10 trials for a given application, and collect the end-to-end latency for  each run                         

import os
import sys
import subprocess

runs = 25

app = sys.argv[1]

for x in range (0, runs):
    subprocess.call(['./pyronia_build/python', os.path.abspath(app)])
    print('Finished run '+str(x+1)+' for app '+app)
