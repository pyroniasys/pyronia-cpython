import os

print("\n---- Memory interaction testing ----")

import memtestlib

x = 3

# test 1: A native lib shouldn't be able to change the value of an
# immutable Python object (corresponds to pass-by-value)
print("test 1: Trying to change the value of an immutable object (expect AssertionError if the change succeeds)")
print("Adding 4 to "+str(x)+"; want "+str(x+4)+", get "+str(memtestlib.add_4(x)))

# test 2: A native lib should be able to change the value of a
# mutable Python object (corresponds to pass-by-reference)
print("test 2: Trying to change the value of a mutable object (expect AssertionError if the change fails)")
print("Adding 5 to "+str(x)+"; want "+str(x+5)+", get "+str(memtestlib.add_5(x)))

# test 3: Python should be able to change the value of a native global
print("test 3: Accessing a native global variable: ")
print("Adding 12 to "+str(x)+"; want "+str(x+12)+", get "+str(memtestlib.add_12(x)))

# test 4: The native library will call back into the print_result function
# in the main Python program
def print_result(x):
    print(x)

print("test 4: Making a callback from native into the main Python program:")
memtestlib.sum(3, print_result)

# test 5: Perform a longjump from a native back into the print_result function
# in the main Python program
print("test 5: longjmp'ing from native into the main Python program:")
memtestlib.prettify("hey you", print_result)
