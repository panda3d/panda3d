from DirectFrame import *

"""
import DirectWaitBar
d = DirectWaitBar.DirectWaitBar()

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
            ('width',           1.0,              self.setup),
            ('height',          0.2,              self.setup),
            ('range',           100,              self.setup),
            ('value',           50,               self.setValue),
            ('barBorderWidth',  (0,0),            self.setBarBorderWidth),
            ('barColor',        (1,0,0,1),     self.setBarColor),
            ('barRelief',       FLAT,             self.setBarRelief),
            )

        self.barStyle = PGFrameStyle()
        
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # Call option initialization functions
        self.initialiseoptions(DirectWaitBar)

        self.setup()
        self.updateBarStyle()
        

    def setup(self):
        print self['width'], self['height'], self['range']
        self.guiItem.setup(self['width'], self['height'], self['range'])

    def setValue(self):
        self.guiItem.setValue(self['value'])

    def getPercent(self):
        return self.guiItem.getPercent()

    def updateBarStyle(self):
        if not self.fInit:
            print 'updateing'
            self.guiItem.setBarStyle(self.barStyle)            

    def setBarRelief(self):
        print 1
        self.barStyle.setType(self['barRelief'])
        self.updateBarStyle()

    def setBarBorderWidth(self):
        print 2
        self.barStyle.setWidth(*self['barBorderWidth'])
        self.updateBarStyle()

    def setBarColor(self):
        print 3
        self.barStyle.setColor(*self['barColor'])
        self.updateBarStyle()
        
        
    
