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

        #arrow = loader.loadModelOnce('phase_3/models/props/scroll-arrow')
        arrow = None
        
        if arrow == None:
            self.up = Button.Button(name + '-up', '*',
                                    scale = self.scale,
                                    width = 2,
                                    drawOrder = self.drawOrder,
                                    font = self.font)
            self.down = Button.Button(name + '-down', '*',
                                      scale = self.scale,
                                      width = 2,
                                      drawOrder = self.drawOrder,
                                      font = self.font)
        else:
            arrowScale = 0.1
            self.up = Button.Button(name + '-up', arrow,
                                    left = -1, right = 1,
                                    bottom = 0, top = 0.5,
                                    scale = arrowScale,
                                    drawOrder = drawOrder)
            arrow.setR(180)
            self.down = Button.Button(name + '-down', arrow,
                                      left = -1, right = 1,
                                      bottom = -0.5, top = 0,
                                      scale = arrowScale,
                                      drawOrder = drawOrder)
            arrow.removeNode()


        self.listBox = GuiListBox(self.name, self.numSlots,
                                  self.up.button, self.down.button)

        self.managed = 0

        self.items = []

	return None

    def addItem(self, item, label):
        button = Button.Button(item, label,
                               scale = self.scale,
                               width = self.width,
                               drawOrder = self.drawOrder,
                               font = self.font)
        
        self.items.append(item)
        self.listBox.addItem(button.button)

    def addItems(self, items):
        for i in items:
            if isinstance(i, types.StringType):
                self.addItem(i, i)
            else:
                self.addItem(i[0], i[1])

    def cleanup(self):
        if (self.managed):
            self.listBox.unmanage()
        self.up = None
        self.down = None
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
