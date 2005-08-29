from DirectFrame import *
from DirectButton import *
from DirectGuiBase import _OPT_VALUE

"""
import DirectScrollBar
d = DirectScrollBar(borderWidth=(0,0))

"""

class DirectScrollBar(DirectFrame):
    """
    DirectScrollBar -- a widget which represents a scroll bar the user can
    use for paging through a large document or panel.
    """
    def __init__(self, parent = None, **kw):
        optiondefs = (
            # Define type of DirectGuiWidget
            ('pgFunc',         PGSliderBar,        None),
            ('state',          NORMAL,             None),
            ('frameColor',     (0.6,0.6,0.6,1),    None),

            ('range',          (0, 1),             self.setRange),
            ('value',          0,                  self.__setValue),
            ('scrollSize',     0.01,               self.setScrollSize),
            ('pageSize',       0.1,                self.setPageSize),
            ('orientation',    HORIZONTAL,         self.setOrientation),
            ('manageButtons',  1,                  self.setManageButtons),
            ('resizeThumb',    1,                  self.setResizeThumb),

            # Function to be called repeatedly as the bar is scrolled
            ('command',        None,               None),
            ('extraArgs',      [],                 None),
            )

        if kw.get('orientation') == VERTICAL:
            # These are the default options for a vertical layout.
            optiondefs += (
                ('frameSize',      (-0.04,0.04,-0.5,0.5),   None),
                )
        else:
            # These are the default options for a horizontal layout.
            optiondefs += (
                ('frameSize',      (-0.5,0.5,-0.04,0.04),  None),
                )

        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        self.thumb = self.createcomponent(
            "thumb", (), None,
            DirectButton, (self,),
            borderWidth = self['borderWidth'])
        self.incButton = self.createcomponent(
            "incButton", (), None,
            DirectButton, (self,),
            borderWidth = self['borderWidth'])
        self.decButton = self.createcomponent(
            "decButton", (), None,
            DirectButton, (self,),
            borderWidth = self['borderWidth'])

        self.guiItem.setThumbButton(self.thumb.guiItem)
        self.guiItem.setLeftButton(self.decButton.guiItem)
        self.guiItem.setRightButton(self.incButton.guiItem)

        # Bind command function
        self.bind(ADJUST, self.commandFunc)

        # Call option initialization functions
        self.initialiseoptions(DirectScrollBar)
        
    def setRange(self):
        # Try to preserve the value across a setRange call.
        v = self['value']
        r = self['range']
        self.guiItem.setRange(r[0], r[1])
        self['value'] = v

    def __setValue(self):
        # This is the internal function that is called when
        # self['value'] is directly assigned.
        self.guiItem.setValue(self['value'])

    def setValue(self, value):
        # This is the public function that is meant to be called by a
        # user that doesn't like to use (or doesn't understand) the
        # preferred interface of self['value'].
        self['value'] = value

    def getValue(self):
        return self.guiItem.getValue()

    def getRatio(self):
        return self.guiItem.getRatio()

    def setScrollSize(self):
        self.guiItem.setScrollSize(self['scrollSize'])

    def setPageSize(self):
        self.guiItem.setPageSize(self['pageSize'])

    def setOrientation(self):
        if self['orientation'] == HORIZONTAL:
            self.guiItem.setAxis(Vec3(1, 0, 0))
        else:
            self.guiItem.setAxis(Vec3(0, 0, -1))

    def setManageButtons(self):
        self.guiItem.setManagePieces(self['manageButtons'])

    def setResizeThumb(self):
        self.guiItem.setResizeThumb(self['resizeThumb'])

    def destroy(self):
        DirectFrame.destroy(self)

    def commandFunc(self):
        # Store the updated value in self['value']
        self._optionInfo['value'][_OPT_VALUE] = self.guiItem.getValue()

        if self['command']:
            apply(self['command'], self['extraArgs'])
            
