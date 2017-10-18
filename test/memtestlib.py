import memtestlib_native

# test 1: trying to change the value of an immutable object in native
def add_4(x):
    y = 4
    memtestlib_native.pass_by_value(y)
    assert(y == 4)
    return x + y

# test 2: trying to change the value of a mutable object in native
def add_5(x):
    y = [5]
    memtestlib_native.pass_by_ref(y)
    assert(y[0] != 5)
    return x + y[0]

# test 3: accessing a native global variable
def add_12(x):
    print("Global variable from native: "+str(memtestlib_native.global_var))
    memtestlib_native.global_var = 11
    return x + memtestlib_native.global_var
