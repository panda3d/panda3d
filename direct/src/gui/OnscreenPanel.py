from ShowBaseGlobal import *
import GuiGlobals
import PandaObject
import Button
import Label
import OnscreenText
import types


def findPanel(uniqueName):
    """findPanel(string uniqueName)
    
    Returns the panel whose uniqueName is given.  This is mainly
    useful for debugging, to get a pointer to the current onscreen
    panel of a particular type.

    """
    if OnscreenPanel.AllPanels.has_key(uniqueName):
        return OnscreenPanel.AllPanels[uniqueName]
    return None

def cleanupPanel(uniqueName):
    """cleanupPanel(string uniqueName)

    Cleans up (removes) the panel with the given uniqueName.  This
    may be useful when some panels know about each other and know
    that opening panel A should automatically close panel B, for
    instance.
    """
        
    if OnscreenPanel.AllPanels.has_key(uniqueName):
        # calling cleanup() will remove it out of the AllPanels dict
        # This way it will get removed from the dict even it we did
        # not clean it up using this interface (ie somebody called
        # self.cleanup() directly
        OnscreenPanel.AllPanels[uniqueName].cleanup()


class OnscreenPanel(PandaObject.PandaObject, NodePath):
    """OnscreenPanel:

    This class defines the basic interface to a user-interface panel
    that pops up within the Panda window, overlaying whatever graphics
    we may have already.

    """

    AllPanels = {}

    def __init__(self, panelName):
        self.panelName = panelName
        self.panelSetup = 0
        
        # initialize our NodePath essence.
        NodePath.__init__(self, aspect2d.attachNewNode(panelName))
        NodePath.hide(self)
        
    def makePanel(self,
                  rect = (-0.5, 0.5, -0.5, 0.5),
                  bg = (1, 1, 1, 1),
                  geom = GuiGlobals.getDefaultPanel(),
                  geomRect = (-0.5, 0.5, -0.5, 0.5),
                  drawOrder = 0,
                  font = GuiGlobals.getDefaultFont(),
                  support3d = 0):
        """makePanel()

        Initializes the geometry to render the panel with the
        specified parameters.  This should only be called once, and
        generally by the __init__ function.

        The parameters are as follows:

          rect: the (left, right, bottom, top) of the panel on the
              screen.  This is in aspect2d coordinates.  The panel
              will be set up in its own coordinate system so that (0,
              0) is the center of the panel.

          bg: the (r, g, b, a) background color of the panel.

          geom: the model to use as the background panel geometry.
              Normally you can safely let this default.

          geomRect: the (left, right, bottom, top) rectangle around
              the background panel geometry as it is modeled.  This is
              used to compute how the panel should be scaled to make
              it fit the rectangle specified by rect, above.

          drawOrder: the drawing order of this panel with respect to
              all other things in the 'fixed' bin within render2d.
              Buttons and text created within the panel via
              makeButton() and makeText() will by default be given a
              drawOrder slightly higher than this.  Normally you can
              safely let this default, unless you really expect this
              panel to overlay some other panels.

          font: the default font for buttons and text created within
              the panel via makeButton() and makeText().

          support3d: if this is set true, the panel will be set up so
              that 3-d geometry (like a floating head) may be safely
              parented to the panel.
        """

        assert not self.panelSetup

        # Clean up any previously existing panel with the same unique
        # name.  We don't allow any two panels with the same name to
        # coexist.
        uniqueName = self.getUniqueName()
        cleanupPanel(uniqueName)

        # Store this panel in our map of all open panels.
        OnscreenPanel.AllPanels[uniqueName] = self
        
        self.panelSetup = 1
        self.panelDrawOrder = drawOrder
        self.panelFont = font

        self.panelButtons = []
        self.panelText = []

        if geom == None:
            # If 'geom' is None, it means not to have a background
            # panel at all.
            self.panelGeom = None
            
        elif isinstance(geom, types.StringType):
            # If 'geom' is a string, it's the name of a model to load.
            self.panelGeom = loader.loadModelCopy(geom)
            self.panelGeom.reparentTo(self)
        else:
            # Otherwise, it's a model to instance.
            self.panelGeom = geom.instanceTo(self)

        centerX = (rect[0] + rect[1]) / 2.0
        centerY = (rect[2] + rect[3]) / 2.0
        NodePath.setPos(self, centerX, 0, centerY)

        self.setBin('fixed', self.panelDrawOrder)
        self.panelRegion = None

        if self.panelGeom != None:
            # Scale and position the geometry to fill up our desired
            # rectangle.
            gCenterX = (geomRect[0] + geomRect[1]) / 2.0
            gCenterY = (geomRect[2] + geomRect[3]) / 2.0

            self.panelGeom.setPos(-gCenterX, 0, -gCenterY)
            self.panelGeom.setScale((rect[1] - rect[0]) / (geomRect[1] - geomRect[0]), 1,
                                    (rect[3] - rect[2]) / (geomRect[3] - geomRect[2]))

            if bg[3] != 1:
                self.panelGeom.setTransparency(1)
            self.panelGeom.setColor(bg[0], bg[1], bg[2], bg[3])

            if support3d:
                # If we're supposed to support 3-d geometry in front of
                # the panel, set a few extra attributes to ensure the
                # panel geometry fills up the depth buffer as it goes,
                # making it safe to put 3-d geometry in front of it.
                dw = DepthWriteTransition()
                self.panelGeom.setY(100)
                self.panelGeom.arc().setTransition(dw, 2)

            # Set up the panel as its own mouse region so mouse clicks on
            # the panel don't inadvertently drive the toon around.  This
            # must be done after the panelGeom has been scaled
            # appropriately, above.
            self.geomRect = geomRect
            self.panelRegion = MouseWatcherRegion(uniqueName, 0, 0, 0, 0)
            self.panelRegion.setRelative(self.panelGeom,
                                         geomRect[0], geomRect[1],
                                         geomRect[2], geomRect[3])
            self.panelRegion.setSort(self.panelDrawOrder)

    def cleanup(self):
        """cleanup(self):

        Removes the panel and frees all the resources associated with
        it.  This must be called to safely remove the panel from the
        screen.

        The return value is true if the panel was cleaned up, or false
        if it had already been cleaned up.

        """
        if not self.panelSetup:
            return 0

        self.hide()
        
        for button in self.panelButtons:
            button.cleanup()
        del self.panelButtons

        for text in self.panelText:
            text.cleanup()
        del self.panelText

        if not self.isEmpty():
            self.removeNode()

        # Remove this panel out of the AllPanels list
        uniqueName = self.getUniqueName()
        if OnscreenPanel.AllPanels.has_key(uniqueName):
            del OnscreenPanel.AllPanels[uniqueName]

        self.panelSetup = 0
        return 1

    def show(self):
        """show(self):
        Show everything and hang hooks
        """
        if not self.panelSetup:
            return 0

        NodePath.show(self)

        # show the buttons that are meant to be shown
        for button in self.panelButtons:
            if button.panelManage:
                button.manage(self)
            if button.func != None:
                if (button.event != None):
                    self.accept(button.event, button.func, [button.button])
                else:
                    self.accept(button.button.getDownRolloverEvent(),
                                button.func, [button.button])
                button.startBehavior()

        if self.panelRegion != None:
            base.mouseWatcherNode.addRegion(self.panelRegion)
        
    def hide(self):
        """hide(self):
        Hide everything and remove hooks
        """
        if not self.panelSetup:
            return 0

        NodePath.hide(self)

        # hide the shown buttons and remove all hooks
        for button in self.panelButtons:        
            if (button.event != None):
                self.ignore(button.event)
            else:
                self.ignore(button.button.getDownRolloverEvent())
            if button.panelManage:
                button.unmanage()

        if self.panelRegion != None:
            base.mouseWatcherNode.removeRegion(self.panelRegion)

    def makeButton(self, name,
                   func = None,
                   manage = 1,
                   label = None,
                   labels = None,
                   scale = 0.1,
                   width = None,
                   align = None,
                   drawOrder = None,
                   font = None,
                   pos = (0, 0),
                   geomRect = None,
                   supportInactive = 0,
                   inactive = 0,
                   upStyle = Label.ButtonUp,
                   litStyle = Label.ButtonLit,
                   downStyle = Label.ButtonDown,
                   inactiveStyle = Label.ButtonInactive,
                   event = None):
        """makeButton(self, ...)

        Creates a button on the panel.  The return value is the button
        itself.  The position of the button is relative to the panel,
        where (0, 0) is the center.

        The button will automatically be managed (i.e. made visible)
        unless manage is set to 0.

        If func is specified, it is the name of a function that will
        be called when the button is clicked.  In this case, a hook
        will automatically be assigned for the button, and removed
        when cleanup() is called.
        """

        assert self.panelSetup

        if (label == None) and (labels == None):
            label = name
        if drawOrder == None:
            drawOrder = self.panelDrawOrder + 10
        if font == None:
            font = self.panelFont

        buttonName = self.getUniqueName() + '-' + name
            
        button = Button.Button(buttonName,
                               label = label,
                               labels = labels,
                               scale = scale,
                               width = width,
                               align = align,
                               drawOrder = drawOrder,
                               font = font,
                               pos = pos,
                               geomRect = geomRect,
                               supportInactive = supportInactive,
                               inactive = inactive,
                               upStyle = upStyle,
                               litStyle = litStyle,
                               downStyle = downStyle,
                               inactiveStyle = inactiveStyle,
                               event = event)

        self.panelButtons.append(button)
        
        button.panelManage = manage
        button.func = func
        
        return button

    def makeText(self, text = '',
                 style = OnscreenText.Plain,
                 pos = (0, 0),
                 scale = None,
                 fg = None,
                 bg = None,
                 shadow = None,
                 frame = None,
                 align = None,
                 wordwrap = None,
                 drawOrder = None,
                 font = None,
                 parent = None,
                 mayChange = 0):
        """makeText(self, ...)

        Creates some text on the panel.  The return value is an
        OnscreenText object.  The position of the text is relative to
        the panel, where (0, 0) is the center.

        The text need not be further managed by the derived class.  It
        will automatically be removed when cleanup() is called.
        
        """

        assert self.panelSetup

        if drawOrder == None:
            drawOrder = self.panelDrawOrder + 10
        if font == None:
            font = self.panelFont
        if parent == None:
            parent = self

        text = OnscreenText.OnscreenText(text,
                                         style = style,
                                         pos = pos,
                                         scale = scale,
                                         fg = fg,
                                         bg = bg,
                                         shadow = shadow,
                                         frame = frame,
                                         align = align,
                                         wordwrap = wordwrap,
                                         drawOrder = drawOrder,
                                         font = font,
                                         parent = parent,
                                         mayChange = mayChange)
        
        self.panelText.append(text)
        return text

    def getUniqueName(self):
        """getUniqueName(self):

        Returns a name unique to this panel.  If no two instances of
        the same panel will ever be in existence at once, this can
        simply be the panel name.  If, for some reason, you want to
        define a panel type that can have multiple instances, you
        should redefine this function to return a unique name for each
        instance.

        """
        
        return self.panelName


    def setPos(self, x, y, z):
        """setPos(self, x, y, z)

        Repositions the panel onscreen, taking all of the panel's
        managed buttons along with it.
        
        """
        assert self.panelSetup
        NodePath.setPos(self, x, y, z)

        for button in self.panelButtons:
            if button.managed:
                button.unmanage()
                button.manage(self)

        if self.panelRegion != None:
            self.panelRegion.setRelative(self.panelGeom,
                                         self.geomRect[0], self.geomRect[1],
                                         self.geomRect[2], self.geomRect[3])

