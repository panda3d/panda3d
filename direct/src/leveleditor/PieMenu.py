
from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.directtools.DirectGeometry import *
from direct.task import Task

class PieMenu(NodePath, DirectObject):
    def __init__(self, visibleMenu, menuItems,
                 action = None, fUpdateOnlyOnChange = 1, selectedSF = 1):
        NodePath.__init__(self)
        # Create a toplevel node for aspect ratio scaling
        self.assign(hidden.attachNewNode('PieMenu'))
        self.dr = base.direct.dr
        # Attach the menu
        self.visibleMenu = visibleMenu
        # Try to flatten the visibleMenu (note, flattenStrong is too strong
        # for texture text
        self.visibleMenu.flattenMedium()
        self.visibleMenu.reparentTo(self)
        # Initialize instance variables
        self.menuItems = menuItems
        self.numItems = len(self.menuItems)
        if self.numItems == 0:
            self.degreesPerItem = 360.0
        else:
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
        self.originX = self.dr.mouseX
        self.originY = self.dr.mouseY

        # Pop up menu
        self.reparentTo(render2d)
        self.setPos(self.originX, 0.0, self.originY)
        # Compensate for window aspect ratio
        self.setScale(1.0, 1.0, 1.0)
        # Start drawing the selection line
        self.lines.reset()
        self.lines.moveTo(0, 0, 0)
        self.lines.drawTo(0, 0, 0)
        self.lines.create()

        # Spawn task to update line and select new texture
        self.currItem = -1
        taskMgr.add(self.pieMenuTask, 'pieMenuTask')

    def pieMenuTask(self,state):
        # Don't do anything if nothing in the menu
        if self.numItems == 0:
            self.currItem = -1
            return Task.cont
        
        mouseX = self.dr.mouseX
        mouseY = self.dr.mouseY
        deltaX = mouseX - self.originX
        deltaY = mouseY - self.originY

        # Update the line
        #self.lines.setVertex(1, (deltaX/self.sfx), 0.0, (deltaY/self.sfz))

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

    def setInitialState(self, state):
        self.initialState = state

    def getInitialState(self):
        return self.initialState

    def setItemOffset(self, newOffset):
        self.itemOffset = newOffset

    def setUpdateOnlyOnChange(self, flag):
        self.fUpdateOnlyOnChange = flag


class TextPieMenu(PieMenu):
    def __init__(self, textList, radius = 0.5, sf = 1,
                 action = None, fUpdateOnlyOnChange = 1):
        numItems = len(textList)
        # Create top level node for new menu
        newMenu = hidden.attachNewNode('TextMenu')
        # Compute angle per item
        angle = deg2Rad(360.0/numItems)
        prop = base.win.getProperties()
        if prop.hasSize():
            width = prop.getXSize()
            height = prop.getYSize()
        else:
            width = 640
            height = 480
        aspectRatio = width/float(height)
        # Add items
        from direct.gui.DirectGuiGlobals import getDefaultFont
        for i in range (numItems):
            # Create text node for each item
            if (textList[i] != None):
                tn = TextNode('TextItem')
                tn.setFont(getDefaultFont())
                tn.setTransform(Mat4.scaleMat(0.07, 0.07, 0.07 * aspectRatio))
                tn.setTextColor(0, 0, 0, 1)
                tn.setCardColor(1, 1, 1, 1)
                tn.setCardAsMargin(0.1, 0.1, 0.1, 0.1)
                tn.setText(str(textList[i]))

                # Reposition it
                card = tn.getCardTransformed()
                center = (card[1] - card[0], card[3] - card[2])

                node = newMenu.attachNewNode(tn)
                node.setScale(sf)
                node.setPos(radius * math.cos(i * angle) - center[0], 0.0,
                            ((radius * aspectRatio * math.sin(i * angle)) -
                            center[1]))
        # Create and return a pie menu
        PieMenu.__init__(self, newMenu, textList, action = action,
                         fUpdateOnlyOnChange = fUpdateOnlyOnChange)
        self.accept('mouse3', self.spawnPieMenuTask)
        self.accept('mouse3-up', self.removePieMenuTask)
    def destroy(self):
        self.ignore('mouse3')
        self.ignore('mouse3-up')
        self.removeNode()



