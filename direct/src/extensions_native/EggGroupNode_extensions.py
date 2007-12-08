from extension_native_helpers import *
Dtool_PreloadDLL("libpandaegg")
from libpandaegg import *

####################################################################
#Dtool_funcToMethod(func, class)        
#del func
#####################################################################
    # For iterating over children
def getChildren(self):
        """Returns a Python list of the egg node's children."""
        result = []
        child = self.getFirstChild()
        while (child != None):
            result.append(child)
            child = self.getNextChild()
        return result
    
Dtool_funcToMethod(getChildren, EggGroupNode)        
del getChildren
