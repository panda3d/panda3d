from ShowBaseGlobal import *
from GuiGlobals import *
import PandaObject
import Button
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

    def cleanupPanel(self, uniqueName):
        """cleanupPanel(self, string uniqueName)

        Cleans up (removes) the panel with the given uniqueName.  This
        may be useful when some panels know about each other and know
        that opening panel A should automatically close panel B, for
        instance.
        """
        
        if OnscreenPanel.AllPanels.has_key(uniqueName):
            OnscreenPanel.AllPanels[uniqueName].cleanup()
            del OnscreenPanel.AllPanels[uniqueName]

    def makePanel(self,
                  rect = (-0.5, 0.5, -0.5, 0.5),
                  bg = (1, 1, 1, 1),
                  geom = getDefaultPanel(),
                  geomRect = (-0.5, 0.5, -0.5, 0.5),
                  drawOrder = 0,
                  font = getDefaultFont(),
                  support3d = 0):
        """makePanel()

        Initializes the geometry to render the panel with the
        specified parameters.  This should only be called once, and
        generally by the __init__ function.

        The parameters are as follows:

          rect: the left, right, bottom, top of the panel on the
              screen.  This is in aspect2d coordinates.  The panel
              will be set up in its own coordinate system so that (0,
              0) is the center of the panel.

          bg: the r, g, b, a background color of the panel.

          geom: the model to use as the background panel geometry.
              Normally you can safely let this default.

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
        self.cleanupPanel(uniqueName)

        # Store this panel in our map of all open panels.
        OnscreenPanel.AllPanels[uniqueName] = self
        
        self.panelSetup = 1
        self.panelDrawOrder = drawOrder
        self.panelFont = font

        self.panelButtons = []
        self.panelText = []

        centerX = (rect[0] + rect[1]) / 2.0
        centerY = (rect[2] + rect[3]) / 2.0
        self.setPos(centerX, 0, centerY)

        if isinstance(geom, types.StringType):
            # If 'geom' is a string, it's the name of a model to load.
            self.panelGeom = loader.loadModelOnce(geom)
            self.panelGeom.reparentTo(self)
        else:
            # Otherwise, it's a model to instance.
            self.panelGeom = geom.instanceTo(self)

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

        self.setBin('fixed', self.panelDrawOrder)

        if support3d:
            # If we're supposed to support 3-d geometry in front of
            # the panel, set a few extra attributes to ensure the
            # panel geometry fills up the depth buffer as it goes,
            # making it safe to put 3-d geometry in front of it.
            dw = DepthWriteTransition()
            self.panelGeom.setY(100)
            self.panelGeom.arc().setTransition(dw, 2)

        # Set up the panel as its own mouse region so mouse clicks on
        # the panel don't inadvertently drive the toon around.
        self.panelRegion = MouseWatcherRegion(uniqueName, 0, 0, 0, 0)
        self.panelRegion.setRelative(self.panelGeom,
                                     geomRect[0], geomRect[1],
                                     geomRect[2], geomRect[3])
        base.mouseWatcher.node().addRegion(self.panelRegion)

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

        for button in self.panelButtons:
            button.cleanup()
            self.ignore(button.getName() + '-down-rollover')

        for text in self.panelText:
            text.cleanup()

        base.mouseWatcher.node().removeRegion(self.panelRegion)

        if not self.isEmpty():
            self.removeNode()

        self.panelSetup = 0
        return 1

    def makeButton(self, name,
                   func = None,
                   manage = 1,
                   label = None,
                   scale = 0.1,
                   width = None,
                   drawOrder = None,
                   font = None,
                   pos = (0, 0),
                   geomRect = None):
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

        if label == None:
            label = name
        if drawOrder == None:
            drawOrder = self.panelDrawOrder + 10
        if font == None:
            font = self.panelFont

        buttonName = self.getUniqueName() + '-' + name
            
        button = Button.Button(buttonName,
                               label = label,
                               scale = scale,
                               width = width,
                               drawOrder = drawOrder,
                               font = font,
                               pos = pos,
                               geomRect = geomRect)

        self.panelButtons.append(button)
        
        if manage:
            button.manage(self)

        if func != None:
            self.accept(buttonName + '-down-rollover', func)

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
                 parent = None):
        """makeText(self, ...)

        Creates some text on the panel.  The return value is an
        OnscreenText object.  The position of the text is relative to
        the panel, where (0, 0) is the center.

        The text need not be further managed by the derived class.  It
        will automatically be removed when cleanup() is called.
        
        """

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
                                         parent = parent)
        
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

