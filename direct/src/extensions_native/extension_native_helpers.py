###  Tools
__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod", "Dtool_PreloadDLL"]

import imp,sys,os

# The following code exists to work around a problem that exists
# with Python 2.5 or greater.

# Specifically, Python 2.5 is designed to import files named *.pyd
# only; it will not import files named *.dll (or *.so).  We work
# around this problem by explicitly preloading all of the dll's we
# expect to need.

dll_suffix = ''
if sys.platform == "win32":
    # On Windows, dynamic libraries end in ".dll".
    dll_ext = '.dll'

    # If we launched from python_d.exe, we need to load
    # libpanda_d.dll, etc.
    if sys.executable.endswith('_d.exe'):
        dll_suffix = '_d'
elif sys.platform == "darwin":
    # On OSX, the dynamic libraries usually end in .dylib, but
    # sometimes we need .so.
    try:
        from direct.extensions_native.extensions_darwin import dll_ext
    except ImportError:
        dll_ext = '.dylib'
else:
    # On most other UNIX systems (including linux), .so is used.
    dll_ext = '.so'

if sys.platform == "win32":
    # On Windows, we must furthermore ensure that the PATH is modified
    # to locate all of the DLL files.

    # First, search for the directory that contains all of our compiled
    # modules.
    target = None
    filename = "libpandaexpress%s%s" % (dll_suffix, dll_ext)
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, filename)
        if (os.path.exists(lib)):
            target = dir
    if target == None:
        message = "Cannot find %s" % (filename)
        raise ImportError, message

    # And add that directory to the system path.
    path = os.environ["PATH"]
    if not path.startswith(target + ";"):
        os.environ["PATH"] = target + ";" + path

def Dtool_PreloadDLL(module):
    if (sys.modules.has_key(module)):
        return

    # Search for the appropriate directory.
    target = None
    filename = module + dll_suffix + dll_ext
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, filename)
        if (os.path.exists(lib)):
            target = dir
            break
    if target == None:
        message = "DLL loader cannot find %s." % (module)
        raise ImportError, message

    # Now import the file explicitly.
    pathname = os.path.join(target, filename)
    imp.load_dynamic(module, pathname)

Dtool_PreloadDLL("libpandaexpress")
from libpandaexpress import *

def Dtool_ObjectToDict(clas, name, obj):
    clas.DtoolClassDict[name] = obj;

def Dtool_funcToMethod(func, clas, method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    func.im_class=clas
    func.im_func=func
    func.im_self=None
    if not method_name:
            method_name = func.__name__
    clas.DtoolClassDict[method_name] = func;


