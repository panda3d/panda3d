from PandaObject import *
from DirectGeometry import *
import Task

class PieMenu(NodePath, PandaObject):
    def __init__(self, visibleMenu, menuItems,
                 action = None, fUpdateOnlyOnChange = 1):
        NodePath.__init__(self)
        # Create a toplevel node for aspect ratio scaling
        self.assign(hidden.attachNewNode('PieMenu'))
        # Attach the menu
        self.visibleMenu = visibleMenu
        # Try to flatten the visibleMenu (note, flattenStrong is too strong
        # for texture text
        self.visibleMenu.flattenMedium()
        self.visibleMenu.reparentTo(self)
        # Initialize instance variables
        self.menuItems = menuItems
        self.numItems = len(self.menuItems)
        self.degreesPerItem = 360.0/self.numItems
        self.itemOffset = self.degreesPerItem / 2.0
        self.sfx = self.visibleMenu.getSx()
        self.sfz = self.visibleMenu.getSz()
        # Record target and action
        self.action = action
        self.initialState = None
        # Marking lines
        self.lines = LineNodePath(self.visibleMenu)
        self.lines.setColor(VBase4(1))
        self.lines.setThickness(1)
        # Set flags
        self.fUpdateOnlyOnChange = fUpdateOnlyOnChange

    def performAction(self, value):
        if self.action:
            self.action(value)

    def removePieMenuTask(self):
        taskMgr.remove('pieMenuTask')
        self.reparentTo(hidden)
        self.lines.reset()

    def spawnPieMenuTask(self):
        # Make sure no errant tasks lying around
        taskMgr.remove('pieMenuTask')

        # Where did the user press the button?
        self.originX = direct.dr.mouseX
        self.originY = direct.dr.mouseY

        # Pop up menu
        self.reparentTo(render2d)
        self.setPos(self.originX,0.0,self.originY)
        # Compensate for window aspect ratio
        self.setScale(1.0, 1.0,1.0)
        #direct.dr.width/float(direct.dr.height))
        # Start drawing the selection line
        self.lines.reset()
        self.lines.moveTo(0,0,0)
        self.lines.drawTo(0,0,0)
        self.lines.create()

        # Spawn task to update line and select new texture
        self.currItem = -1
        taskMgr.add(self.pieMenuTask, 'pieMenuTask')

    def pieMenuTask(self,state):
        mouseX = direct.dr.mouseX
        mouseY = direct.dr.mouseY
        deltaX = mouseX - self.originX
        deltaY = mouseY - self.originY

        # Update the line
        self.lines.setVertex(1,(deltaX/self.sfx),0.0,(deltaY/self.sfz))

        # How far from starting point has user moved the cursor?
        if ((abs(deltaX) < 0.1) and (abs(deltaY) < 0.1)):
            # In the center
            if self.fUpdateOnlyOnChange:
                # Only do this when things change
                if (self.currItem != -1):
                    self.performAction(self.initialState)
            else:
                # Alway let use know mouse is in the center
                self.performAction(self.initialState)
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
                    self.performAction(self.menuItems[newItem])
            else:
                self.performAction(self.menuItems[newItem])
            self.currItem = newItem
        # Continue task
        return Task.cont

    def setInitialState(self,state):
        self.initialState = state

    def getInitialState(self):
        return self.initialState

    def setItemOffset(self,newOffset):
        self.itemOffset = newOffset

    def setUpdateOnlyOnChange(self,flag):
        self.fUpdateOnlyOnChange = flag

