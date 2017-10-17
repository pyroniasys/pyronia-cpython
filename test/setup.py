from distutils.core import setup, Extension

module1 = Extension('attacklib_native',
                    sources = ['attacklib_native.c'])

module2 = Extension('memtestlib_native',
                    sources = ['memlib_native.c'])

setup (name = 'FrameHackProto',
       version = '1.0',
       description = 'Frame hacking from native lib prototype',
       ext_modules = [module1, module2])
