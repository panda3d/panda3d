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
    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGWaitBar,          None),
            ('frameSize',      (-1,1,-0.08,0.08),  None),
            ('borderWidth',    (0,0),              None),
            ('range',          100,                self.setRange),
            ('value',          0,                  self.setValue),
            ('barBorderWidth', (0,0),              self.setBarBorderWidth),
            ('barColor',       (1,0,0,1),          self.setBarColor),
            ('barRelief',      FLAT,               self.setBarRelief),
            ('sortOrder',      NO_FADE_SORT_INDEX, None),
            )
        if kw.has_key('text'):
            textoptiondefs = (
                ('text_pos',    (0,-0.025),          None),
                ('text_scale',  0.1,                 None)
                )
        else:
            textoptiondefs = ()
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs + textoptiondefs)
        # Initialize superclasses
        DirectFrame.__init__(self, parent)
        self.barStyle = PGFrameStyle()
        # Call option initialization functions
        self.initialiseoptions(DirectWaitBar)
        self.updateBarStyle()

    def destroy(self):
        del self.barStyle
        DirectFrame.destroy(self)
        
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

    def update(self, value):
        self['value'] = value

        # Render a frame out-of-sync with the igloop to update the
        # window right now.  This allows the wait bar to be updated
        # even though we are not normally rendering frames.
        base.graphicsEngine.renderFrame()

    def finish(self):
        # Fill the bar in N frames
        N = 10 # take 10 frames
        remaining = self['range'] - self['value']
        if remaining:
            step = max(1, int(remaining / N))
            count = self['value']
            while count != self['range']:
                count += step
                if count > self['range']:
                    count = self['range']
                self.update(count)
        
