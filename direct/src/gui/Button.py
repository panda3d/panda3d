from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiButton
import Label


class Button(DirectObject):

    def __init__(self, name,
                 label = None,
                 scale = 0.1,
                 width = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont(),
                 height = None,
                 left = None,
                 right = None,
                 bottom = None,
                 top = None):
        self.name = name
        # if no label given, use the button name
        if (label == None):
            label = self.name

        # check to see if this is an actual guiLabel or just text
        if (type(label) == type('')):
            # text label, make text button
            self.label = label

            self.l1 = Label.textLabel(self.label, Label.ButtonUp,
                                      scale, width, drawOrder, font)
            self.l2 = Label.textLabel(self.label, Label.ButtonLit,
                                      scale, width, drawOrder, font)
            self.l3 = Label.textLabel(self.label, Label.ButtonDown,
                                      scale, width, drawOrder, font)

        elif (isinstance(label, NodePath)):
            # If it's a NodePath, assume it's a little texture card.
            if height != None and width != None:
                self.l1 = Label.modelLabel(label, height, width,
                                           scale = scale,
                                           drawOrder = drawOrder)
            elif left != None and right != None and \
                 bottom != None and top != None:
                self.l1 = Label.modelLabel(label, left, right, bottom, top,
                                           scale = scale,
                                           drawOrder = drawOrder)
            else:
                self.l1 = Label.modelLabel(label, 1, 1,
                                           scale = scale,
                                           drawOrder = drawOrder)
            
            self.l2 = self.l1
            self.l3 = self.l1
            
        else:
            # label provided, use it for all labels
            self.l1 = self.l2 = self.l3 = label

        self.button = GuiButton.GuiButton(self.name, self.l1, self.l2,
                                          self.l3, self.l3, self.l1)
        self.button.setDrawOrder(drawOrder)
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

    def getPos(self):
        return self.button.getPos()
    
    def setPos(self, x, y, node = None):
        if node == None:
            v3 = Vec3(x, 0., y)
        else:
            mat = node.getMat(base.render2d)
            v3 = Vec3(mat.xformPoint(Point3(x, 0., y)))
            
        self.button.setPos(v3)

