from extension_native_helpers import *
Dtool_PreloadDLL("libpanda")
from libpanda import *

####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
OdeGeom-extensions module: contains methods to extend functionality
of the OdeGeom class
"""

def convert(self):
    """
    Do a sort of pseudo-downcast on this geom in 
    order to expose its specialized functions.
    """
    if self.getClass() == OdeGeom.GCSphere:
        return self.convertToSphere()
    elif self.getClass() == OdeGeom.GCBox:
        return self.convertToBox()
    elif self.getClass() == OdeGeom.GCCappedCylinder:
        return self.convertToCappedCylinder()
    elif self.getClass() == OdeGeom.GCPlane:
        return self.convertToPlane()
    elif self.getClass() == OdeGeom.GCRay:
        return self.convertToRay()
    # elif self.getClass() == OdeGeom.GCConvex:
    #     return self.convertToConvex()
    # elif self.getClass() == OdeGeom.GCGeomTransform:
    #     return self.convertToGeomTransform()
    elif self.getClass() == OdeGeom.GCTriMesh:
        return self.convertToTriMesh()
    # elif self.getClass() == OdeGeom.GCHeightfield:
    #     return self.convertToHeightfield()
    elif self.getClass() == OdeGeom.GCSimpleSpace:
        return self.convertToSimpleSpace()
    elif self.getClass() == OdeGeom.GCHashSpace:
        return self.convertToHashSpace()
    elif self.getClass() == OdeGeom.GCQuadTreeSpace:
        return self.convertToQuadTreeSpace()
Dtool_funcToMethod(convert, OdeGeom)
del convert

def getConvertedSpace(self):
    """
    """
    return self.getSpace().convert()
Dtool_funcToMethod(getConvertedSpace, OdeGeom)
del getConvertedSpace

def getAABounds(self):
    """
    A more Pythonic way of calling getAABB()
    """
    min = Point3()
    max = Point3()
    self.getAABB(min,max)
    return min,max
Dtool_funcToMethod(getAABounds, OdeGeom)
del getAABounds

