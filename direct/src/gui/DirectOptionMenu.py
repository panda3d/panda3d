"""Implements a pop-up menu containing multiple clickable options."""

__all__ = ['DirectOptionMenu']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from .DirectButton import *
from .DirectLabel import *
from .DirectFrame import *

class DirectOptionMenu(DirectButton):
    """
    DirectOptionMenu(parent) - Create a DirectButton which pops up a
    menu which can be used to select from a list of items.
    Execute button command (passing the selected item through) if defined
    To cancel the popup menu click anywhere on the screen outside of the
    popup menu.  No command is executed in this case.
    """
    def __init__(self, parent = None, **kw):
        # Inherits from DirectButton
        optiondefs = (
            # List of items to display on the popup menu
            ('items',       [],             self.setItems),
            # Initial item to display on menu button
            # Can be an integer index or the same string as the button
            ('initialitem', None,           DGG.INITOPT),
            # Amount of padding to place around popup button indicator
            ('popupMarkerBorder', (.1, .1), None),
            # The initial position of the popup marker
            ('popupMarker_pos', None, None),
            # Background color to use to highlight popup menu items
            ('highlightColor', (.5, .5, .5, 1), None),
            # Extra scale to use on highlight popup menu items
            ('highlightScale', (1, 1), None),
            # Alignment to use for text on popup menu button
            # Changing this breaks button layout
            ('text_align',  TextNode.ALeft, None),
            # Remove press effect because it looks a bit funny
            ('pressEffect',     0,          DGG.INITOPT),
           )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        # Initialize superclasses
        DirectButton.__init__(self, parent)
        # Record any user specified frame size
        self.initFrameSize = self['frameSize']
        # Create a small rectangular marker to distinguish this button
        # as a popup menu button
        self.popupMarker = self.createcomponent(
            'popupMarker', (), None,
            DirectFrame, (self,),
            frameSize = (-0.5, 0.5, -0.2, 0.2),
            scale = 0.4,
            relief = DGG.RAISED)
        # Record any user specified popup marker position
        self.initPopupMarkerPos = self['popupMarker_pos']
        # This needs to popup the menu too
        self.popupMarker.bind(DGG.B1PRESS, self.showPopupMenu)
        # Check if item is highlighted on release and select it if it is
        self.popupMarker.bind(DGG.B1RELEASE, self.selectHighlightedIndex)
        # Make popup marker have the same click sound
        if self['clickSound']:
            self.popupMarker.guiItem.setSound(
                DGG.B1PRESS + self.popupMarker.guiId, self['clickSound'])
        else:
            self.popupMarker.guiItem.clearSound(DGG.B1PRESS + self.popupMarker.guiId)
        # This is created when you set the menu's items
        self.popupMenu = None
        self.selectedIndex = None
        self.highlightedIndex = None
        # A big screen encompassing frame to catch the cancel clicks
        self.cancelFrame = self.createcomponent(
            'cancelframe', (), None,
            DirectFrame, (self,),
            frameSize = (-1, 1, -1, 1),
            relief = None,
            state = 'normal')
        # Make sure this is on top of all the other widgets
        self.cancelFrame.setBin('gui-popup', 0)
        self.cancelFrame.node().setBounds(OmniBoundingVolume())
        self.cancelFrame.bind(DGG.B1PRESS, self.hidePopupMenu)
        # Default action on press is to show popup menu
        self.bind(DGG.B1PRESS, self.showPopupMenu)
        # Check if item is highlighted on release and select it if it is
        self.bind(DGG.B1RELEASE, self.selectHighlightedIndex)
        # Call option initialization functions
        self.initialiseoptions(DirectOptionMenu)
        # Need to call this since we explicitly set frame size
        self.resetFrameSize()

    def setItems(self):
        """
        self['items'] = itemList
        Create new popup menu to reflect specified set of items
        """
        # Remove old component if it exits
        if self.popupMenu != None:
            self.destroycomponent('popupMenu')
        # Create new component
        self.popupMenu = self.createcomponent('popupMenu', (), None,
                                              DirectFrame,
                                              (self,),
                                              relief = 'raised',
                                              )
        # Make sure it is on top of all the other gui widgets
        self.popupMenu.setBin('gui-popup', 0)
        if not self['items']:
            return
        # Create a new component for each item
        # Find the maximum extents of all items
        itemIndex = 0
        self.minX = self.maxX = self.minZ = self.maxZ = None
        for item in self['items']:
            c = self.createcomponent(
                'item%d' % itemIndex, (), 'item',
                DirectButton, (self.popupMenu,),
                text = item, text_align = TextNode.ALeft,
                command = lambda i = itemIndex: self.set(i))
            bounds = c.getBounds()
            if self.minX == None:
                self.minX = bounds[0]
            elif bounds[0] < self.minX:
                self.minX = bounds[0]
            if self.maxX == None:
                self.maxX = bounds[1]
            elif bounds[1] > self.maxX:
                self.maxX = bounds[1]
            if self.minZ == None:
                self.minZ = bounds[2]
            elif bounds[2] < self.minZ:
                self.minZ = bounds[2]
            if self.maxZ == None:
                self.maxZ = bounds[3]
            elif bounds[3] > self.maxZ:
                self.maxZ = bounds[3]
            itemIndex += 1
        # Calc max width and height
        self.maxWidth = self.maxX - self.minX
        self.maxHeight = self.maxZ - self.minZ
        # Adjust frame size for each item and bind actions to mouse events
        for i in range(itemIndex):
            item = self.component('item%d' %i)
            # So entire extent of item's slot on popup is reactive to mouse
            item['frameSize'] = (self.minX, self.maxX, self.minZ, self.maxZ)
            # Move it to its correct position on the popup
            item.setPos(-self.minX, 0, -self.maxZ - i * self.maxHeight)
            item.bind(DGG.B1RELEASE, self.hidePopupMenu)
            # Highlight background when mouse is in item
            item.bind(DGG.WITHIN,
                      lambda x, i=i, item=item:self._highlightItem(item, i))
            # Restore specified color upon exiting
            fc = item['frameColor']
            item.bind(DGG.WITHOUT,
                      lambda x, item=item, fc=fc: self._unhighlightItem(item, fc))
        # Set popup menu frame size to encompass all items
        f = self.component('popupMenu')
        f['frameSize'] = (0, self.maxWidth, -self.maxHeight * itemIndex, 0)

        # Determine what initial item to display and set text accordingly
        if self['initialitem']:
            self.set(self['initialitem'], fCommand = 0)
        else:
            # No initial item specified, just use first item
            self.set(0, fCommand = 0)

        # Position popup Marker to the right of the button
        pm = self.popupMarker
        pmw = (pm.getWidth() * pm.getScale()[0] +
               2 * self['popupMarkerBorder'][0])
        if self.initFrameSize:
            # Use specified frame size
            bounds = list(self.initFrameSize)
        else:
            # Or base it upon largest item
            bounds = [self.minX, self.maxX, self.minZ, self.maxZ]
        if self.initPopupMarkerPos:
            # Use specified position
            pmPos = list(self.initPopupMarkerPos)
        else:
            # Or base the position on the frame size.
            pmPos = [bounds[1] + pmw/2.0, 0, bounds[2] + (bounds[3] - bounds[2])/2.0]
        pm.setPos(pmPos[0], pmPos[1], pmPos[2])
        # Adjust popup menu button to fit all items (or use user specified
        # frame size
        bounds[1] += pmw
        self['frameSize'] = (bounds[0], bounds[1], bounds[2], bounds[3])
        # Set initial state
        self.hidePopupMenu()

    def showPopupMenu(self, event = None):
        """
        Make popup visible and try to position it just to right of
        mouse click with currently selected item aligned with button.
        Adjust popup position if default position puts it outside of
        visible screen region
        """

        # Needed attributes (such as minZ) won't be set unless the user has specified
        # items to display. Let's assert that we've given items to work with.
        items = self['items']
        assert items and len(items) > 0, 'Cannot show an empty popup menu! You must add items!'

        # Show the menu
        self.popupMenu.show()
        # Make sure its at the right scale
        self.popupMenu.setScale(self, VBase3(1))
        # Compute bounds
        b = self.getBounds()
        fb = self.popupMenu.getBounds()
        # Position menu at midpoint of button
        xPos = (b[1] - b[0])/2.0 - fb[0]
        self.popupMenu.setX(self, xPos)
        # Try to set height to line up selected item with button
        self.popupMenu.setZ(
            self, self.minZ + (self.selectedIndex + 1)*self.maxHeight)
        # Make sure the whole popup menu is visible
        pos = self.popupMenu.getPos(render2d)
        scale = self.popupMenu.getScale(render2d)
        # How are we doing relative to the right side of the screen
        maxX = pos[0] + fb[1] * scale[0]
        if maxX > 1.0:
            # Need to move menu to the left
            self.popupMenu.setX(render2d, pos[0] + (1.0 - maxX))
        # How about up and down?
        minZ = pos[2] + fb[2] * scale[2]
        maxZ = pos[2] + fb[3] * scale[2]
        if minZ < -1.0:
            # Menu too low, move it up
            self.popupMenu.setZ(render2d, pos[2] + (-1.0 - minZ))
        elif maxZ > 1.0:
            # Menu too high, move it down
            self.popupMenu.setZ(render2d, pos[2] + (1.0 - maxZ))
        # Also display cancel frame to catch clicks outside of the popup
        self.cancelFrame.show()
        # Position and scale cancel frame to fill entire window
        self.cancelFrame.setPos(render2d, 0, 0, 0)
        self.cancelFrame.setScale(render2d, 1, 1, 1)

    def hidePopupMenu(self, event = None):
        """ Put away popup and cancel frame """
        self.popupMenu.hide()
        self.cancelFrame.hide()

    def _highlightItem(self, item, index):
        """ Set frame color of highlighted item, record index """
        item['frameColor'] = self['highlightColor']
        item['frameSize'] = (self['highlightScale'][0]*self.minX, self['highlightScale'][0]*self.maxX, self['highlightScale'][1]*self.minZ, self['highlightScale'][1]*self.maxZ)
        item['text_scale'] = self['highlightScale']
        self.highlightedIndex = index

    def _unhighlightItem(self, item, frameColor):
        """ Clear frame color, clear highlightedIndex """
        item['frameColor'] = frameColor
        item['frameSize'] = (self.minX, self.maxX, self.minZ, self.maxZ)
        item['text_scale'] = (1,1)
        self.highlightedIndex = None

    def selectHighlightedIndex(self, event = None):
        """
        Check to see if item is highlighted (by cursor being within
        that item).  If so, selected it.  If not, do nothing
        """
        if self.highlightedIndex is not None:
            self.set(self.highlightedIndex)
            self.hidePopupMenu()

    def index(self, index):
        intIndex = None
        if isinstance(index, int):
            intIndex = index
        elif index in self['items']:
            i = 0
            for item in self['items']:
                if item == index:
                    intIndex = i
                    break
                i += 1
        return intIndex

    def set(self, index, fCommand = 1):
        # Item was selected, record item and call command if any
        newIndex = self.index(index)
        if newIndex is not None:
            self.selectedIndex = newIndex
            item = self['items'][self.selectedIndex]
            self['text'] = item
            if fCommand and self['command']:
                # Pass any extra args to command
                self['command'](*[item] + self['extraArgs'])

    def get(self):
        """ Get currently selected item """
        return self['items'][self.selectedIndex]

    def commandFunc(self, event):
        """
        Override popup menu button's command func
        Command is executed in response to selecting menu items
        """
        pass
