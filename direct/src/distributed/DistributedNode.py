"""DistributedNode module: contains the DistributedNode class"""

from ShowBaseGlobal import *
import NodePath
import DistributedObject
import Correction
import Prediction
import Task

class DistributedNode(DistributedObject.DistributedObject, NodePath.NodePath):
    """Distributed Node class:"""

    def __init__(self, cr):
        try:
            self.DistributedNode_initialized
        except:
            self.DistributedNode_initialized = 1
            DistributedObject.DistributedObject.__init__(self, cr)
            self.DeadReckoningFlag = 0
        return None

    def disable(self):
        self.reparentTo(hidden)
        DistributedObject.DistributedObject.disable(self)

    def delete(self):
        try:
            self.DistributedNode_deleted
        except:
            self.DistributedNode_deleted = 1
            if not self.isEmpty():
                self.removeNode()
            DistributedObject.DistributedObject.delete(self)

    def generate(self):
        # Turn on dead reckoning
        #print ("Turning on dead reckoning!!!")
        #self.deadReckoningOn()
        DistributedObject.DistributedObject.generate(self)

    def deadReckoningOn(self):
        # Be sure that it isn't already on
        self.deadReckoningOff()
        
        self.deadReckoningFlag = 1

        self.Predictor = NullPrediction(Point3(self.getX(), self.getY(),
                                               self.getZ()))
        self.Corrector = SplineCorrection(Point3(self.getX(), self.getY(),
                                                 self.getZ()), Vec3(0))
        taskName = self.taskName("correctionPos")
        # remove any old tasks
        taskMgr.removeTasksNamed(taskName)
        # spawn new task
        task = Task.Task(self.correctPos)
        taskMgr.spawnTaskNamed(task, taskName)
        return self.deadReckoningFlag

    def deadReckoningOff(self):
        self.deadReckoningFlag = 0
        self.Predictor = None
        self.Corrector = None
        taskName = self.taskName("correctionPos")
        taskMgr.removeTasksNamed(taskName)
        return self.deadReckoningFlag

    ### setParent ###

    def b_setParent(self, parentString):
        self.setParent(parentString)
        self.d_setParent(parentString)
        return None

    def d_setParent(self, parentString):
        self.sendUpdate("setParent", [parentString])
        return None

    def setParent(self, parentString):
        # print "setting parent of %s to %s" % (self.getName(), parentString)
        assert(self.cr.name2nodePath.has_key(parentString))
        parent = self.cr.name2nodePath[parentString]
        self.wrtReparentTo(parent)
        return None

    ###### set pos and hpr functions #######

    # setX provided by NodePath
    def d_setX(self, x):
        self.sendUpdate("setX", [x])

    # setY provided by NodePath
    def d_setY(self, y):
        self.sendUpdate("setY", [y])
    
    # setZ provided by NodePath
    def d_setZ(self, z):
        self.sendUpdate("setZ", [z])
    
    # setH provided by NodePath
    def d_setH(self, h):
        self.sendUpdate("setH", [h])
    
    # setP provided by NodePath
    def d_setP(self, p):
        self.sendUpdate("setP", [p])
    
    # setR provided by NodePath
    def d_setR(self, r):
        self.sendUpdate("setR", [r])

    def setXY(self, x, y):
        self.setX(x)
        self.setY(y)
    def d_setXY(self, x, y):
        self.sendUpdate("setXY", [x, y])

    def setXZ(self, x, z):
        self.setX(x)
        self.setZ(z)
    def d_setXZ(self, x, z):
        self.sendUpdate("setXZ", [x, z])

    # setPos provided by NodePath
    def d_setPos(self, x, y, z):
        self.sendUpdate("setPos", [x, y, z])

    # setHpr provided by NodePath
    def d_setHpr(self, h, p, r):
        self.sendUpdate("setHpr", [h, p, r])

    def setXYH(self, x, y, h):
        self.setX(x)
        self.setY(y)
        self.setH(h)
    def d_setXYH(self, x, y, h):
        self.sendUpdate("setXYH", [x, y, h])

    def setXYZH(self, x, y, z, h):
        self.setPos(x, y, z)
        self.setH(h)
    def d_setXYZH(self, x, y, z, h):
        self.sendUpdate("setXYZH", [x, y, z, h])

    # setPosHpr provided by NodePath
    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])

    ###### Dead Reckoned set pos and hpr functions ######

    def setDRX(self, x):
        curPoint = self.Predictor.getPos()
        self.Predictor.newTelemetry(x, curPoint[1], curPoint[2])
    def d_setDRX(self, x):
        self.sendUpdate("setDRX", [x])

    def setDRY(self, y):
        curPoint = self.Predictor.getPos()
        self.Predictor.newTelemetry(curPoint[0], y, curPoint[2])
    def d_setDRY(self, y):
        self.sendUpdate("setDRY", [y])
    
    def setDRZ(self, z):
        curPoint = self.Predictor.getPos()
        self.Predictor.newTelemetry(curPoint[0], curPoint[1], z)
    def d_setDRZ(self, z):
        self.sendUpdate("setDRZ", [z])
    
    def setDRH(self, h):
        self.setH(h)
    def d_setDRH(self, h):
        self.sendUpdate("setDRH", [h])
    
    def setDRP(self, p):
        self.setP(p)
    def d_setDRP(self, p):
        self.sendUpdate("setDRP", [p])
    
    def setDRR(self, r):
        self.setR(r)
    def d_setDRR(self, r):
        self.sendUpdate("setDRR", [r])

    def setDRXY(self, x, y):
        curPoint = self.Predictor.getPos()
        self.Predictor.newTelemetry(Point3(x, y, curPoint[2]))
    def d_setDRXY(self, x, y):
        self.sendUpdate("setDRXY", [x, y])

    def setDRPos(self, x, y, z):
        self.Predictor.newTelemetry(Point3(x, y, z))
    def d_setDRPos(self, x, y, z):
        self.sendUpdate("setDRPos", [x, y, z])

    def setDRHpr(self, h, p, r):
        self.setHpr(h, p, r)
    def d_setDRHpr(self, h, p, r):
        self.sendUpdate("setDRHpr", [h, p, r])

    def setDRXYH(self, x, y, h):
        curPoint = self.Predictor.getPos()
        self.Predictor.newTelemetry(Point3(x, y, curPoint[2]))
        self.setH(h)
    def d_setDRXYH(self, x, y, h):
        self.sendUpdate("setDRXYH", [x, y, h])

    def setDRXYZH(self, x, y, z, h):
        self.Predictor.newTelemetry(Point3(x, y, z))
        self.setH(h)
    def d_setDRXYZH(self, x, y, z, h):
        self.sendUpdate("setDRXYZH", [x, y, z, h])

    def setDRPosHpr(self, x, y, z, h, p, r):
            self.Predictor.newTelemetry(Point3(x, y, z))
            NodePath.NodePath.setHpr(self, h, p, r)
    def d_setDRPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setDRPosHpr", [x, y, z, h, p, r])

    def correctPos(self, task):
        self.Corrector.newTarget(self.Predictor.getPos(),
                                 self.Predictor.getVel())
        self.Corrector.step()
        NodePath.NodePath.setPos(self, self.Corrector.getPos())
        return Task.cont

