###  Tools
__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod", "Dtool_PreloadDLL"]

import imp,sys,os

if (sys.platform == "win32"):
    target = None
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, "libpandaexpress.dll")
        if (os.path.exists(lib)):
            target = dir
    if (target == None):
        raise "Cannot find libpandaexpress.dll"
    path=os.environ["PATH"]
    if (path.startswith(target+";")==0):
        os.environ["PATH"] = target+";"+path

def Dtool_PreloadDLL(module):
    """ Preloading solves the problem that python 2.5 on
    windows can't find DLLs - it can only find PYDs.  The
    preloader is able to find DLLs."""

    if (sys.platform != "win32"):
        return
    if (sys.modules.has_key(module)):
        return
    target = None
    for dir in sys.path + [sys.prefix]:
        lib = os.path.join(dir, module + ".dll")
        if (os.path.exists(lib)):
            target = dir
            break
    if (target == None):
        raise "DLL loader cannot find "+module+"."
    imp.load_dynamic(module, os.path.join(target, module + ".dll"))

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


