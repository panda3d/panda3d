from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiBackground

class Background(DirectObject):

    def __init__(self, name, item, label):
        self.name = name
        self.item = item
        self.label = label
        self.background = GuiBackground.GuiBackground(name, self.item,
                                                      self.label)
        self.managed = 0
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
        if (self.managed):
            self.unmanage()
        self.background = None
	return None

    def __str__(self):
        return "Background: %s behind %s" % (self.name, self.item )
    
    # accessing
    def getName(self):
        return self.name

    def getGuiItem(self):
        return self.background

    def setPos(self, x, y):
        v3 = Vec3.Vec3(x, 0, y)
        self.background.setPos(v3)
        
    def setScale(self, scale):
        self.background.setScale(scale)

    # actions
    def manage(self):
        self.managed = 1
        self.background.manage(guiMgr, base.eventMgr.eventHandler)

    def unmanage(self):
        self.managed = 0
        self.background.unmanage()

    def getItem(self):
        return self.item
