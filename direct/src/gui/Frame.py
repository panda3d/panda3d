from ShowBaseGlobal import *
import GuiManager
import GuiFrame
import Vec3

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node())
font = (loader.loadModelOnce("fonts/ttf-comic")).node()

class Frame:

    # special methods
    def __init__(self, name):
        self.name = name
        self.frame = GuiFrame.GuiFrame(name)
        self.items = []

    def __str__(self):
        return "Frame: %s = %s" % self.name, self.items

    # frame functions
    def getName(self):
        return self.name
    
    def manage(self):
        self.frame.manage(guiMgr, base.eventMgr.eventHandler)

    def setPos(Self, x, y):
        v3 = Vec3.Vec3(x, 0., y)
        self.frame.setPos(v3)

    def setScale(self, scale):
        self.frame.setScale(scale)

    # content functions
    def addItem(self, item):
        self.frame.addItem(item.getGuiItem())
        self.items.append(item)

    def getItems(self):
        return self.items

    def printItems(self):
        print "frame items: %s" % (self.items)
        
    def packItem(self, itemNum, relation, otherItemNum):
        self.frame.packItem(self.items[itemNum].getGuiItem(), relation,
                            self.items[otherItemNum].getGuiItem())

    def makeVetical(self):
        # make each item (except first) align under the last
        for itemNum in range(1, len(self.items)):
            self.packItem(itemNum, GuiFrame.GuiFrame.UNDER, itemNum - 1)

    def makeHorizontal(self):
        # make each item (except first) align right of the last
        for itemNum in range(1, len(self.items)):
            self.packItem(itemNum, GuiFrame.GuiFrame.RIGHT, itemNum - 1)
            
