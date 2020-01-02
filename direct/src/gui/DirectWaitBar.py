"""Contains the DirectWaitBar class, a progress bar widget.

See the :ref:`directwaitbar` page in the programming manual for a more
in-depth explanation and an example of how to use this class.
"""

__all__ = ['DirectWaitBar']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectFrame import *
import sys

if sys.version_info >= (3, 0):
    stringType = str
else:
    stringType = basestring

"""
import DirectWaitBar
d = DirectWaitBar(borderWidth=(0, 0))
"""

class DirectWaitBar(DirectFrame):
    """ DirectWaitBar - A DirectWidget that shows progress completed
    towards a task.  """

    def __init__(self, parent = None, **kw):
        # Inherits from DirectFrame
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGWaitBar,          None),
            ('frameSize',      (-1, 1, -0.08, 0.08),  None),
            ('borderWidth',    (0, 0),              None),
            ('range',          100,                self.setRange),
            ('value',          0,                  self.setValue),
            ('barBorderWidth', (0, 0),             self.setBarBorderWidth),
            ('barColor',       (1, 0, 0, 1),       self.setBarColor),
            ('barTexture',     None,               self.setBarTexture),
            ('barRelief',      DGG.FLAT,           self.setBarRelief),
            ('sortOrder',      DGG.NO_FADE_SORT_INDEX, None),
            )
        if 'text' in kw:
            textoptiondefs = (
                ('text_pos',    (0, -0.025),          None),
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
        self.barStyle = None
        DirectFrame.destroy(self)

    def setRange(self):
        """Updates the bar range which you can set using bar['range'].
        This is the value at which the WaitBar indicates 100%."""
        self.guiItem.setRange(self['range'])

    def setValue(self):
        """Updates the bar value which you can set using bar['value'].
        The value should range between 0 and bar['range']."""
        self.guiItem.setValue(self['value'])

    def getPercent(self):
        """Returns the percentage complete."""
        return self.guiItem.getPercent()

    def updateBarStyle(self):
        if not self.fInit:
            self.guiItem.setBarStyle(self.barStyle)

    def setBarRelief(self):
        """Updates the bar relief, which you can set using bar['barRelief']."""
        self.barStyle.setType(self['barRelief'])
        self.updateBarStyle()

    def setBarBorderWidth(self):
        """Updates the bar's border width, which you can set using bar['barBorderWidth']."""
        self.barStyle.setWidth(*self['barBorderWidth'])
        self.updateBarStyle()

    def setBarColor(self):
        """Updates the bar color, which you can set using bar['barColor']."""
        color = self['barColor']
        self.barStyle.setColor(color[0], color[1], color[2], color[3])
        self.updateBarStyle()

    def setBarTexture(self):
        """Updates the bar texture, which you can set using bar['barTexture']."""
        # this must be a single texture (or a string).
        texture = self['barTexture']
        if isinstance(texture, stringType):
            texture = loader.loadTexture(texture)
        if texture:
            self.barStyle.setTexture(texture)
        else:
            self.barStyle.clearTexture()
        self.updateBarStyle()

    def update(self, value):
        """Updates the bar with the given value and renders a frame."""
        self['value'] = value

        # Render a frame out-of-sync with the igLoop to update the
        # window right now.  This allows the wait bar to be updated
        # even though we are not normally rendering frames.
        base.graphicsEngine.renderFrame()

    def finish(self, N = 10):
        """Fill the bar in N frames. This call is blocking."""
        remaining = self['range'] - self['value']
        if remaining:
            step = max(1, int(remaining / N))
            count = self['value']
            while count != self['range']:
                count += step
                if count > self['range']:
                    count = self['range']
                self.update(count)

