
from panda3d.core import *
from panda3d.direct import *
from direct.interval.IntervalGlobal import *
from direct.directnotify.DirectNotifyGlobal import directNotify

from direct.distributed.DistributedNode import DistributedNode
from direct.task import Task
from direct.gui import DirectGuiGlobals
from direct.showbase.EventGroup import EventGroup
from direct.showbase.PythonUtil import report
from direct.distributed.GridParent import GridParent

if __debug__:
    # For grid drawing
    from direct.directtools.DirectGeometry import *
    from direct.showbase.PythonUtil import randFloat

from .CartesianGridBase import CartesianGridBase

# increase this number if you want to visualize the grid lines
# above water level
GRID_Z_OFFSET = 0.0

class DistributedCartesianGrid(DistributedNode, CartesianGridBase):
    notify = directNotify.newCategory("DistributedCartesianGrid")
    notify.setDebug(0)

    VisualizeGrid = ConfigVariableBool("visualize-cartesian-grid", False)

    RuleSeparator = ":"

    def __init__(self, cr):
        DistributedNode.__init__(self, cr)
        # Let the derived classes instantiate the NodePath
        self.visAvatar = None
        self.gridVisContext = None
        # Do we have grid lines visualized?
        self._onOffState = False
        if __debug__:
            self.haveGridLines = 0

    def generate(self):
        DistributedNode.generate(self)

    def disable(self):
        DistributedNode.disable(self)
        self.stopProcessVisibility()

    def delete(self):
        DistributedNode.delete(self)
        # TODO: when teleporting off an island...
        taskMgr.remove(self.taskName("processVisibility"))

    def isGridParent(self):
        # If this distributed object is a DistributedGrid return 1.  0 by default
        return 1

    def setCellWidth(self, width):
        self.cellWidth = width

    def setParentingRules(self, style, rule):
        assert self.notify.debug("setParentingRules: style: %s, rule: %s" % (style, rule))
        rules = rule.split(self.RuleSeparator)
        assert len(rules) == 3
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

    def handleChildArrive(self, child, zoneId):
        DistributedNode.handleChildArrive(self, child, zoneId)
        if (zoneId >= self.startingZone):
            if not child.gridParent:
                child.gridParent = GridParent(child)
            child.gridParent.setGridParent(self, zoneId)
        elif child.gridParent:
            child.gridParent.delete()
            child.gridParent = None

    def handleChildArriveZone(self, child, zoneId):
        DistributedNode.handleChildArrive(self, child, zoneId)
        if (zoneId >= self.startingZone):
            if not child.gridParent:
                child.gridParent = GridParent(child)
            child.gridParent.setGridParent(self, zoneId)
        elif child.gridParent:
            child.gridParent.delete()
            child.gridParent = None

    def handleChildLeave(self, child, zoneId):
        if child.gridParent:
            child.gridParent.delete()
            child.gridParent = None

    @report(types = ['deltaStamp', 'avLocation', 'args'], dConfigParam = ['connector','shipboard'])
    def startProcessVisibility(self, avatar):
        if not self._onOffState:
            # if we've been told that we're OFF, don't try
            # to process visibilty
            return

        assert not self.cr._noNewInterests
        if self.cr.noNewInterests():
            self.notify.warning(
                'startProcessVisibility(%s): tried to open a new interest during logout'
                % self.doId)
            return
        taskMgr.remove(self.taskName("processVisibility"))
        self.acceptOnce(self.cr.StopVisibilityEvent, self.stopProcessVisibility)
        self.visAvatar = avatar
        self.visZone = None
        self.visDirty = True
        taskMgr.add(
            self.processVisibility, self.taskName("processVisibility"))
        self.processVisibility(0)

    @report(types = ['deltaStamp', 'avLocation', 'args'], dConfigParam = ['connector','shipboard'])
    def stopProcessVisibility(self, clearAll=False, event=None):
        self.ignore(self.cr.StopVisibilityEvent)
        taskMgr.remove(self.taskName("processVisibility"))
        if event is not None:
            eventGroup = EventGroup('DistCartesianGrid.stopProcessVis',
                                    doneEvent=event)
        if self.gridVisContext is not None:
            if event is not None:
                removeEvent = eventGroup.newEvent('%s.removeInterest' % self.doId)
            else:
                removeEvent = None
            self.cr.removeInterest(self.gridVisContext, removeEvent)
            self.gridVisContext = None
        else:
            # if we were given an event but we have not interest open,
            # just send the event right away
            if event is not None:
                messenger.send(event)
        self.visAvatar = None
        self.visZone = None

        # sometimes we also need to remove vis avatar from
        # my parent if it is also a grid
        if (clearAll):
            if event is not None:
                parentEvent = eventGroup.newEvent('%s.parent.removeInterest' % self.doId)
            else:
                parentEvent = None

            ##HACK BANDAID FOR PVP INSTANCES
            if(hasattr(self.cr.doId2do[self.parentId],"worldGrid")):
                self.cr.doId2do[self.parentId].worldGrid.stopProcessVisibility(event=parentEvent)

    def processVisibility(self, task):
        if self.visAvatar == None:
            # no avatar to process visibility for
            return Task.done
        if(self.visAvatar.isDisabled()):
            self.visAvatar = None
            return Task.done
        if self.visAvatar.gameFSM.state == 'Cutscene':
            return Task.cont

        pos = self.visAvatar.getPos(self)
        # Check to make sure our x and y are positive
        dx = self.cellWidth * self.gridSize * .5
        x = pos[0] + dx
        y = pos[1] + dx
        col = x // self.cellWidth
        row = y // self.cellWidth
        assert self.notify.debug(
            "processVisibility: %s: avatar pos: %s %s" % (self.doId, x, y))
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
            assert self.notify.debug(
                "processVisibility: %s: interest did not change" % (self.doId))
            if self.visDirty:
                messenger.send(self.uniqueName("visibility"))
                self.visDirty = False
            return Task.cont
        else:
            assert self.notify.debug(
                "processVisibility: %s: new interest" % (self.doId))
            self.visZone = zoneId
            if not self.gridVisContext:
                self.gridVisContext = self.cr.addInterest(
                    self.getDoId(), self.visZone,
                    self.uniqueName("visibility"),
                    event = self.uniqueName("visibility"))
            else:
                assert self.notify.debug(
                    "processVisibility: %s: altering interest to zoneId: %s" %
                    (self.doId, zoneId))

                event = None
                if self.visDirty:
                    event = self.uniqueName("visibility")
                self.cr.alterInterest(
                    self.gridVisContext, self.getDoId(), self.visZone,
                    event = event)

                # If the visAvatar is parented to this grid, also do a
                # setLocation
                parentId = self.visAvatar.parentId
                oldZoneId = self.visAvatar.zoneId
                assert self.notify.debug(
                    "processVisibility: %s: parentId: %s oldZoneId: %s" %
                    (self.doId, parentId, oldZoneId))
                if parentId == self.doId:
                    assert self.notify.debug(
                        "processVisibility: %s: changing location" %
                        (self.doId))
                    messenger.send("avatarZoneChanged", [self.visAvatar, self.doId, zoneId])
                    #self.handleAvatarZoneChange(self.visAvatar, zoneId)
            self.visDirty = False
            return Task.cont

    # Update our location based on our avatar's position on the grid
    # Assumes our position is correct, relative to the grid
    def addObjectToGrid(self, av):
        assert self.notify.debug("addObjectToGrid %s" % av)
        # Get our pos relative to the island grid
        pos = av.getPos(self)
        # Figure out what zone in that island grid
        zoneId = self.getZoneFromXYZ(pos)
        # Do the wrtReparenting to the grid node
        messenger.send("avatarZoneChanged", [av, self.doId, zoneId])
        #self.handleAvatarZoneChange(av, zoneId)

    def removeObjectFromGrid(self, av):
        assert self.notify.debug("removeObjectFromGrid %s" % av)
        # TODO: WHAT LOCATION SHOULD WE SET THIS TO?
        #av.reparentTo(hidden)
        if av.getParent() == self:
            # only detach if object is directly parented
            av.detachNode()
        #av.b_setLocation(0, 0)


    def handleAvatarZoneChange(self, av, zoneId):
        assert self.notify.debug("handleAvatarZoneChange(%s, %s)" % (av.doId, zoneId))
        # This method can be overridden by derived classes that
        # want to do some special management when the avatar changes
        # zones.
        # Make sure this is a valid zone
        if not self.isValidZone(zoneId):
            assert self.notify.warning("handleAvatarZoneChange: not a valid zone (%s)" % zoneId)
            return

        # Set the location on the server
        av.b_setLocation(self.doId, zoneId)

    def turnOff(self):
        self._onOffState = False
        self.stopProcessVisibility()

    def turnOn(self, av = None):
        self._onOffState = True
        if av:
            self.startProcessVisibility(av)

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
            self.centerLines.setColor(VBase4(1, 0, 0, 0))
            self.centerLines.setThickness(3)

            # Load up grid parts to initialize grid object
            # Polygon used to mark grid plane
            # self.gridBack = loader.loadModel('models/misc/gridBack')
            # self.gridBack.reparentTo(self)
            # self.gridBack.setColor(0.2, 0.2, 0.2, 0.5)

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
                    center.moveTo(icw, -scaledSize, GRID_Z_OFFSET)
                    center.drawTo(icw, scaledSize, GRID_Z_OFFSET)
                    center.moveTo(-scaledSize, icw, GRID_Z_OFFSET)
                    center.drawTo(scaledSize, icw, GRID_Z_OFFSET)
                else:
                    if (i % 5) == 0:
                        major.moveTo(icw, -scaledSize, GRID_Z_OFFSET)
                        major.drawTo(icw, scaledSize, GRID_Z_OFFSET)
                        major.moveTo(-scaledSize, icw, GRID_Z_OFFSET)
                        major.drawTo(scaledSize, icw, GRID_Z_OFFSET)
                    else:
                        minor.moveTo(icw, -scaledSize, GRID_Z_OFFSET)
                        minor.drawTo(icw, scaledSize, GRID_Z_OFFSET)
                        minor.moveTo(-scaledSize, icw, GRID_Z_OFFSET)
                        minor.drawTo(scaledSize, icw, GRID_Z_OFFSET)
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
                                                GRID_Z_OFFSET+3.0, # z
                                                # Lay them down flat
                                                0, -90, 0, # hpr
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
                    marker = loader.loadModel("models/misc/smiley")
                    marker.reparentTo(self.markerParent)
                    marker.setPos(i * self.cellWidth - dx,
                                  j * self.cellWidth - dx,
                                  GRID_Z_OFFSET + 1.0)
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

    def setWorldContext(self, worldContext):
        pass

    def clearWorldContext(self, event = None):
        pass
