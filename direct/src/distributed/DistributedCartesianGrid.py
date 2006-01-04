
from pandac.PandaModules import *

from direct.distributed import DistributedNode
from direct.task import Task
from direct.interval.IntervalGlobal import *
from direct.gui import DirectGuiGlobals

if __debug__:
    # For grid drawing
    from direct.directtools.DirectGeometry import *
    from direct.showbase.PythonUtil import randFloat

import CartesianGridBase


class DistributedCartesianGrid(DistributedNode.DistributedNode,
                               CartesianGridBase.CartesianGridBase):

    notify = directNotify.newCategory("DistributedCartesianGrid")
    notify.setDebug(0)

    VisualizeGrid = config.GetBool("visualize-cartesian-grid", 0)

    RuleSeparator = ":"

    def __init__(self, cr):
        DistributedNode.DistributedNode.__init__(self, cr)
        # Let the derived classes instantiate the NodePath
        self.visAvatar = None
        self.gridVisContext = None
        # Do we have grid lines visualized?
        if __debug__:
            self.haveGridLines = 0

    def generate(self):
        DistributedNode.DistributedNode.generate(self)
        
    def disable(self):
        DistributedNode.DistributedNode.disable(self)
        self.stopProcessVisibility()
        
    def delete(self):
        DistributedNode.DistributedNode.delete(self)
        # TODO: when teleporting off an island...
        taskMgr.remove(self.taskName("processVisibility"))

    def isGridParent(self):
        # If this distributed object is a DistributedGrid return 1.  0 by default
        return 1
        
    def setParentingRules(self, style, rule):
        assert self.notify.debug("setParentingRules: style: %s, rule: %s" % (style, rule))
        rules = rule.split(self.RuleSeparator)
        assert(len(rules) == 3)
        self.style = style
        self.startingZone = int(rules[0])
        self.gridSize = int(rules[1])
        self.viewingRadius = int(rules[2])

        # Store the center of the grid
        cx = self.cellWidth * self.gridSize/2.0
        self.centerPos = Vec3(cx, cx, 0)

        if __debug__:
            if self.VisualizeGrid:
                self.visualizeGrid()

    def getCenterPos(self):
        return self.centerPos

    def startProcessVisibility(self, avatar):
        self.stopProcessVisibility()
        self.visAvatar = avatar
        self.visZone = None
        self.gridVisContext = self.cr.addInterest(self.getDoId(), 0, self.uniqueName("visibility"))
        taskMgr.add(self.processVisibility, self.taskName("processVisibility"))

    def stopProcessVisibility(self,clearAll=False):
        taskMgr.remove(self.taskName("processVisibility"))
        if self.gridVisContext is not None:
            self.cr.removeInterest(self.gridVisContext)
            self.gridVisContext = None
        self.visAvatar = None
        self.visZone = None

        # sometimes we also need to remove vis avatar from
        # my parent if it is also a grid
        if (clearAll):
            self.cr.doId2do[self.parentId].worldGrid.stopProcessVisibility()

    def processVisibility(self, task):
        pos = self.visAvatar.getPos(self)
        # Check to make sure our x and y are positive
        dx = self.cellWidth * self.gridSize * .5
        x = pos[0] + dx
        y = pos[1] + dx
        col = x // self.cellWidth
        row = y // self.cellWidth
        assert self.notify.debug("processVisibility: %s: avatar pos: %s %s" % (self.doId, x,y))
        if (row < 0) or (col < 0) or (row > self.gridSize) or (col > self.gridSize):
            assert self.notify.debug("processVisibility: %s: not on the grid" % (self.doId))
            # If we are viewingRadius away from this entire grid,
            # remove interest in any current visZone we may have
            if self.gridVisContext:
                self.cr.removeInterest(self.gridVisContext)
                self.visZone = None
                self.gridVisContext = None
            return Task.cont
        # Compute which zone we are in
        zoneId = int(self.startingZone + ((row * self.gridSize) + col))
        assert self.notify.debug("processVisibility: %s: row: %s col: %s zoneId: %s" %
                                 (self.doId, row, col, zoneId))
        if (zoneId == self.visZone):
            assert self.notify.debug("processVisibility: %s: interest did not change" % (self.doId))
            return Task.cont
        else:
            assert self.notify.debug("processVisibility: %s: new interest" % (self.doId))
            self.visZone = zoneId
            if not self.gridVisContext:
                self.gridVisContext = self.cr.addInterest(self.getDoId(), self.visZone,
                                                      self.uniqueName("visibility"))
            else:
                assert self.notify.debug("processVisibility: %s: altering interest to zoneId: %s" %
                                         (self.doId, zoneId))
                self.cr.alterInterest(self.gridVisContext, self.getDoId(), self.visZone)
                # If the visAvatar is parented to this grid, also do a setLocation
                parentId = self.visAvatar.parentId
                oldZoneId = self.visAvatar.zoneId
                assert self.notify.debug(
                    "processVisibility: %s: parentId: %s oldZoneId: %s" %
                    (self.doId, parentId, oldZoneId))
                if parentId == self.doId:
                    assert self.notify.debug("processVisibility: %s: changing location" % (self.doId))
                    self.handleAvatarZoneChange(self.visAvatar, zoneId)
            return Task.cont

    # Take an avatar (or other object) from somewhere in the world and
    # wrtReparent him to the grid.
    def addObjectToGrid(self, av):
        self.notify.debug("addObjectToGrid %s" % av)
        # Get our pos relative to the island grid
        pos = av.getPos(self)
        # Figure out what zone in that island grid
        zoneId = self.getZoneFromXYZ(pos)
        # Do the wrtReparenting to the grid node
        self.handleAvatarZoneChange(av, zoneId)

    def removeObjectFromGrid(self, av):
        self.notify.debug("removeObjectFromGrid %s" % av)
        # TODO: WHAT LOCATION SHOULD WE SET THIS TO?
        #av.reparentTo(hidden)
        if (av.getParent().compareTo(self) == 0):
            # only detach if object is directly parented
            av.detachNode()
        #av.b_setLocation(0,0)

        
    def handleAvatarZoneChange(self, av, zoneId):
        assert self.notify.debug("handleAvatarZoneChange(%s, %s)" % (av.doId, zoneId))
        # This method can be overridden by derived classes that
        # want to do some special management when the avatar changes
        # zones.
        # Make sure this is a valid zone
        if not self.isValidZone(zoneId):
            self.notify.warning("handleAvatarZoneChange: not a valid zone (%s)" % zoneId)
            return
                
        # Set the location on the server
        av.b_setLocation(self.doId, zoneId)


    ##################################################
    # Visualization Tools
    ##################################################

    if __debug__:

        def initializeGridLines(self):
            # Grid Lines
            self.gridColor = VBase4(0.4 + randFloat(0.4),
                                    0.4 + randFloat(0.4),
                                    0.4 + randFloat(0.4),
                                    1)
            # A Dark version of the grid color
            color = self.gridColor * 0.5
            color.setW(1)
            
            self.lines = self.attachNewNode('gridLines')
            self.minorLines = LineNodePath(self.lines)
            self.minorLines.lineNode.setName('minorLines')
            self.minorLines.setColor(color)
            self.minorLines.setThickness(1)

            self.majorLines = LineNodePath(self.lines)
            self.majorLines.lineNode.setName('majorLines')
            self.majorLines.setColor(color)
            self.majorLines.setThickness(5)

            self.centerLines = LineNodePath(self.lines)
            self.centerLines.lineNode.setName('centerLines')
            self.centerLines.setColor(VBase4(1,0,0,0))
            self.centerLines.setThickness(3)

            # Load up grid parts to initialize grid object
            # Polygon used to mark grid plane
            # self.gridBack = loader.loadModel('models/misc/gridBack')
            # self.gridBack.reparentTo(self)
            # self.gridBack.setColor(0.2,0.2,0.2,0.5)

            self.cellLabelParent = None
            self.markerParent = None
            self.haveGridLines = 1
            
        def updateGrid(self):
            # Update grid lines based upon current grid spacing and grid size
            # First reset existing grid lines
            self.minorLines.reset()
            self.majorLines.reset()
            self.centerLines.reset()
            # Now redraw lines
            numLines = self.gridSize
            scaledSize = numLines * self.cellWidth / 2.0
            center = self.centerLines
            minor = self.minorLines
            major = self.majorLines
            cw = self.cellWidth
            dx = cw * self.gridSize * .5
            for i in range(numLines+1):
                icw = i * cw - dx
                if i == numLines/2:
                    center.moveTo(icw, -scaledSize, 0)
                    center.drawTo(icw, scaledSize, 0)
                    center.moveTo(-scaledSize, icw, 0)
                    center.drawTo(scaledSize, icw, 0)
                else:
                    if (i % 5) == 0:
                        major.moveTo(icw, -scaledSize, 0)
                        major.drawTo(icw, scaledSize, 0)
                        major.moveTo(-scaledSize, icw, 0)
                        major.drawTo(scaledSize, icw, 0)
                    else:
                        minor.moveTo(icw, -scaledSize, 0)
                        minor.drawTo(icw, scaledSize, 0)
                        minor.moveTo(-scaledSize, icw, 0)
                        minor.drawTo(scaledSize, icw, 0)
            center.create()
            minor.create()
            major.create()
            # self.gridBack.setScale(scaledSize)
            self.labelCells()

        def labelCells(self):
            if self.cellLabelParent:
                self.cellLabelParent.removeNode()
            self.cellLabelParent = self.attachNewNode('cellLabels')
            cw = self.cellWidth
            scale = cw / 10.0
            dx = cw * self.gridSize * .5
            font = DirectGuiGlobals.getDefaultFont()
            color = self.gridColor
            for i in range(self.gridSize):
                for j in range(self.gridSize):
                    zoneId = self.startingZone + ((j * self.gridSize) + i)
                    zoneStr = str(zoneId)
                    textNode = TextNode(zoneStr)
                    textNode.setText(zoneStr)
                    textNode.setFont(font)
                    textNode.setTextColor(color)
                    textNode.setAlign(TextNode.ACenter)
                    genTextNode = textNode.generate()
                    textNodePath = self.cellLabelParent.attachNewNode(genTextNode)
                    # Place the text node in the center of the cell
                    textNodePath.setPosHprScale((i * cw - dx) + (cw * 0.5), # x
                                                (j * cw - dx) + (cw * 0.5), # y
                                                3.0, # z
                                                # Lay them down flat
                                                0,-90,0, # hpr
                                                scale, scale, scale)
            self.cellLabelParent.flattenLight()

        def markCells(self):
            if self.markerParent:
                self.markerParent.removeNode()
            self.markerParent = self.attachNewNode('markers')
            self.cellMarkers = []
            dx = self.cellWidth * self.gridSize * .5
            for i in range(self.gridSize):
                for j in range(self.gridSize):
                    marker = loader.loadModelCopy("models/misc/smiley")
                    marker.reparentTo(self.markerParent)
                    marker.setPos(i * self.cellWidth - dx,
                                  j * self.cellWidth - dx,
                                  1.0)
                    marker.setScale(5)
                    self.cellMarkers.append(marker)

        def unmarkCells(self):
            if self.markerParent:
                self.markerParent.removeNode()
            self.markerParent = None

        def visualizeGrid(self):
            if not self.haveGridLines:
                self.initializeGridLines()
            self.updateGrid()
