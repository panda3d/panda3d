from extension_native_helpers import *
from libpandaegg import *
####################################################################
#Dtool_funcToMethod(func, class)        
#del func
#####################################################################
    # For iterating over vertices
def getVertices(self):
        """Returns a Python list of the egg primitive's vertices."""
        result = []
        for i in range(self.getNumVertices()):
            result.append(self.getVertex(i))
        return result
Dtool_funcToMethod(getVertices, EggPrimitive)        
del getVertices
