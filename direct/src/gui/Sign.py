from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiSign
import GuiLabel
import Label

class Sign(DirectObject):

    def __init__(self, name,
                 label = None,
                 style = Label.Sign,
                 scale = 0.1,
                 width = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont()):
        self.name = name
        self.labelText = None
        
        if not label:
            label = self.name
                
        if (type(label) == type('')):
            (self.label, self.labelText) = \
                         Label.textLabelAndText(label, style,
                                                scale, width, drawOrder, font)
        else:
            self.label = label

        self.sign = GuiSign.GuiSign(self.name, self.label)
        self.managed = 0
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
        if (self.managed):
            self.unmanage()
        self.sign = None
	return None

    def __str__(self):
        return "sign: %s contains label: %s" % (self.name, self.label)
    
    # accessing
    def getName(self):
        return self.name

    def setText(self, text):
        self.labelText.setText(text)
        
    def getLabel(self):
        return self.label
    
    def getGuiItem(self):
        return self.sign

    def getPos(self):
        self.sign.getPos()
        
    def setPos(self, x, y):
        self.sign.setPos(Vec3(x, 0, y))

    def getWidth(self):
        return self.label.getWidth()
        
    # actions
    def manage(self, nodepath = aspect2d):
        if nodepath:
            self.sign.manage(guiMgr, base.eventMgr.eventHandler,
                               nodepath.node())
        else:
            self.sign.manage(guiMgr, base.eventMgr.eventHandler)
        self.managed = 1

    def unmanage(self):
        self.managed = 0
        self.sign.unmanage()
