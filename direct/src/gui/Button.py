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
                 pos = (0, 0),
                 geomRect = None,
                 style = Label.ButtonUp):
        self.name = name
        self.width = width
        # if no label given, use the button name
        if (label == None):
            label = self.name

        # check to see if this is an actual guiLabel or just text
        if (type(label) == type('')):
            # text label, make text button
            self.label = label

            self.l1 = Label.textLabel(self.label, style,
                                      scale, width, drawOrder, font)

            if width == None:
                width = self.l1.getWidth() / scale
                self.width = width
            
            self.l2 = Label.textLabel(self.label, Label.ButtonLit,
                                      scale, width, drawOrder, font)
            self.l3 = Label.textLabel(self.label, Label.ButtonDown,
                                      scale, width, drawOrder, font)

        elif (isinstance(label, NodePath)):
            # If it's a NodePath, assume it's a little texture card.
            self.l1 = Label.modelLabel(label,
                                       geomRect = geomRect,
                                       scale = scale,
                                       drawOrder = drawOrder)

            if width == None:
                width = self.l1.getWidth() / scale
                self.width = width
            
            self.l2 = self.l1
            self.l3 = self.l1
            
        else:
            # label provided, use it for all labels
            self.l1 = self.l2 = self.l3 = label
            if width == None:
                width = self.l1.getWidth()

        self.button = GuiButton.GuiButton(self.name, self.l1, self.l2,
                                          self.l3, self.l3, self.l1)
        self.button.setDrawOrder(drawOrder)
        self.setPos(pos[0], pos[1])
        self.managed = 0

	return None

    def cleanup(self):
        if (self.managed):
            self.unmanage()
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
        return self.width
    
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

