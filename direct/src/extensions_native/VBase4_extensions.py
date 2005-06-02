"""
Methods to extend functionality of the VBase4 class
"""

from extension_native_helpers import *
from libpanda import *


def pPrintValues(self):
    """
    Pretty print
    """
    return "% 10.4f, % 10.4f, % 10.4f, % 10.4f" % (self[0],self[1],self[2],self[3])
Dtool_funcToMethod(pPrintValues, VBase4)
del pPrintValues
