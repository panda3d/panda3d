from ToontownGlobals import *
import DirectNotifyGlobal
import BasicEntities

class PathEntity(BasicEntities.NodePathEntity):
    pathData = [
        [Vec3(10, 0, 0),
         Vec3(10, 10,0),
         Vec3(-10, 10, 0),
         Vec3(-10, 0, 0),
         ],
        [Vec3(10, 5, 0),
         Vec3(10, 0,0),
         Vec3(-10, -5, 0),
         ],
        [Vec3(-8.29501342773,-9.22601318359,0.0),
         Vec3(13.7590026855,-12.9730224609,0.0),
         Vec3(16.7769775391,10.7899780273,0.0),
         Vec3(-8.17102050781,14.1640014648,0.0),
         ],
        [Vec3(-47.9110107422,-6.86798095703,0.0),
         Vec3(27.691986084,-5.68200683594,0.0),
         Vec3(34.049987793,3.55303955078,0.0),
         Vec3(-39.983001709,3.68499755859,0.0)
         ],
        [Vec3(1.25,21,0),
         Vec3(-.2,7.9,0),
         Vec3(-22.2,-12.1,0),
         Vec3(-5.2,1.4,0),
         ],
        [Vec3(12.70, -51.9, 0.0),
         Vec3(12.4, -33.0, 0.0),
         Vec3(-1.16, -18.6, 0.0),
         Vec3(9.27, -34.3, 0.0),
         ],
        ]
    
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.path = self.pathData[self.pathIndex]
            
    def destroy(self):
        BasicEntities.NodePathEntity.destroy(self)

    def setPathIndex(self, pathIndex):
        self.pathIndex = pathIndex
        self.path = self.pathData[self.pathIndex]
    
    
        
