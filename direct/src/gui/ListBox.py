from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import Button
import Label
import types


class ListBox(DirectObject):

    def __init__(self, name, numSlots,
                 scale = 0.1,
                 width = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont()):

        self.name = name
        self.numSlots = numSlots

        self.scale = scale
        self.width = width
        self.drawOrder = drawOrder
        self.font = font

        arrow = loader.loadModelOnce('phase_3/models/props/scroll-arrow')

        arrowScale = 0.1
        self.up = Button.Button(name + '-up', arrow,
                                geomRect = (-1, 1, 0, 0.5),
                                scale = arrowScale,
                                drawOrder = drawOrder,
                                downStyle = (0.5, 0, 0, 1),
                                supportInactive = 1)
        arrow.setR(180)
        self.down = Button.Button(name + '-down', arrow,
                                  geomRect = (-1, 1, -0.5, 0),
                                  scale = arrowScale,
                                  drawOrder = drawOrder,
                                  downStyle = (0.5, 0, 0, 1),
                                  supportInactive = 1)
        arrow.removeNode()

        self.listBox = GuiListBox(self.name + '-lb', self.numSlots,
                                  self.up.button, self.down.button)

        self.managed = 0

        self.items = []

	return None

    def addItem(self, item, label, event = None, param = None):
        if event:
            button = Button.Button(item, label, scale = self.scale,
                                   width = self.width,
                                   drawOrder = self.drawOrder,
                                   font = self.font, event = event)
            button.button.setBehaviorEventParameter(param)
            button.button.startBehavior()
        else:
            button = Button.Button(item, label,
                                   scale = self.scale,
                                   width = self.width,
                                   drawOrder = self.drawOrder,
                                   font = self.font)
        
        self.items.append((item, button))
        self.listBox.addItem(button.button)

    def addItems(self, items):
        for i in items:
            if isinstance(i, types.StringType):
                self.addItem(i, i)
            else:
                if (len(i) == 3):
                    self.addItem(i[0], i[1])
                else:
                    self.addItem(i[0], i[1], i[3], i[4])

    def cleanup(self):
        if (self.managed):
            self.unmanage()
        for i in self.items:
            i[1].cleanup()
        self.up.cleanup()
        self.down.cleanup()
        self.listBox = None
	return None
        
    def __str__(self):
        return "ListBox: %s" % (self.name)

    def getName(self):
        return self.name

    def getNumSlots(self):
        return self.numSlots
    
    def getGuiItem(self):
        return self.listBox

    def getUpButton(self):
        return self.up

    def getDownButton(self):
        return self.down

    def freeze(self):
        self.listBox.freeze()

    def thaw(self):
        self.listBox.thaw()
        
    def manage(self, nodepath = aspect2d):
        if nodepath:
            self.listBox.manage(guiMgr, base.eventMgr.eventHandler,
                                nodepath.node())
        else:
            self.listBox.manage(guiMgr, base.eventMgr.eventHandler)
        self.listBox.startBehavior()
        self.managed = 1

    def unmanage(self):
        self.listBox.unmanage()
        self.listBox.stopBehavior()
        self.managed = 0
    
    def setPos(self, x, y):
        self.listBox.setPos(Vec3(x, 0., y))
