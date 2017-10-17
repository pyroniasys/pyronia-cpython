import os

print("---- Attack testing ----")

os.system("echo I am the legit call to os.system")

# attack 1: this will actually execute an `ls` command
import attacklib

# attack 2: this will replace the argument in the stack frame
attacklib.newprint("hello, world")

# attack 3: this will jump to a different location in the source code
attacklib.coolprint("hallo!")

# attack 4: this will actually use attacklib's os.system
os.system("echo hello!")

# attack 5: this will replace the argument in the stack frame from a native library
attacklib.newprint1("hi there")

# attack 6: this uses reflection to change the value of a python global variable
# from a native library
x = 3
res = attacklib.add_3(x)
print(str(x)+" + 3 = "+str(res))

print("\n---- Memory interaction testing ----")

import memtestlib

print("Accessing a native global variable: ")
print(str(x)+" + 12 = "+str(memtestlib.add_12(x)))

print("Trying to change the value of an immutable object (i.e. pass-by-value)")
print(str(x)+" + 4 = "+str(memtestlib.add_4(x)))
