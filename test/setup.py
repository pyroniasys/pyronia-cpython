from distutils.core import setup, Extension

module1 = Extension('memtestlib_native',
                    sources = ['memlib_native.c'])

setup (name = 'MemoryInteractionProto',
       version = '1.0',
       description = 'Memory interaction tests from native lib prototype',
       ext_modules = [module1])
