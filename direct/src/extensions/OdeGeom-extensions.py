
def convert(self):
    """
    Do a sort of pseudo-downcast on this geom in 
    order to expose its specialized functions.
    """
    if self.getGeomClass() == OdeGeom.GCSphere:
        return self.convertToSphere()
    elif self.getGeomClass() == OdeGeom.GCBox:
        return self.convertToBox()
    elif self.getGeomClass() == OdeGeom.GCCappedCylinder:
        return self.convertToCappedCylinder()
    elif self.getGeomClass() == OdeGeom.GCPlane:
        return self.convertToPlane()
    elif self.getGeomClass() == OdeGeom.GCRay:
        return self.convertToRay()
    # elif self.getGeomClass() == OdeGeom.GCConvex:
    #     return self.convertToConvex()
    # elif self.getGeomClass() == OdeGeom.GCGeomTransform:
    #     return self.convertToGeomTransform()
    elif self.getGeomClass() == OdeGeom.GCTriMesh:
        return self.convertToTriMesh()
    # elif self.getGeomClass() == OdeGeom.GCHeightfield:
    #     return self.convertToHeightfield()
    elif self.getGeomClass() == OdeGeom.GCSimpleSpace:
        return self.convertToSimpleSpace()
    elif self.getGeomClass() == OdeGeom.GCHashSpace:
        return self.convertToHashSpace()
    elif self.getGeomClass() == OdeGeom.GCQuadTreeSpace:
        return self.convertToQuadTreeSpace()

def getConvertedSpace(self):
    """
    """
    return self.getSpace().convert()

def getAABounds(self):
    """
    A more Pythonic way of calling getAABB().
    """
    min = Point3()
    max = Point3()
    self.getAABB(min,max)
    return min,max