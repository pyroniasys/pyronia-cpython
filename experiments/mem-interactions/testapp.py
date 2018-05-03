import os
from datetime import datetime

print("---- Memory interaction testing: %s ----" % datetime.now().strftime("%m-%d-%y %H:%M"))

import memtestlib
import native

x = 3

# test 1: A native lib shouldn't be able to change the value of an
# immutable Python object (corresponds to pass-by-value)
print("-- test 1: Trying to change the value of an immutable object (expect AssertionError if the change succeeds)")
print("Adding 4 to "+str(x)+"; want "+str(x+4)+", get "+str(memtestlib.add_4(x)))

# test 2: A native lib should be able to change the value of a
# mutable Python object (corresponds to pass-by-reference)
print("\n-- test 2: Trying to change the value of a mutable object (expect AssertionError if the change fails)")
print("Adding 5 to "+str(x)+"; want "+str(x+5)+", get "+str(memtestlib.add_5(x)))

# test 3: Python should be able to change the value of a native global
print("\n-- test 3: Accessing a native global variable: ")
print("Adding 12 to "+str(x)+"; want "+str(x+12)+", get "+str(memtestlib.add_12(x)))

# test 4: The native library will call back into the print_result function
# in the main Python program
def print_result(x):
    print(x)

print("\n-- test 4: Making a callback from native into the main Python program:")
memtestlib.sum(3, print_result)

# test 5: Perform a longjump from a native back into the print_result function
# in the main Python program
print("\n-- test 5: longjmp'ing from native into the main Python program:")
memtestlib.prettify("hey you", print_result)

# test 6: Compare the memory address of a first-party native extension's
# pointer to a function in a second first-party extension to the second
# extension's pointer to the same function
print("\n-- test 6: comparing mem addresses of extA.func and extB->extA.func:")
memtestlib.fp_ext_memaddr_cmp()

# test 7: Compare the memory address of a first-party native extension's
# pointer to a libC function to a second extension's pointer to the same
# libC function
print("\n-- test 7: comparing mem addresses of extA->libc_func and extB->libc_func:")
memtestlib.libc_memaddr_cmp()

# test 8: Compare the memory address of a native extension's
# pointer to an openssl function to a second extension's pointer to the same
# openssl function. In this case, libssl is dynamically linked into each extension.
print("\n-- test 8: comparing mem addresses of explcitly dyn. linked extA->openssl_func and extB->openssl_func:")
memtestlib.openssl_memaddr_cmp()

# test 9: Try to replace the function pointer in one native extension to
# point to a function in another extension
print("\n-- test 9: trying to replace a function in one extension with a function in another extension:")
memtestlib.replace_func_ptr()
