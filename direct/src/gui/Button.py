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
        self.l1 = GuiLabel.GuiLabel.makeSimpleTextLabel(name, font)
        self.l1.setForegroundColor(0., 0., 0., 1.)
        self.l1.setBackgroundColor(1., 1., 1., 1.)
        self.l2 = GuiLabel.GuiLabel.makeSimpleTextLabel(name, font)
        self.l2.setForegroundColor(1., 1., 1., 1.)
        self.l2.setBackgroundColor(0., 0., 0., 1.)
        self.button = GuiButton.GuiButton(name, self.l1, self.l1,
                                          self.l2, self.l2, self.l1)
        self.setScale(0.1)
        self.setPos(0., 0.)

    def __str__(self):
        return "Button: %s" % self.name
    
    def getName(self):
        return name
    
    def getGuiItem(self):
        return self.button
    
    def manage(self):
        self.button.manage(guiMgr, base.eventMgr.eventHandler)

    def setPos(self, x, y):
        v3 = Vec3.Vec3(x, 0., y)
        self.button.setPos(v3)

    def setScale(self, scale):
        self.button.setScale(scale)
