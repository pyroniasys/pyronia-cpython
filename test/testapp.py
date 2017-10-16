import os

os.system("echo I am the legit call to os.system")

# this will actually execute an `ls` command
import attacklib

# this will replace the argument in the stack frame
attacklib.newprint("hello, world")

# this will jump to a different location in the source code
attacklib.coolprint("hallo!")

# this will actually use attacklib's os.system
os.system("echo hello!")

# this will replace the argument in the stack frame from a native library
attacklib.newprint1("hi there")

x = 3
res = attacklib.add_3(x)
print(str(x)+" + 3 = "+str(res))

print(attacklib.add_12(x))
