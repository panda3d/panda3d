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
        self.managed = 0
        self.frame = GuiFrame.GuiFrame(name)
        self.items = []

    def __del__(self):
        if (self.managed):
            self.frame.unmanage()
        del(self.frame)
        
    def __str__(self):
        return "Frame: %s = %s" % self.name, self.items

    # frame functions
    def getName(self):
        return self.name
    
    def manage(self):
        self.frame.manage(guiMgr, base.eventMgr.eventHandler)
        self.managed = 1
        
    def unmanage(self):
        self.frame.unmanage()
        self.unmanage = 0
        
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

    # convenience functions
    def makeVertical(self):
        # remove any previous packing
        #self.frame.clearAllPacking()
        # make each item (except first) align under the last
        for itemNum in range(1, len(self.items)):
            # self.frame.clearPacking(self.items[itemNum].getGuiItem())
            self.packItem(itemNum, GuiFrame.GuiFrame.UNDER, itemNum - 1)

    def makeHorizontal(self):
        # remove any previous packing
        #self.frame.clearAllPacking()        
        # make each item (except first) align right of the last
        for itemNum in range(1, len(self.items)):
            #self.frame.clearPacking(self.items[itemNum].getGuiItem())
            self.packItem(itemNum, GuiFrame.GuiFrame.RIGHT, itemNum - 1)
            
    def makeWideAsWidest(self):
        # make all the buttons as wide as the widest button in
        # the frame
        widest = 0
        widestWidth = 0.0
        # find the widest
        for item in self.items:
            thisWidth = item.getWidth()
            if (thisWidth > widestWidth):
                widest = self.items.index(item)
                widestWidth = thisWidth

        # make them all this wide
        for item in self.items:
            item.setWidth(widestWidth)

            
        
