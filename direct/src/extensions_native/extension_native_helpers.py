###  Tools
__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod", "Dtool_PreloadDLL"]

import imp,sys,os

is_python_d = False
dll_suffix = ''
dll_ext = '.dll'
if (sys.platform == "darwin"):
    dll_ext = '.dylib'

if (sys.platform == "win32"):
    # If we launched from python_d.exe, we need to load libpanda_d.dll, etc.
    is_python_d = (sys.executable.endswith('_d.exe'))
    if is_python_d:
        dll_suffix = '_d'
    
    target = None
    filename = "libpandaexpress%s%s" % (dll_suffix, dll_ext)
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, filename)
        if (os.path.exists(lib)):
            target = dir
    if (target == None):
        message = "Cannot find %s" % (filename)
        raise message
    path=os.environ["PATH"]
    if (path.startswith(target+";")==0):
        os.environ["PATH"] = target+";"+path

def Dtool_PreloadDLL(module):
    """ Preloading solves the problem that python 2.5 on
    windows can't find DLLs - it can only find PYDs.  The
    preloader is able to find DLLs."""

    if (sys.platform != "win32" and sys.platform != "darwin"):
        return
    if (sys.modules.has_key(module)):
        return
    target = None
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, module + dll_suffix + dll_ext)
        if (os.path.exists(lib)):
            target = dir
            break
    if (target == None):
        raise "DLL loader cannot find "+module+"."
    imp.load_dynamic(module, os.path.join(target, module + dll_suffix + dll_ext))

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


