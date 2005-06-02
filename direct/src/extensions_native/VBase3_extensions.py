"""
Methods to extend functionality of the VBase3 class
"""

from extension_native_helpers import *
from libpanda import *


def pPrintValues(self):
    """
    Pretty print
    """
    return "% 10.4f, % 10.4f, % 10.4f" % (self[0],self[1],self[2])
Dtool_funcToMethod(pPrintValues, VBase3)
del pPrintValues
