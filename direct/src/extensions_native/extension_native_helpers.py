__all__ = ["Dtool_ObjectToDict", "Dtool_funcToMethod"]


def Dtool_ObjectToDict(cls, name, obj):
    cls.DtoolClassDict[name] = obj


def Dtool_funcToMethod(func, cls, method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    func.__func__ = func
    func.__self__ = None
    if not method_name:
        method_name = func.__name__
    cls.DtoolClassDict[method_name] = func
