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
# extensions pointer to the same function
def memaddr_cmp():
    fp_libB = memtestlib_native.dep_func
    fp_libA = id(native.hello)
    if fp_libA != fp_libB:
        print("Different mem addresses for fp_libA.func and fp_libB->fp_libA.func: memtestlib_native.PyNative_Hello["+str(fp_libA)+"], native.PyNative_Hello["+str(fp_libB)+"]")
    else:
        print("Same mem address for fp_libA.func and fp_libB->fp_libA.func: ["+str(fp_libA)+"]")
