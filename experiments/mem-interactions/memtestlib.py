import memtestlib_native
import native

# test 1: trying to change the value of an immutable object in native
def add_4(x):
    y = 4
    print("Original param value: "+str(y))
    memtestlib_native.pass_by_value(y)
    assert(y == 4)
    print("New param value: "+str(y))
    return x + y

# test 2: trying to change the value of a mutable object in native
def add_5(x):
    y = [5]
    print("Original param value: ["+str(y[0])+"]")
    memtestlib_native.pass_by_ref(y)
    assert(y[0] != 5)
    print("New param value: ["+str(y[0])+"]")
    return x + y[0]

# test 3: accessing a native global variable
def add_12(x):
    print("Global variable from native: "+str(memtestlib_native.global_var))
    memtestlib_native.global_var = 11
    print("New global variable value: "+str(memtestlib_native.global_var))
    return x + memtestlib_native.global_var

# test 4: function callback from native into Python
def sum(x, func):
    memtestlib_native.set_callback(func)
    print("The sum from 1 to "+str(x)+" is: ")
    res = 0
    for i in range(1, x+1):
        res += i
    memtestlib_native.make_callback(res)

# test 5: Long jump from native library back into Python (uses callback
# mechanism to return to Python)
def prettify(txt, func):
    memtestlib_native.do_jmp(txt, func)

# test 6: Compare the memory address of a first-party native extension's
# pointer to a function in a second first-party extension to the second
# extension's pointer to the same function
def fp_ext_memaddr_cmp():
    fp_libA = native.ext_func
    hello_ptr = id(native.hello)
    fp_libB = memtestlib_native.ext_func

    if fp_libA != hello_ptr:
        print("Different mem addresses for the function pointer and corresponding python function object: ["+str(fp_libA)+"], ["+str(hello_ptr)+"]")

    if fp_libA != fp_libB:
        print("Different mem addresses: memtestlib_native.PyNative_Hello["+str(fp_libB)+"], native.PyNative_Hello["+str(fp_libA)+"]")
    else:
        print("Same mem address native.PyNative_Hello and memtestlib_native->native.PyNative_Hello: ["+str(fp_libA)+"]")

# test 7: Compare the memory address of a native extension's
# pointer to a libC function to a second extension's pointer to the same
# libC function
def libc_memaddr_cmp():
    fp_libA = native.libc_func
    fp_libB = memtestlib_native.libc_func

    if fp_libA != fp_libB:
        print("Different mem addresses: native->fopen["+str(fp_libA)+"], memtestlib_native->fopen["+str(fp_libB)+"]")
    else:
        print("Same mem address for native->fopen and memtestlib_native->fopen: ["+str(fp_libA)+"]")

# test 8: Compare the memory address of a native extension's
# pointer to an openssl function to a second extension's pointer to the same
# openssl function. In this case, libssl is dynamically linked into each extension.
def openssl_memaddr_cmp():
    fp_libA = native.openssl_func
    fp_libB = memtestlib_native.openssl_func

    if fp_libA != fp_libB:
        print("Different mem addresses: native->RAND_bytes["+str(fp_libA)+"], memtestlib_native->RAND_bytes["+str(fp_libB)+"]")
    else:
        print("Same mem address for native->RAND_bytes and memtestlib_native->RAND_bytes: ["+str(fp_libA)+"]")

# test 9: Try to replace a function pointer in native within memtestlib_native
def replace_func_ptr():
    native.hello()
    memtestlib_native.replace_func()
    native.hello()
