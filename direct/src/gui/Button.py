from ShowBaseGlobal import *
import GuiManager
import GuiLabel
import GuiButton
import Vec3

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node())
font = (loader.loadModelOnce("fonts/ttf-comic")).node()

class Button:

    def __init__(self, name):
        self.name = name
        self.managed = 0
        # up
        self.l1 = GuiLabel.GuiLabel.makeSimpleTextLabel(name, font)
        self.l1.setForegroundColor(0., 0., 0., 1.)
        self.l1.setBackgroundColor(1., 1., 1., 1.)
        # roll-over up
        self.l2 = GuiLabel.GuiLabel.makeSimpleTextLabel(name, font)
        self.l2.setForegroundColor(0., 0., 0., 1.)
        self.l2.setBackgroundColor(1., 1., 0., 1.)
        # roll-over down
        self.l3 = GuiLabel.GuiLabel.makeSimpleTextLabel(name, font)
        self.l3.setForegroundColor(1., 1., 1., 1.)
        self.l3.setBackgroundColor(0., 0., 0., 1.)
        self.button = GuiButton.GuiButton(name, self.l1, self.l2,
                                          self.l3, self.l3, self.l1)
        self.setScale(0.1)
        self.setPos(0., 0.)

    def __del__(self):
        if (self.managed):
            self.button.unmanage()
        del(self.button)
        del(self.l1)
        del(self.l2)
        
    def __str__(self):
        return "Button: %s" % self.name
    
    def getName(self):
        return self.name
    
    def getGuiItem(self):
        return self.button

    def getWidth(self):
        # assume all buttons have the same width
        return self.l1.getWidth()

    
    def setWidth(self, width):
        self.l1.setWidth(width / self.button.getScale())
        self.l2.setWidth(width / self.button.getScale())
        self.l3.setWidth(width / self.button.getScale())
        
    def manage(self):
        self.button.manage(guiMgr, base.eventMgr.eventHandler)
        self.managed = 1

    def unmanage(self):
        self.button.unmanage()
        self.managed = 0
        
    def setPos(self, x, y):
        v3 = Vec3.Vec3(x, 0., y)
        self.button.setPos(v3)

    def setScale(self, scale):
        self.button.setScale(scale)
