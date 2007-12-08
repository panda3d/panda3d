from extension_native_helpers import *
Dtool_PreloadDLL("libpanda")
from libpanda import *

####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
OdeSpace-extensions module: contains methods to extend functionality
of the OdeSpace classe
"""

def convert(self):
    """
    Do a sort of pseudo-downcast on this space in 
    order to expose its specialized functions.
    """
    if self.getClass() == OdeGeom.GCSimpleSpace:
        return self.convertToSimpleSpace()
    elif self.getClass() == OdeGeom.GCHashSpace:
        return self.convertToHashSpace()
    elif self.getClass() == OdeGeom.GCQuadTreeSpace:
        return self.convertToQuadTreeSpace()
Dtool_funcToMethod(convert, OdeSpace)
del convert

def getConvertedGeom(self, index):
    """
    Return a downcast geom on this space.
    """
    return self.getGeom(index).convert()
Dtool_funcToMethod(getConvertedGeom, OdeSpace)
del getConvertedGeom

def getConvertedSpace(self):
    """
    """
    return self.getSpace().convert()
Dtool_funcToMethod(getConvertedSpace, OdeSpace)
del getConvertedSpace

def getAABounds(self):
    """
    A more Pythonic way of calling getAABB()
    """
    min = Point3()
    max = Point3()
    self.getAABB(min,max)
    return min,max
Dtool_funcToMethod(getAABounds, OdeSpace)
del getAABounds

