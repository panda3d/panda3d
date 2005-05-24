from DirectButton import *
from DirectFrame import *

"""
import DirectSliderBar
d = DirectSliderBar(borderWidth=(0,0))

"""

class DirectSliderButton(DirectButton):
    def __init__(self, parent = None, **kw):
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGSliderButton,   None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        # Initialize superclasses
        DirectButton.__init__(self,parent)
        # Call option initialization functions
        self.initialiseoptions(DirectSliderButton)

class DirectSliderBar(DirectFrame):
    """
    DirectEntry(parent) - Create a DirectGuiWidget which responds
    to keyboard buttons
    """
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGSliderBar,        None),
            ('width',          10,                 None),
            ('height',         1,                  None),
            ('button',         None,               None),
            ('sliderOnly',     0,                  self.setSliderOnly),
            ('negativeMapping',0,                  self.setNegativeMapping),
            ('range',          100,                self.setRange),
            ('value',          0,                  self.setValue),
            ('barBorderWidth', (0,0),              self.setBarBorderWidth),
            ('barColor',       (1,0,0,1),          self.setBarColor),
            ('barRelief',      FLAT,               self.setBarRelief),
            ('active',         0,                  self.setActive),
            ('sortOrder',      NO_FADE_SORT_INDEX, None),
            # Command to be called on button movement
            ('command',        None,               None),
            ('extraArgs',      [],                 None),
            )
        if kw.has_key('text'):
            textoptiondefs = (
                ('text_pos',    (0,-0.025),          None),
                ('text_scale',  0.1,                 None),
                ('text_align',  TextNode.ALeft,      None)
                )
        else:
            textoptiondefs = ()
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs + textoptiondefs)
        # Initialize superclasses
        DirectFrame.__init__(self, parent)
        self.barStyle = PGFrameStyle()
        # Call option initialization functions
        self.initialiseoptions(DirectSliderBar)

        if (self['button'] != None):
            self.guiItem.setSliderButton(self['button'], self['button'].guiItem)
        
        if (self['image'] != None):
            self.guiItem.setScale(self['image_scale'][0])
            self.guiItem.setup(self.getWidth(), self.getHeight(), self['range'])
        else:
            #figure out what is happening?????
            #self.guiItem.setState(0)
            #self.guiItem.clearStateDef(0)
            self.guiItem.setFrame(-3.0, 3.0, -0.125, 0.125)
            self.barStyle.setWidth(0.05, 0.05)
            self.barStyle.setColor(0.6,0.6,0.6,1)
            self.barStyle.setType(PGFrameStyle.TBevelIn)
            self.guiItem.setFrameStyle(0, self.barStyle)
            self.guiItem.setScale(2)
            self.guiItem.setup(6, 0.5, self['range'])
            self.guiItem.setValue(self['value'])
            if (self['scale'] != None):
                self.setScale(self['scale'])
            else:
                self.setScale(0.1)

        self.guiItem.setActive(1)

        #self.barStyle.setColor(0.8,0.8,0.8,1)
        #self.barStyle.setType(PGFrameStyle.TBevelOut)
        #self.updateBarStyle()

        if (self['command'] != None):
            # Attach command function to slider button movement
            self.bind('updated-slider-', self.commandFunc)

    def destroy(self):
        del self.barStyle
        DirectFrame.destroy(self)
        
    def setSliderOnly(self):
        self.guiItem.setSliderOnly(self['sliderOnly'])

    def setNegativeMapping(self):
        self.guiItem.setNegativeMapping(self['negativeMapping'])

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
        color = self['barColor']
        self.barStyle.setColor(color[0], color[1], color[2], color[3])
        self.updateBarStyle()

    def setActive(self):
        self.guiItem.setActive(self['active'])

    def commandFunc(self):
        if self['command']:
            # Pass any extra args to command
            apply(self['command'], self['extraArgs'])
            
