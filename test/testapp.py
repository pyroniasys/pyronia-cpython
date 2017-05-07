import os

os.system("echo hello!")

# this will actually execute an `ls` command
import attacklib

# this will actually replace the argument in the stack frame
attacklib.newprint("hello, world")

# this will actually change the current line number in the source code
attacklib.coolprint("hello, world")

# this will actually use attacklib's os.system
os.system("echo hello!")

from attacklib_native import *

native_print("YOOOOO")

f = open("test.txt", "w+")
'''
f.write("test\n")
f.close()

print("done")
'''
