from DirectFrame import *

"""
import DirectWaitBar
d = DirectWaitBar(borderWidth=(0,0))

"""

class DirectWaitBar(DirectFrame):
    """
    DirectEntry(parent) - Create a DirectGuiWidget which responds
    to keyboard buttons
    """
    def __init__(self, parent = guiTop, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',          PGWaitBar,        None),
            ('frameSize',       (-1,1,-0.1,0.1),  None),
            ('range',           100,              self.setRange),
            ('value',           50,               self.setValue),
            ('barBorderWidth',  (0,0),            self.setBarBorderWidth),
            ('barColor',        (1,0,0,1),        self.setBarColor),
            ('barRelief',       FLAT,             self.setBarRelief),
            )
        
        self.barStyle = PGFrameStyle()
        
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # Call option initialization functions
        self.initialiseoptions(DirectWaitBar)

        self.updateBarStyle()
        
    def setRange(self):
        self.guiItem.setRange(self['range'])

    def setValue(self):
        self.guiItem.setValue(self['value'])

    def getPercent(self):
        return self.guiItem.getPercent()

    def updateBarStyle(self):
        if not self.fInit:
            self.guiItem.setBarStyle(self.barStyle)            

    def setBarRelief(self):
        self.barStyle.setType(self['barRelief'])
        self.updateBarStyle()

    def setBarBorderWidth(self):
        self.barStyle.setWidth(*self['barBorderWidth'])
        self.updateBarStyle()

    def setBarColor(self):
        self.barStyle.setColor(*self['barColor'])
        self.updateBarStyle()
        
        
    
