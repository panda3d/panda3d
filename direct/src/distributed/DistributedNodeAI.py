from panda3d.core import NodePath
from . import DistributedObjectAI
from . import GridParent


class DistributedNodeAI(DistributedObjectAI.DistributedObjectAI, NodePath):
    def __init__(self, air, name=None):
        # Be careful not to create multiple NodePath objects
        try:
            self.DistributedNodeAI_initialized
        except:
            self.DistributedNodeAI_initialized = 1
            DistributedObjectAI.DistributedObjectAI.__init__(self, air)
            if name is None:
                name = self.__class__.__name__
            NodePath.__init__(self, name)
            self.gridParent = None

    def delete(self):
        if self.gridParent:
            self.gridParent.delete()
            self.gridParent = None
        if not self.isEmpty():
            self.removeNode()
        DistributedObjectAI.DistributedObjectAI.delete(self)

    def setLocation(self, parentId, zoneId, teleport=0):
        # Redefine DistributedObject setLocation, so that when
        # location is set to the ocean grid, we can update our parenting
        # under gridParent
        DistributedObjectAI.DistributedObjectAI.setLocation(self, parentId, zoneId)
        parentObj = self.air.doId2do.get(parentId)
        if parentObj:
            if parentObj.isGridParent():
                if not self.gridParent:
                    self.gridParent = GridParent.GridParent(self)
                self.gridParent.setGridParent(parentObj, zoneId)
            else:
                if self.gridParent:
                    self.gridParent.delete()
                    self.gridParent = None
                    # NOTE: at this point the avatar has been detached from the scene
                    # graph.  Someone else needs to reparent him to something in the scene graph
            # TODO: handle DistributedNode parenting


    ### setParent ###

    def b_setParent(self, parentToken):
        if type(parentToken) == str:
            self.setParentStr(parentToken)
        else:
            self.setParent(parentToken)
        self.d_setParent(parentToken)

    def d_setParent(self, parentToken):
        if type(parentToken) == type(''):
            self.sendUpdate("setParentStr", [parentToken])
        else:
            self.sendUpdate("setParent", [parentToken])

    def setParentStr(self, parentToken):
        self.notify.debug('setParentStr(%s): %s' % (self.doId, parentToken))
        if len(parentToken) > 0:
            self.do_setParent(parentToken)

    def setParent(self, parentToken):
        self.notify.debug('setParent(%s): %s' % (self.doId, parentToken))
        if parentToken == 0:
            senderId = self.air.getAvatarIdFromSender()
            self.air.writeServerEvent('suspicious', senderId, 'setParent(0)')
        else:
            self.do_setParent(parentToken)

    def do_setParent(self, parentToken):
        self.getParentMgr().requestReparent(self, parentToken)

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

    def b_setXYZH(self, x, y, z, h):
        self.setXYZH(x, y, z, h)
        self.d_setXYZH(x, y, z, h)
    def setXYZH(self, x, y, z, h):
        self.setPos(x, y, z)
        self.setH(h)
    def getXYZH(self):
        pos = self.getPos()
        h = self.getH()
        return pos[0], pos[1], pos[2], h

    def d_setXYZH(self, x, y, z, h):
        self.sendUpdate("setXYZH", [x, y, z, h])

    # setPosHpr provided by NodePath
    def b_setPosHpr(self, x, y, z, h, p, r):
        self.setPosHpr(x, y, z, h, p, r)
        self.d_setPosHpr(x, y, z, h, p, r)
    def d_setPosHpr(self, x, y, z, h, p, r):
        self.sendUpdate("setPosHpr", [x, y, z, h, p, r])
