from ShowBaseGlobal import *
import GuiManager
import GuiLabel
import GuiButton
import Vec3

#import ClockObject
#clock = ClockObject.ClockObject.getGlobalClock()

guiMgr = GuiManager.GuiManager.getPtr(base.win, base.mak.node())
font = (loader.loadModelOnce("phase_3/models/fonts/ttf-comic")).node()

class Button:

    def __init__(self, name, label=None):
        self.name = name
        # if no label given, use the button name
        if (label == None):
            self.label = name
        else:
            self.label = label
        # up
        self.l1 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label, font)
        #print "made the label: t = %.3f" % clock.getRealTime()
        self.l1.setForegroundColor(0., 0., 0., 1.)
        #print "set the label colors: t = %.3f" % clock.getRealTime()        
        # roll-over up
        self.l2 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label, font)
        #print "made the label: t = %.3f" % clock.getRealTime()
        self.l2.setForegroundColor(0., 0., 0., 1.)
        self.l2.setBackgroundColor(1., 1., 0., 1.)        
        #print "set the label colors: t = %.3f" % clock.getRealTime()
        # roll-over down
        self.l3 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label, font)
        #print "made the label: t = %.3f" % clock.getRealTime()
        self.l3.setForegroundColor(1., 1., 1., 1.)
        self.l3.setBackgroundColor(0., 0., 0., 1.)
        #print "set the label colors: t = %.3f" % clock.getRealTime()        
        self.button = GuiButton.GuiButton(self.name, self.l1, self.l2,
                                          self.l3, self.l3, self.l1)
        #print "made the button: t = %.3f" % clock.getRealTime()
        self.setScale(0.1)
        #print "scale button: t = %.3f" % clock.getRealTime()                
        self.managed = 0


    def __del__(self):
        if (self.managed):
            self.button.unmanage()
        del(self.l1)
        del(self.l2)
        del(self.button)
        
    def __str__(self):
        return "Button: %s" % self.name
    
    def getName(self):
        return self.name

    def getLabel(self):
        return self.label
    
    def getGuiItem(self):
        return self.button

    def getWidth(self):
        # assume all labels have the same width
        return self.l1.getWidth()
    
    def setWidth(self, width):
        self.l1.setWidth(width)
        self.l2.setWidth(width)
        self.l3.setWidth(width)
        
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
