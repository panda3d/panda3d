from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiCollection
import Vec3

class Collection(DirectObject):

    def __init__(self, name):
        self.name = name
        self.collection = GuiCollection.GuiCollection(self.name)
        self.items= []
        self.managed = 0
        return None

    def cleanup(self):
        """cleanup(self)
        """
        if (self.managed):
            self.unmanage()
        self.collection = None
        return None

    def __str__(self):
        return "Collection: %s = %s" % (self.name, self.items)
    
    # accessing
    def getName(self):
        return self.name

    def getGuiItem(self):
        return self.collection

    def setPos(self, x, y):
        v3 = Vec3.Vec3(x, 0, y)
        self.collection.setPos(v3)
        
    def setScale(self, scale):
        self.collection.setScale(scale)

    # actions
    def manage(self):
        self.managed = 1
        self.collection.manage(guiMgr, base.eventMgr.eventHandler)

    def unmanage(self):
        self.managed = 0
        self.collection.unmanage()

    def addItem(self, item):
        self.items.append(item)
        self.collection.addItem(item.getGuiItem())

    def removeItem(self, item):
        self.items.remove(item)
        self.collection.removeItem(item.getGuiItem())

    def getItems(self):
        return self.items
