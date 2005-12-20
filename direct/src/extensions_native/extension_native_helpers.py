###  Tools 
from libpandaexpress import *


def Dtool_ObjectToDict(clas,name,obj):
    clas.DtoolClassDict[name] = obj;            

def Dtool_funcToMethod(func,clas,method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    func.im_class=clas
    func.im_func=func
    func.im_self=None
    if not method_name: 
            method_name = func.__name__
    clas.DtoolClassDict[method_name] = func;            


