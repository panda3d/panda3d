####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
Mat3-extensions module: contains methods to extend functionality
of the LMatrix3f class.
"""

from panda3d.core import Mat3
from .extension_native_helpers import Dtool_funcToMethod

def pPrintValues(self):
        """
        Pretty print
        """
        return "\n%s\n%s\n%s" % (
            self.getRow(0).pPrintValues(), self.getRow(1).pPrintValues(), self.getRow(2).pPrintValues())
Dtool_funcToMethod(pPrintValues, Mat3)
del pPrintValues
#####################################################################
