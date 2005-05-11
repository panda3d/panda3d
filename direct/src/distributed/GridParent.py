
from pandac.PandaModules import *

#
# GridParent.py
# Any object that can be parented to the ocean grid
# (or any grid whose size is too large to represent in 16 bits),
# should derive from GridParent.  Can be used on client and AI code.

# GridParent will put a node inbetween the object and the grid so
# that the object is broadcasting its position relative to the gridCell
# it lies in.

class GridParent:
    
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
        self.cellOrigin = NodePath("cellOrigin")

    def delete(self):
        if self.av:
            if self.av.getParent() == self.cellOrigin:
                self.av.detachNode()
            del self.av
            self.av = None
        # Remove the gridNodes
        self.cellOrigin.removeNode()
                
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

        if grid != self.grid:
            # Setup the grid for the first time
            # set/change the grid that we are working on.
            self.grid = grid
            # Reparent the gridNodes under this grid
            self.cellOrigin.reparentTo(grid)
            self.cellOrigin.setPosHpr(0,0,0,0,0,0)
        
        # Get grid cell origin
        cellPos = self.grid.getZoneCellOrigin(zoneId)

        # Set the gridNode's position
        self.cellOrigin.setPos(*cellPos)
        # For readability when debugging, append the zone to the name
        self.cellOrigin.setName("cellOrigin-%s" % (zoneId))

        # Reparent our avatar to this node
        if not teleport:
            self.av.wrtReparentTo(self.cellOrigin)
        else:
            self.av.reparentTo(self.cellOrigin)
            
        #print "gridParent: reparent to %s" % self.av
        #print "gridParent: pos = %s,%s" % (self.av.getPos(), self.av.getParent().getPos())

        
