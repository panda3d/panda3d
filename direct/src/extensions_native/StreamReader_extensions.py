from extension_native_helpers import *
Dtool_PreloadDLL("libpandaexpress")
from libpandaexpress import *

"""
StreamReader_extensions module: contains methods to extend functionality
of the StreamReader class
"""

def readlines(self):
    """Reads all the lines at once and returns a list."""
    lines = []
    line = self.readline()
    while line:
        lines.append(line)
        line = self.readline()
    return lines
    
Dtool_funcToMethod(readlines, StreamReader)        
del readlines
