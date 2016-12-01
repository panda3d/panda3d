
from panda3d.core import *
from panda3d.direct import *

#
# GridParent.py
# Any object that can be parented to the ocean grid
# (or any grid whose size is too large to represent in 16 bits),
# should derive from GridParent.  Can be used on client and AI code.

# GridParent will put a node inbetween the object and the grid so
# that the object is broadcasting its position relative to the gridCell
# it lies in.

class GridParent:

    # this lets GridParents share CellOrigins
    GridZone2CellOrigin = {}
    GridZone2count = {}
    @staticmethod
    def getCellOrigin(grid, zoneId):
        tup = (grid, zoneId)
        if tup not in GridParent.GridZone2count:
            GridParent.GridZone2count[tup] = 0
            # For readability when debugging, append the zone to the name
            GridParent.GridZone2CellOrigin[tup] = grid.attachNewNode("cellOrigin-%s" % zoneId)
            # Get grid cell origin
            cellPos = grid.getZoneCellOrigin(zoneId)
            # Set the gridNode's position
            GridParent.GridZone2CellOrigin[tup].setPos(*cellPos)
        GridParent.GridZone2count[tup] += 1
        return GridParent.GridZone2CellOrigin[tup]
    @staticmethod
    def releaseCellOrigin(grid, zoneId):
        tup = (grid, zoneId)
        GridParent.GridZone2count[tup] -= 1
        if GridParent.GridZone2count[tup] == 0:
            del GridParent.GridZone2count[tup]
            GridParent.GridZone2CellOrigin[tup].removeNode()
            del GridParent.GridZone2CellOrigin[tup]

    def __init__(self, av):
        # The object on the grid will need to broadcast his position relative to
        # his current grid cell in order to use 16 bit
        # telemetry.  To do this, we will have a node attached to the
        # grid cell origin, and the object will wrtReparent himself to it when
        # crossing into that grid cell.  We don't need to create a node for each
        # cell origin.  We just need two nodes:  one that we are currently parented
        # to, and the other that we will wrtReparentTo.  Just before wrtReparenting
        # to the new node, set it's position to the new grid cell origin.
        self.av = av
        self.grid = None
        # NOTE: this node gets renamed when it is put on a zone, so if you
        # are looking for it by name, try cellOrigin*.
        self.ownCellOrigin = NodePath("cellOrigin")
        self.cellOrigin = self.ownCellOrigin

    def delete(self):
        if self.av:
            if self.av.getParent() == self.cellOrigin:
                self.av.detachNode()
            del self.av
            self.av = None
        # Remove the gridNodes
        if self.ownCellOrigin is not None:
            self.ownCellOrigin.removeNode()
            self.ownCellOrigin = None
        if self.grid is not None:
            self.releaseCellOrigin(self.grid, self.zoneId)
            self.grid = None
            self.zoneId = None

    def setGridParent(self, grid, zoneId, teleport=0):
        # If teleport=0, preserve the avatar's absolute position.  If teleport=1
        # the avatars previous world position is invalid, so don't wrtReparent,
        # just do a regular reparent, and let the cellOrigin give us our new position

        # Also, if the avatar has no parent, then force teleport=1
        if self.av.getParent().isEmpty():
            teleport = 1

        if not teleport:
            # Stick the avatar under hidden while we move the cellOrigin into
            # position so we do not lose the avatars absolute position.
            self.av.wrtReparentTo(hidden)

        if self.grid is not None:
            self.releaseCellOrigin(self.grid, self.zoneId)
        self.grid = grid
        self.zoneId = zoneId
        self.cellOrigin = self.getCellOrigin(self.grid, self.zoneId)

        # Reparent our avatar to this node
        if not teleport:
            self.av.wrtReparentTo(self.cellOrigin)
        else:
            self.av.reparentTo(self.cellOrigin)

        #print "gridParent: reparent to %s" % self.av
        #print "gridParent: pos = %s, %s" % (self.av.getPos(), self.av.getParent().getPos())


