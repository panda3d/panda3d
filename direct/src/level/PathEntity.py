from ToontownGlobals import *
from IntervalGlobal import *
import DirectNotifyGlobal
import BasicEntities
import GoonPathData

class PathEntity(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.path = GoonPathData.Paths[self.level.factoryId][self.pathIndex]
            
    def destroy(self):
        BasicEntities.NodePathEntity.destroy(self)

    def setPathIndex(self, pathIndex):
        self.pathIndex = pathIndex
        self.path = GoonPathData.Paths[self.level.factoryId][self.pathIndex]
    
    def makePathTrack(self, node, velocity, name, turnTime=1, lookAroundNode=None):
        track = Sequence(name = name)
        assert (len(self.path) > 1)

        # end with the starting point at the end, so we have a continuous loop
        path = self.path + [self.path[0]]
        for pointIndex in range(len(path) - 1):
            startPoint = path[pointIndex]
            endPoint = path[pointIndex + 1]
            # Face the endpoint
            v = startPoint - endPoint

            # figure out the angle we have to turn to look at the next point
            # Note: this will only look right for paths that are defined in a
            # counterclockwise order.  Otherwise the goon will always turn the
            # "long" way to look at the next point
            node.setPos(startPoint[0], startPoint[1],startPoint[2])
            node.headsUp(endPoint[0], endPoint[1], endPoint[2])
            theta = node.getH() % 360
                              
            track.append(
                LerpHprInterval(node, # stop and look around
                                turnTime,
                                Vec3(theta,0,0)))
            
            # Calculate the amount of time we should spend walking
            distance = Vec3(v).length()
            duration = distance / velocity
            
            # Walk to the end point
            track.append(
                LerpPosInterval(node, duration=duration,
                                pos=Point3(endPoint),
                                startPos=Point3(startPoint)))
        return track

    
        
        
