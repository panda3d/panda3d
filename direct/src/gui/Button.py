from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiLabel
import GuiButton


class Button(DirectObject):

    def __init__(self, name, label=None, font=getDefaultFont()):
        self.name = name
        # if no label given, use the button name
        if (label == None):
            label = self.name

        # check to see if this is an actual guiLabel or just text
        if (type(label) == type('')):
            # text label, make text button
            self.label = label
            # up
            self.l1 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label,
                                                            font)
            self.l1.setForegroundColor(0., 0., 0., 1.)
            self.l1.thaw()
            # roll-over up
            self.l2 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label,
                                                            font)
            self.l2.setForegroundColor(0., 0., 0., 1.)
            self.l2.setBackgroundColor(1., 1., 0., 1.)         
            self.l2.thaw()
            # roll-over down
            self.l3 = GuiLabel.GuiLabel.makeSimpleTextLabel(self.label,
                                                            font)
            self.l3.setForegroundColor(1., 1., 1., 1.)
            self.l3.setBackgroundColor(0., 0., 0., 1.)
            self.l3.thaw()
        else:
            # label provided, use it for all labels
            self.l1 = self.l2 = self.l3 = label

        self.button = GuiButton.GuiButton(self.name, self.l1, self.l2,
                                          self.l3, self.l3, self.l1)

        self.setScale(0.1)
        self.managed = 0

	return None

    def cleanup(self):
        if (self.managed):
            self.button.unmanage()
        self.l1 = None
        self.l2 = None
        self.button = None
	return None
        
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

    def freeze(self):
        self.l1.freeze()
        self.l2.freeze()
        self.l3.freeze()
        self.button.freeze()

    def thaw(self):
        self.l1.thaw()
        self.l2.thaw()
        self.l3.thaw()
        self.button.thaw()
        
    def manage(self, nodepath = aspect2d):
        if nodepath:
            self.button.manage(guiMgr, base.eventMgr.eventHandler,
                               nodepath.node())
        else:
            self.button.manage(guiMgr, base.eventMgr.eventHandler)
        self.managed = 1

    def unmanage(self):
        self.button.unmanage()
        self.managed = 0
        
    def setPos(self, x, y, node = None):
        if node == None:
            v3 = Vec3(x, 0., y)
        else:
            mat = node.getMat(base.render2d)
            v3 = Vec3(mat.xformPoint(Point3(x, 0., y)))
            
        self.button.setPos(v3)

    def setScale(self, scale):
        self.button.setScale(scale)

    def setDrawOrder(self, drawOrder):
        self.button.setDrawOrder(drawOrder)
