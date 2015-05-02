###  Tools
__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod", "Dtool_PreloadDLL"]

import imp, sys, os

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
    module_ext = '.pyd'

    # We allow the caller to preload dll_suffix into the sys module.
    dll_suffix = getattr(sys, 'dll_suffix', None)

    if dll_suffix is None:
        # Otherwise, we try to determine it from the executable name:
        # python_d.exe implies _d across the board.
        dll_suffix = ''
        if sys.executable.endswith('_d.exe'):
            dll_suffix = '_d'
            
elif sys.platform == "darwin":
    # On OSX, the dynamic libraries usually end in .dylib, but
    # sometimes we need .so.
    try:
        from direct.extensions_native.extensions_darwin import dll_ext
    except ImportError:
        dll_ext = '.dylib'
    module_ext = '.so'
else:
    # On most other UNIX systems (including linux), .so is used.
    dll_ext = '.so'
    module_ext = '.so'

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
        raise ImportError(message)

    # And add that directory to the system path.
    path = os.environ["PATH"]
    if not path.startswith(target + ";"):
        os.environ["PATH"] = target + ";" + path

def Dtool_FindModule(module):
    # Finds a .pyd module on the Python path.
    filename = module.replace('.', os.path.sep) + module_ext
    for dir in sys.path:
        lib = os.path.join(dir, filename)
        if (os.path.exists(lib)):
            return lib

    return None

def Dtool_PreloadDLL(module):
    if module in sys.modules:
        return

    # First find it as a .pyd module on the Python path.
    if Dtool_FindModule(module):
        # OK, we should have no problem importing it as is.
        return

    # Nope, we'll need to search for a dynamic lib and preload it.
    # Search for the appropriate directory.
    target = None
    filename = module.replace('.', os.path.sep) + dll_suffix + dll_ext
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, filename)
        if (os.path.exists(lib)):
            target = dir
            break

    if target is None:
        message = "DLL loader cannot find %s." % (module)
        raise ImportError(message)

    # Now import the file explicitly.
    pathname = os.path.join(target, filename)
    imp.load_dynamic(module, pathname)    

# Nowadays, we can compile libpandaexpress with libpanda into a
# .pyd file called panda3d/core.pyd which can be imported without
# any difficulty.  Let's see if this is the case.

# In order to support things like py2exe that play games with the
# physical python files on disk, we can't entirely rely on
# Dtool_FindModule to find our panda3d.core module.  However, we
# should be able to import it.  To differentiate the old-style Panda
# build (with .dll's) from the new-style Panda build (with .pyd's), we
# first try to import panda3d.core directly; if it succeeds we're in a
# new-style build, and if it fails we must be in an old-style build.
try:
    from panda3d.core import *
except ImportError:
    Dtool_PreloadDLL("libpandaexpress")
    from libpandaexpress import *

def Dtool_ObjectToDict(cls, name, obj):
    cls.DtoolClassDict[name] = obj;

def Dtool_funcToMethod(func, cls, method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    if sys.version_info < (3, 0):
        func.im_class = cls
    func.im_func = func
    func.im_self = None
    if not method_name:
        method_name = func.__name__
    cls.DtoolClassDict[method_name] = func;
