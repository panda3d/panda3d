from extension_native_helpers import *
Dtool_PreloadDLL("libpanda")
from libpanda import *

#####################################################################

# For iterating over children
def asList(self):
    """Converts a NodePathCollection into a list"""
    print "Warning: NodePathCollection.asList() is no longer needed and deprecated.  Iterate on the collection directly instead."
    return list(self)
        
Dtool_funcToMethod(asList, NodePathCollection)        
del asList
#####################################################################3333      

def getTightBounds(self):
    from pandac.PandaModules import Point3
    
    if self.getNumPaths() == 0:
        return (Point3.Point3(0), Point3.Point3(0))
    v1, v2 = self.getPath(0).getTightBounds()
    for i in range(1, self.getNumPaths()):
        v1x, v2x = self.getPath(i).getTightBounds()
        v1 = Point3.Point3(min(v1[0], v1x[0]),
                           min(v1[1], v1x[1]),
                           min(v1[2], v1x[2]))
        v2 = Point3.Point3(max(v2[0], v2x[0]),
                           max(v2[1], v2x[1]),
                           max(v2[2], v2x[2]))
    return v1, v2
    
Dtool_funcToMethod(getTightBounds, NodePathCollection)        
del getTightBounds
#####################################################################3333      
