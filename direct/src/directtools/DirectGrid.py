
from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from DirectUtil import *
from DirectGeometry import *

class DirectGrid(NodePath, DirectObject):
    def __init__(self,gridSize=100.0,gridSpacing=5.0,planeColor=(0.5,0.5,0.5,0.5),parent = None):
        # Initialize superclass
        NodePath.__init__(self, 'DirectGrid')
        # Don't wireframe or light
        useDirectRenderStyle(self)

        # Load up grid parts to initialize grid object
        # Polygon used to mark grid plane
        self.gridBack = loader.loadModel('models/misc/gridBack')
        self.gridBack.reparentTo(self)
        self.gridBack.setColor(*planeColor)

        # Grid Lines
        self.lines = self.attachNewNode('gridLines')
        self.minorLines = LineNodePath(self.lines)
        self.minorLines.lineNode.setName('minorLines')
        self.minorLines.setColor(VBase4(0.3, 0.55, 1, 1))
        self.minorLines.setThickness(1)

        self.majorLines = LineNodePath(self.lines)
        self.majorLines.lineNode.setName('majorLines')
        self.majorLines.setColor(VBase4(0.3, 0.55, 1, 1))
        self.majorLines.setThickness(5)

        self.centerLines = LineNodePath(self.lines)
        self.centerLines.lineNode.setName('centerLines')
        self.centerLines.setColor(VBase4(1, 0, 0, 0))
        self.centerLines.setThickness(3)

        # Small marker to hilight snap-to-grid point
        self.snapMarker = loader.loadModel('models/misc/sphere')
        self.snapMarker.node().setName('gridSnapMarker')
        self.snapMarker.reparentTo(self)
        self.snapMarker.setColor(1, 0, 0, 1)
        self.snapMarker.setScale(0.3)
        self.snapPos = Point3(0)

        # Initialize Grid characteristics
        self.fXyzSnap = 1
        self.fHprSnap = 1
        self.gridSize = gridSize
        self.gridSpacing = gridSpacing
        self.snapAngle = 15.0
        self.enable(parent = parent)

    def enable(self, parent = None):
        if parent:
            self.reparentTo(parent)
        else:
            self.reparentTo(base.direct.group)
            
        self.updateGrid()
        self.fEnabled = 1

    def disable(self):
        self.detachNode()
        self.fEnabled = 0

    def toggleGrid(self, parent = None):
        if self.fEnabled:
            self.disable()
        else:
            self.enable(parent = parent)

    def isEnabled(self):
        return self.fEnabled

    def updateGrid(self):
        # Update grid lines based upon current grid spacing and grid size
        # First reset existing grid lines
        self.minorLines.reset()
        self.majorLines.reset()
        self.centerLines.reset()

        # Now redraw lines
        numLines = int(math.ceil(self.gridSize/self.gridSpacing))
        scaledSize = numLines * self.gridSpacing

        center = self.centerLines
        minor = self.minorLines
        major = self.majorLines
        for i in range(-numLines, numLines + 1):
            if i == 0:
                center.moveTo(i * self.gridSpacing, -scaledSize, 0)
                center.drawTo(i * self.gridSpacing, scaledSize, 0)
                center.moveTo(-scaledSize, i * self.gridSpacing, 0)
                center.drawTo(scaledSize, i * self.gridSpacing, 0)
            else:
                if (i % 5) == 0:
                    major.moveTo(i * self.gridSpacing, -scaledSize, 0)
                    major.drawTo(i * self.gridSpacing, scaledSize, 0)
                    major.moveTo(-scaledSize, i * self.gridSpacing, 0)
                    major.drawTo(scaledSize, i * self.gridSpacing, 0)
                else:
                    minor.moveTo(i * self.gridSpacing, -scaledSize, 0)
                    minor.drawTo(i * self.gridSpacing, scaledSize, 0)
                    minor.moveTo(-scaledSize, i * self.gridSpacing, 0)
                    minor.drawTo(scaledSize, i * self.gridSpacing, 0)

        center.create()
        minor.create()
        major.create()
        if (self.gridBack):
            self.gridBack.setScale(scaledSize)

    def setXyzSnap(self, fSnap):
        self.fXyzSnap = fSnap

    def getXyzSnap(self):
        return self.fXyzSnap

    def setHprSnap(self, fSnap):
        self.fHprSnap = fSnap

    def getHprSnap(self):
        return self.fHprSnap

    def computeSnapPoint(self, point):
        # Start of with current point
        self.snapPos.assign(point)
        # Snap if necessary
        if self.fXyzSnap:
            self.snapPos.set(
                ROUND_TO(self.snapPos[0], self.gridSpacing),
                ROUND_TO(self.snapPos[1], self.gridSpacing),
                ROUND_TO(self.snapPos[2], self.gridSpacing))

        # Move snap marker to this point
        self.snapMarker.setPos(self.snapPos)

        # Return the hit point
        return self.snapPos

    def computeSnapAngle(self, angle):
        return ROUND_TO(angle, self.snapAngle)

    def setSnapAngle(self, angle):
        self.snapAngle = angle

    def getSnapAngle(self):
        return self.snapAngle

    def setGridSpacing(self, spacing):
        self.gridSpacing = spacing
        self.updateGrid()

    def getGridSpacing(self):
        return self.gridSpacing

    def setGridSize(self, size):
        # Set size of grid back and redraw lines
        self.gridSize = size
        self.updateGrid()

    def getGridSize(self):
        return self.gridSize
