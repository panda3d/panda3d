from ShowBaseGlobal import *
from DirectObject import *
import GuiChooser
import GuiManager
import GuiButton

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node(), base.render2d.node())

class Chooser(DirectObject):

    def __init__(self, name, prev, next):
        self.name = name
        self.prev = prev
        self.next = next
        self.chooser = GuiChooser.GuiChooser(self.name, self.prev, self.next)
        self.managed = 0
        return None

    def cleanup(self):
        """cleanup(self)
        """
        if (self.managed):
            self.unmanage()
        self.chooser = None
        return None

    def __str__(self):
        return "Chooser: %s" % self.name

    # accessing
    def getName(self):
        return self.name

    def getGuiItem(self):
        return self.chooser

    def setScale(self, scale):
        self.chooser.setScale(scale)

    def addItem(self, item):
        self.chooser.addItem(item)

    def getCurrItem(self):
        item = self.chooser.getCurrItem()
        if (item == -1):
            return None
        return item

    def setLoop(self, loop):
        self.chooser.setLoop(loop)

    # actions
    def manage(self):
        self.managed = 1
        self.chooser.manage(guiMgr, base.eventMgr.eventHandler)

    def unmanage(self):
        self.managed = 0
        self.chooser.unmanage()

    def freeze(self):
        self.chooser.freeze()

    def thaw(self):
        self.chooser.thaw()
