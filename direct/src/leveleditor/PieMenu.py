from PandaObject import *
from DirectGeometry import *

class PieMenu(NodePath, PandaObject):
    def __init__(self, direct, menu, action = None, fUpdateOnlyOnChange = 1):
        NodePath.__init__(self)
        # Create a toplevel node for aspect ratio scaling
        self.assign(hidden.attachNewNode(NamedNode('PieMenu')))
        # Attach the menu
        self.menu = menu
        self.menu.reparentTo(self)
        # Initialize instance variables
        self.direct = direct
        self.numItems = self.menu.getNumChildren()
        self.degreesPerItem = 360.0/self.numItems
        self.itemOffset = self.degreesPerItem / 2.0
        self.sfx = self.menu.getSx()
        self.sfz = self.menu.getSz()
        # Record target and action
        self.action = action
        self.initialState = None
        # Marking lines
        self.lines = LineNodePath(self.menu)
        self.lines.setColor(VBase4(1))
        self.lines.setThickness(1)
        # Set flags
        self.fUpdateOnlyOnChange = fUpdateOnlyOnChange

    def performAction(self, value):
        if self.action:
            self.action(value)

    def removePieMenuTask(self):
        taskMgr.removeTasksNamed('pieMenuTask')
	self.reparentTo(hidden)
	self.lines.reset()

    def spawnPieMenuTask(self):
	# Make sure no errant tasks lying around
	taskMgr.removeTasksNamed('pieMenuTask')

	# Where did the user press the button?
	self.originX = self.direct.chan.mouseX
	self.originY = self.direct.chan.mouseY

	# Pop up menu
	self.reparentTo(render2d)
	self.setPos(self.originX,0.0,self.originY)
        # Compensate for window aspect ratio
        self.setScale(1.0, 1.0,1.0)
        #self.direct.chan.width/float(self.direct.chan.height))
	# Start drawing the selection line
	self.lines.reset()
	self.lines.moveTo(0,0,0)
	self.lines.drawTo(0,0,0)
	self.lines.create()

	# Spawn task to update line and select new texture
        self.currItem = -1
        t = Task.Task(self.pieMenuTask)
        taskMgr.spawnTaskNamed(t, 'pieMenuTask')

    def pieMenuTask(self,state):
        mouseX = self.direct.chan.mouseX
        mouseY = self.direct.chan.mouseY
        deltaX = mouseX - self.originX
        deltaY = mouseY - self.originY

        # Update the line
        self.lines.setVertex(1,(deltaX/self.sfx),0.0,(deltaY/self.sfz))

        # How far from starting point has user moved the cursor?
        if ((abs(deltaX) < 0.1) & (abs(deltaY) < 0.1)):
            # In the center
            if self.fUpdateOnlyOnChange:
                # Only do this when things change
                if (self.currItem != -1):
                    self.performAction(-1)
            else:
                # Alway let use know mouse is in the center
                self.performAction(-1)
            self.currItem = -1
        else:
            # Outside of the center
            # Interacting with menu
            # subtract half a slice to effectively center item
            menuAngle = rad2Deg(math.atan2(deltaY, deltaX)) + self.itemOffset
            if menuAngle < 0.0:
                menuAngle = menuAngle + 360.0
            menuAngle = menuAngle % 360.0
            newItem = int(math.floor(menuAngle / self.degreesPerItem))

            if self.fUpdateOnlyOnChange:
                if (self.currItem != newItem):
                    self.performAction(newItem)
            else:
                self.performAction(newItem)
            self.currItem = newItem
        # Continue task
        return Task.cont

    def setInitialState(self,state):
	self.initialState = state

    def getInitialState(self):
        return self.initialState

    def setItemOffset(self,newOffset):
	self.itemOffset = newOffset

    def setNumItems(self,num):
	self.numItems = num
	self.degreesPerItem = 360.0 / self.numItems
	self.itemOffset = self.degreesPerItem / 2.0

    def setUpdateOnlyOnChange(self,flag):
	self.fUpdateOnlyOnChange = flag

