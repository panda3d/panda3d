###  Tools
__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod"]

import sys,os

# Make sure the panda DLL directory is first on the path.
if (sys.platform == "win32"):
    target = None
    for dir in sys.path:
        lib = os.path.join(dir,"libpandaexpress.dll")
        if (os.path.exists(lib)):
            target = dir
    if (target == None):
        print "Cannot find libpandaexpress. Exiting."
        sys.exit(1)
    path=os.environ["PATH"]
    if (path.startswith(target+";")==0):
        os.environ["PATH"] = target+";"+path

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


