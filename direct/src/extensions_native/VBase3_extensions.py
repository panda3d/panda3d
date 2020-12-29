"""
Methods to extend functionality of the VBase3 class
"""

from panda3d.core import VBase3
from .extension_native_helpers import Dtool_funcToMethod
import warnings

def pPrintValues(self):
    """
    Pretty print
    """
    return "% 10.4f, % 10.4f, % 10.4f" % (self[0], self[1], self[2])
Dtool_funcToMethod(pPrintValues, VBase3)
del pPrintValues

def asTuple(self):
    """
    Returns the vector as a tuple.
    """
    if __debug__:
        warnings.warn("VBase3.asTuple() is no longer needed and deprecated.  Use the vector directly instead.", DeprecationWarning, stacklevel=2)
    return tuple(self)
Dtool_funcToMethod(asTuple, VBase3)
del asTuple
