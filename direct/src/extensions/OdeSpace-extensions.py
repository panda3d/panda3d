
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

def getConvertedGeom(self, index):
    """
    Return a downcast geom on this body.
    """
    return self.getGeom(index).convert()

def getConvertedSpace(self):
    """
    """
    return self.getSpace().convert()

def getAABounds(self):
    """
    A more Pythonic way of calling getAABB()
    """
    min = Point3()
    max = Point3()
    self.getAABB(min,max)
    return min,max