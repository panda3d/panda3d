from DirectGuiBase import *

class DirectButton(DirectGuiBase, NodePath):
    def __init__(self, parent = guiTop, **kw):
        # Pass in a background texture, and/or a geometry object,
        # and/or a text string to be used as the visible
        # representation of the button, or pass in a list of geometry
        # objects, one for each state (normal, rollover, pressed,
        # disabled)
        # Bounding box to be used in button/mouse interaction
        # If None, use bounding box of actual button geometry
        # Otherwise, you can pass in:
        #  - a list of [L,R,B,T] (in aspect2d coords)
        #  - a VBase4(L,R,B,T)
        #  - a bounding box object
        optiondefs = (
            ('image',           None,       self.setImage),
            ('geom',            None,       self.setGeom),
            ('text',            '',         self.setText),
            ('command',         None,       None),
            ('commandButtons',  (1,),       self.setCommandButtons),
            ('relief',          FLAT,       self.setRelief),
            ('frameColor',      (1,1,1,1),  self.setFrameColor),
            ('borderWidth',     (.1,.1),    self.setBorderWidth),
            ('frameSize',       None,       self.setFrameSize),
            ('pressEffect',     1,          None),
            ('padSX',           1.2,        None),
            ('padSZ',           1.1,        None),
            ('pos',             None,       None),
            ('scale',           None,       None),
            ('state',           NORMAL,     self.setState),
            ('rolloverSound',   None,       None),
            ('clickSound',      None,       None),
            )
        # Update options to reflect keyword parameters
        apply(DirectGuiBase.__init__, (self, optiondefs, ('text',)), kw)
        # Initialize the superclass
        NodePath.__init__(self)
        # Create a button
        self.guiItem = PGButton()
        self.guiId = self.guiItem.getId()
        # Attach button to parent and make that self
        self.assign(parent.attachNewNode( self.guiItem ) )
        # Set up names
        self.guiItem.setName(self.guiId)
        self.setName(self.guiId + 'NodePath')
        self.stateNodePath = []
        for i in range(4):
            self.stateNodePath.append(NodePath(self.guiItem.getStateDef(i)))
        if self['pressEffect']:
            np = self.stateNodePath[1].attachNewNode('pressEffect')
            np.setScale(0.98)
            self.stateNodePath[1] = np
        # Initialize frame style
        self.frameStyle = []
        for i in range(4):
            self.frameStyle.append(PGFrameStyle())
        # For holding bounds info
        self.ll = Point3(0)
        self.ur = Point3(0)
        # Call initialization functions if necessary
        # To avoid doing things redundantly
        self.fInit = 1
        self.initialiseoptions(DirectButton)
        self.fInit = 0
        # Allow changes to take effect
        self.updateFrameStyle()
        if not self['frameSize']:
            self.setFrameSize()
        # Update pose
        if self['pos']:
            if type(self['pos']) == type(()):
                apply(self.setPos, self['pos'])
            else:
                apply(self.setPos, (self['pos'],))
        if self['scale']:
            if type(self['scale']) == type(()):
                apply(self.setScale, self['scale'])
            else:
                apply(self.setScale, (self['scale'],))

    def updateFrameStyle(self):
        for i in range(4):
            self.guiItem.setFrameStyle(i, self.frameStyle[i])

    def setRelief(self, fSetStyle = 1):
        print 'setting Frame'
        relief = self['relief']
        if relief == None:
            for i in range(4):
                self.frameStyle[i].setType(PGFrameStyle.TNone)
        elif (relief == FLAT) or (relief == 'flat'):
            for i in range(4):
                self.frameStyle[i].setType(FLAT)
        elif (relief == RAISED) or (relief == 'raised'):
            for i in (0,2,3):
                self.frameStyle[i].setType(RAISED)
            self.frameStyle[1].setType(SUNKEN)
        elif (relief == SUNKEN) or (relief == 'sunken'):
            for i in (0,2,3):
                self.frameStyle[i].setType(SUNKEN)
            self.frameStyle[1].setType(RAISED)
        if not self.fInit:
            self.updateFrameStyle()

    def resetFrameSize(self):
        self.setFrameSize(fClearFrame = 1)
        
    def setFrameSize(self, fClearFrame = 0):
        if self['frameSize']:
            # Use user specified bounds
            bounds = self['frameSize']
        else:
            # Use ready state to compute bounds
            frameType = self.frameStyle[0].getType()
            if fClearFrame and (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(PGFrameStyle.TNone)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])
                # To force an update of the button
                self.guiItem.getStateDef(0)
            # Clear out frame before computing bounds
            self.stateNodePath[0].calcTightBounds(self.ll, self.ur)
            # Scale bounds to give a pad around graphics
            bounds = (self.ll[0] * self['padSX'], self.ur[0] * self['padSX'],
                      self.ll[2] * self['padSZ'], self.ur[2] * self['padSZ'])
            # Restore frame style if necessary
            if (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(frameType)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])
        # Set frame to new dimensions
        self.guiItem.setFrame(bounds[0], bounds[1],bounds[2], bounds[3])

    def setFrameColor(self):
        color = self['frameColor']
        for i in range(4):
            self.frameStyle[i].setColor(color[0], color[1], color[2], color[3])
        if not self.fInit:
            self.updateFrameStyle()

    def setBorderWidth(self):
        width = self['borderWidth']
        for i in range(4):
            self.frameStyle[i].setWidth(width[0], width[1])
        if not self.fInit:
            self.updateFrameStyle()

    def setText(self):
        if not self['text']:
            return
        if ((type(self['text']) == type(())) or
            (type(self['text']) == type([]))):
            text = self['text']
        else:
            text = (self['text'],) * 4
        for i in range(4):
            component = 'text' + `i`
            if not self.hascomponent(component):
                self.createcomponent(
                    component, (), 'text',
                    OnscreenText.OnscreenText,
                    (), parent = self.stateNodePath[i],
                    text = text[i], scale = 1,
                    mayChange = 1)
            else:
                self[component + '_text'] = text[i]

    def setGeom(self):
        if not self['geom']:
            return
        if ((type(self['geom']) == type(())) or
            (type(self['geom']) == type([]))):
            geom = self['geom']
        else:
            geom = (self['geom'],) * 4
        for i in range(4):
            component = 'geom' + `i`
            if not self.hascomponent(component):
                self.createcomponent(
                    component, (), 'geom',
                    OnscreenGeom.OnscreenGeom,
                    (), parent = self.stateNodePath[i],
                    geom = geom[i], scale = 1)
            else:
                self[component + '_geom'] = geom[i]

    def setImage(self):
        if not self['image']:
            return
        if type(self['image']) == type(''):
            # Assume its a file name
            image = (self['image'],) * 4
        elif ((type(self['image']) == type(())) or
            (type(self['image']) == type([]))):
            if len(self['image']) == 2:
                # Assume its a model/node pair of strings
                image = (self['image'],) * 4
            elif len(self['image']) == 4:
                # Assume its a 4 tuple with one entry per state
                image = self['image']
            else:
                print 'DirectButton.setImage: wrong argument'
        for i in range(4):
            component = 'image' + `i`
            if not self.hascomponent(component):
                self.createcomponent(
                    component, (), 'image',
                    OnscreenImage.OnscreenImage,
                    (), parent = self.stateNodePath[i],
                    image = image[i], scale = 1)
            else:
                self[component + '_image'] = image[i]

    def setState(self):
        if type(self['state']) == type(0):
            self.guiItem.setActive(self['state'])
        elif (self['state'] == NORMAL) or (self['state'] == 'normal'):
            self.guiItem.setActive(1)
        else:
            self.guiItem.setActive(0)

    def setCommandButtons(self):
        # Attach command function to specified buttons
        self.unbind(B1CLICK)
        self.guiItem.removeClickButton(MouseButton.one())
        self.unbind(B2CLICK)
        self.guiItem.removeClickButton(MouseButton.two())
        self.unbind(B3CLICK)
        self.guiItem.removeClickButton(MouseButton.three())
        for i in self['commandButtons']:
            if i == 1:
                self.guiItem.addClickButton(MouseButton.one())
                self.bind(B1CLICK, self.commandFunc)
            elif i == 2:
                self.guiItem.addClickButton(MouseButton.two())
                self.bind(B2CLICK, self.commandFunc)
            elif i == 3:
                self.guiItem.addClickButton(MouseButton.three())
                self.bind(B3CLICK, self.commandFunc)

    def commandFunc(self, event):
        if self['command']:
            self['command'](event)
            
