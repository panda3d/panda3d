"DIRECT Nine DoF Placer demonstration"

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
import Pmw
import Dial
import Floater

ZERO_VEC = Vec3(0)
UNIT_VEC = Vec3(1)

class Placer(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Placer Panel',     None),
            ('nodePath',    camera,               None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaToplevel.__init__(self, parent)

        # Initialize state
        self.tempCS = render.attachNewNode(NamedNode('placerTempCS'))
        self.refCS = self.tempCS
        self.undoList = []
        self.redoList = []
        
        # Dictionary keeping track of all node paths manipulated so far
        self.nodePathDict = {}
        self.nodePathDict['camera'] = camera
        self.nodePathNames = ['selected', 'camera']

        self.refNodePathDict = {}
        self.refNodePathDict['render'] = render
        self.refNodePathDict['camera'] = camera
        self.refNodePathNames = ['self', 'render', 'camera', 'selected']

        # Get initial state
        self.initPos = Vec3(0)
        self.initHpr = Vec3(0)
        self.initScale = Vec3(1)

        # Init movement mode
        self.movementMode = 'Absolute'

        # Create panel
        # Handle to the toplevels hull
        hull = self.component('hull')

        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        menuFrame.pack(fill = X, expand = 1)

        balloon = self.balloon = Pmw.Balloon()
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')
        
        menuBar = Pmw.MenuBar(menuFrame, hotkeys = 1, balloon = balloon)
        menuBar.pack(side = LEFT, expand = 1, fill = X)
        menuBar.addmenu('Placer', 'Placer Panel Operations')
        menuBar.addmenuitem('Placer', 'command',
                            'Exit Placer Panel',
                            label = 'Exit',
                            command = self.destroy)
        menuBar.addmenuitem('Placer', 'command',
                            'Undo Pos/Hpr/Scale',
                            label = 'Undo',
                            command = self.undo)
        menuBar.addmenuitem('Placer', 'command',
                            'Redo Pos/Hpr/Scale',
                            label = 'Redo',
                            command = self.redo)
        menuBar.addmenuitem('Placer', 'command',
                            'Reset Node Path',
                            label = 'Reset All',
                            command = self.resetAll)
        menuBar.addmenuitem('Placer', 'command',
                            'Print Node Path Info',
                            label = 'Print Info',
                            command = self.printNodePathInfo)

        menuBar.addmenu('Help', 'Placer Panel Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menuBar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)

        self.nodePathMenu = Pmw.ComboBox(
            menuFrame, labelpos = W, label_text = 'Node Path:',
            entry_width = 20,
            selectioncommand = self.selectNodePathNamed,
            scrolledlist_items = self.nodePathNames)
        self.nodePathMenu.selectitem('selected')
        self.nodePathMenuEntry = (
            self.nodePathMenu.component('entryfield_entry'))
        self.nodePathMenuBG = (
            self.nodePathMenuEntry.configure('background')[3])
        self.nodePathMenu.pack(side = 'left', expand = 0)

        modeMenu = Pmw.OptionMenu(menuFrame,
                                  items = ('Absolute', 'Relative To:'),
                                  command = self.setMovementMode,
                                  menubutton_width = 8)
        modeMenu.pack(side = 'left', expand = 0)
        
        self.refNodePathMenu = Pmw.ComboBox(
            menuFrame, entry_width = 16,
            selectioncommand = self.selectRefNodePathNamed,
            scrolledlist_items = self.refNodePathNames)
        self.refNodePathMenu.selectitem('self')
        self.refNodePathMenuEntry = (
            self.refNodePathMenu.component('entryfield_entry'))
        self.refNodePathMenu.pack(side = 'left', expand = 0)

        # The master frame for the dials
	dialFrame = Frame(hull)
        dialFrame.pack(fill = 'both', expand = 1)
        
	# Create and pack the Pos Controls
	posGroup = Pmw.Group(dialFrame,
                             tag_pyclass = Menubutton,
                             tag_text = 'Position',
                             tag_font=('MSSansSerif', 14, 'bold'),
                             tag_activebackground = '#909090',
                             ring_relief = 'raised')
	posMenubutton = posGroup.component('tag')
	posMenu = Menu(posMenubutton)
	posMenu.add_command(label = 'Set to zero', command = self.zeroPos)
	posMenu.add_command(label = 'Reset initial',
                            command = self.resetPos)
	posMenubutton['menu'] = posMenu
	posGroup.pack(side='left', fill = 'both', expand = 1)
        posInterior = posGroup.interior()

        # Create the dials
	self.posX = self.createcomponent('posX', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'X',
                                         initialValue = 0.0,
                                         label_foreground = 'Red')
        self.posX['command'] = self.xform
        self.posX['commandData'] = ['x']
        self.posX['callbackData'] = ['x']
        self.posX.onPress = self.xformStart
        self.posX.onRelease = self.xformStop
        self.posX.pack(expand=1,fill='x')
        
	self.posY = self.createcomponent('posY', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Y',
                                         initialValue = 0.0,
                                         label_foreground = '#00A000')
        self.posY['command'] = self.xform
        self.posY['commandData'] = ['y']
        self.posY['callbackData'] = ['y']
        self.posY.onPress = self.xformStart
        self.posY.onRelease = self.xformStop
        self.posY.pack(expand=1,fill='x')
        
	self.posZ = self.createcomponent('posZ', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Z',
                                         initialValue = 0.0,
                                         label_foreground = 'Blue')
        self.posZ['command'] = self.xform
        self.posZ['commandData'] = ['z']
        self.posZ['callbackData'] = ['z']
        self.posZ.onPress = self.xformStart
        self.posZ.onRelease = self.xformStop
        self.posZ.pack(expand=1,fill='x')

	# Create and pack the Hpr Controls
	hprGroup = Pmw.Group(dialFrame,
                             tag_pyclass = Menubutton,
                             tag_text = 'Orientation',
                             tag_font=('MSSansSerif', 14, 'bold'),
                             tag_activebackground = '#909090',
                             ring_relief = 'raised')
	hprMenubutton = hprGroup.component('tag')
	hprMenu = Menu(hprMenubutton)
	hprMenu.add_command(label = 'Set to zero', command = self.zeroHpr)
	hprMenu.add_command(label = 'Reset initial', command = self.resetHpr)
	hprMenubutton['menu'] = hprMenu
	hprGroup.pack(side='left',fill = 'both', expand = 1)
        hprInterior = hprGroup.interior()
        
	# Create the dials
	self.hprH = self.createcomponent('hprH', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'H', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = 0.0,
                                         label_foreground = 'blue')
        self.hprH['command'] = self.xform
        self.hprH['commandData'] = ['h']
        self.hprH['callbackData'] = ['h']
        self.hprH.onPress = self.xformStart
        self.hprH.onRelease = self.xformStop
        self.hprH.pack(expand=1,fill='x')
        
	self.hprP = self.createcomponent('hprP', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'P', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = 0.0,
                                         label_foreground = 'red')
        self.hprP['command'] = self.xform
        self.hprP['commandData'] = ['p']
        self.hprP['callbackData'] = ['p']
        self.hprP.onPress = self.xformStart
        self.hprP.onRelease = self.xformStop
        self.hprP.pack(expand=1,fill='x')
        
	self.hprR = self.createcomponent('hprR', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'R', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = 0.0,
                                         label_foreground = '#00A000')
        self.hprR['command'] = self.xform
        self.hprR['commandData'] = ['r']
        self.hprR['callbackData'] = ['r']
        self.hprR.onPress = self.xformStart
        self.hprR.onRelease = self.xformStop
        self.hprR.pack(expand=1,fill='x')

        # Create and pack the Scale Controls
	scaleGroup = Pmw.Group(dialFrame,
                               tag_text = 'Scale',
                               tag_pyclass = Menubutton,
                               tag_font=('MSSansSerif', 14, 'bold'),
                               tag_activebackground = '#909090',
                               ring_relief = 'raised')
	scaleMenubutton = scaleGroup.component('tag')
	scaleMenu = Menu(scaleMenubutton)
	scaleModeMenu = Menu(scaleMenu)
	# The available scaling modes
	self.scalingMode = StringVar()
	self.scalingMode.set('Free')
	scaleModeMenu.add_radiobutton(label = 'Free',
                                      variable = self.scalingMode)
	scaleModeMenu.add_radiobutton(label = 'Uniform',
                                      variable = self.scalingMode)
	scaleModeMenu.add_radiobutton(label = 'Proportional',
                                      variable = self.scalingMode)
	
	# First level scaling menu
	scaleMenu.add_command(label = 'Set to unity',
                              command = self.unitScale)
	scaleMenu.add_command(label = 'Reset initial',
                              command = self.resetScale)
        scaleMenu.add_cascade(label = 'Scaling mode...',
                              menu = scaleModeMenu)
	scaleMenubutton['menu'] = scaleMenu
	scaleGroup.pack(side='left',fill = 'both', expand = 1)
        scaleInterior = scaleGroup.interior()
        
	# Create the dials
	self.scaleX = self.createcomponent('scaleX', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'X Scale',
                                           initialValue = 1.0,
                                           label_foreground = 'Red')
        self.scaleX['command'] = self.xform
        self.scaleX['commandData'] = ['sx']
        self.scaleX['callbackData'] = ['sx']
        self.scaleX.onPress = self.xformStart
        self.scaleX.onRelease = self.xformStop
        self.scaleX.pack(expand=1,fill='x')
        
	self.scaleY = self.createcomponent('scaleY', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'Y Scale',
                                           initialValue = 1.0,
                                           label_foreground = '#00A000')
        self.scaleY['command'] = self.xform
        self.scaleY['commandData'] = ['sy']
        self.scaleY['callbackData'] = ['sy']
        self.scaleY.onPress = self.xformStart
        self.scaleY.onRelease = self.xformStop
        self.scaleY.pack(expand=1,fill='x')
        
	self.scaleZ = self.createcomponent('scaleZ', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'Z Scale',
                                           initialValue = 1.0,
                                           label_foreground = 'Blue')
        self.scaleZ['command'] = self.xform
        self.scaleZ['commandData'] = ['sz']
        self.scaleZ['callbackData'] = ['sz']
        self.scaleZ.onPress = self.xformStart
        self.scaleZ.onRelease = self.xformStop
        self.scaleZ.pack(expand=1,fill='x')

        # Make sure appropriate labels are showing
        self.setMovementMode('Absolute')
        # Set up placer for inital node path
        self.selectNodePathNamed('init')

        # Make sure input variables processed 
        self.initialiseoptions(Placer)

    def setMovementMode(self, movementMode):
        # Set prefix
        namePrefix = ''
        self.movementMode = movementMode
        if (movementMode == 'Drive'):
            namePrefix = 'Drive delta '
        elif (movementMode == 'Orbit'):
            namePrefix = 'Orbit '
        elif (movementMode == 'Absolute'):
            namePrefix = 'Absolute '
        elif (movementMode == 'Relative To:'):
            namePrefix = 'Relative '
        # Enable/disable wrt menu
        if(movementMode == 'Relative To:'):
            self.refNodePathMenu.configure(entry_foreground = 'Black')
            self.refNodePathMenu.configure(entry_background = 'SystemWindow')
        else:
            self.refNodePathMenu.configure(entry_foreground = 'gray50')
            self.refNodePathMenu.configure(entry_background = '#E0E0E0')
        # Update pos widgets
        self.posX['text'] = namePrefix + 'X'
        self.posY['text'] = namePrefix + 'Y'
        self.posZ['text'] = namePrefix + 'Z'
        # Update hpr widgets
        if (movementMode == 'Orbit'):
            namePrefix = 'Orbit delta '
        self.hprH['text'] = namePrefix + 'H'
        self.hprP['text'] = namePrefix + 'P'
        self.hprR['text'] = namePrefix + 'R'
        # Update temp cs and initialize widgets
        self.updatePlacer()

    def selectNodePathNamed(self, name):
        nodePath = None
        if name == 'init':
            nodePath = self['nodePath']
            # Add Combo box entry for the initial node path
            self.addNodePath(nodePath)
        elif name == 'selected':
            nodePath = direct.selected.last
            # Add Combo box entry for this selected object
            self.addNodePath(nodePath)
        else:
            nodePath = self.nodePathDict.get(name, None)
            if (nodePath == None):
                # See if this evaluates into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.nodePathMenu.component('scrolledlist')
                    listbox.setlist(self.nodePathNames)
        # Update active node path
        self.setActiveNodePath(nodePath)

    def setActiveNodePath(self, nodePath):
        self['nodePath'] = nodePath
        if self['nodePath']:
            self.nodePathMenuEntry.configure(
                background = self.nodePathMenuBG)
            # Record initial position
            self.updateResetValues(self['nodePath'])
            # Record initial value and initialize the widgets
            self.updatePlacer()
        else:
            # Flash entry
            self.nodePathMenuEntry.configure(background = 'Pink')

    def selectRefNodePathNamed(self, name):
        nodePath = None
        if name == 'self':
            nodePath = self.tempCS
        elif name == 'selected':
            nodePath = direct.selected.last
            # Add Combo box entry for this selected object
            self.addRefNodePath(nodePath)
        else:
            nodePath = self.refNodePathDict.get(name, None)
            if (nodePath == None):
                # See if this evaluates into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addRefNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.refNodePathMenu.component('scrolledlist')
                    listbox.setlist(self.refNodePathNames)
        # Update ref node path accordingly
        self.setReferenceNodePath(nodePath)

    def setReferenceNodePath(self, nodePath):
        self.refCS = nodePath
        if self.refCS:
            self.refNodePathMenuEntry.configure(
                background = self.nodePathMenuBG)
            # Update placer to reflect new state
            self.updatePlacer()
        else:
            # Flash entry
            self.refNodePathMenuEntry.configure(background = 'Pink')
        
    def addNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.nodePathNames,
                               self.nodePathMenu, self.nodePathDict)

    def addRefNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.refNodePathNames,
                               self.refNodePathMenu, self.refNodePathDict)

    def addNodePathToDict(self, nodePath, names, menu, dict):
        if not nodePath:
            return
        # Get node path's name
        name = nodePath.getName()
        if((name == 'render') | (name == 'camera')):
            dictName = name
        else:
            # Generate a unique name for the dict
            dictName = name + '-' + `nodePath.id().this`
        if not dict.has_key(dictName):
            # Update combo box to include new item
            names.append(dictName)
            listbox = menu.component('scrolledlist')
            listbox.setlist(names)
            # Add new item to dictionary
            dict[dictName] = nodePath
        menu.selectitem(dictName)

    def updatePlacer(self):
        pos = Vec3(0)
        hpr = Vec3(0)
        scale = Vec3(1)
        np = self['nodePath']
        if (np != None) & isinstance(np, NodePath):
            # Update temp CS
            self.tempCS.setPosHpr(self['nodePath'], 0,0,0,0,0,0)
            # Update widgets
            if self.movementMode == 'Absolute':
                pos.assign(np.getPos())
                hpr.assign(np.getHpr())
                scale.assign(np.getScale())
            elif self.refCS:
                pos.assign(np.getPos(self.refCS))
                hpr.assign(np.getHpr(self.refCS))
                scale.assign(np.getScale(self.refCS))
        self.updatePosWidgets(pos)
        self.updateHprWidgets(hpr)
        self.updateScaleWidgets(scale)

    def xform(self, value, axis):
        if self.movementMode == 'Absolute':
            self.xformAbsolute(value, axis)
        elif self.movementMode == 'Relative To:':
            self.xformRelative(value, axis)
    
    def xformStart(self, data):
        # Record undo point
        self.pushUndo()
        # Record initial scale
        # Update placer to reflect new state
        self.updatePlacer()
        
    def xformStop(self, data):
        # Update placer to reflect new state
        self.updatePlacer()

    def xformAbsolute(self, value, axis):
        nodePath = self['nodePath']
        if nodePath != None:
            if axis == 'x':
                nodePath.setX(value)
            elif axis == 'y':
                nodePath.setY(value)
            elif axis == 'z':
                nodePath.setZ(value)
            elif axis == 'h':
                nodePath.setH(value)
            elif axis == 'p':
                nodePath.setP(value)
            elif axis == 'r':
                nodePath.setR(value)
                
    def xformRelative(self, value, axis):
        nodePath = self['nodePath']
        if (nodePath != None) & (self.refCS != None):
            if axis == 'x':
                nodePath.setX(self.refCS, value)
            elif axis == 'y':
                nodePath.setY(self.refCS, value)
            elif axis == 'z':
                nodePath.setZ(self.refCS, value)
            elif axis == 'h':
                nodePath.setH(self.refCS, value)
            elif axis == 'p':
                nodePath.setP(self.refCS, value)
            elif axis == 'r':
                nodePath.setR(self.refCS, value)

    def updatePosWidgets(self, pos):
        self.posX.set(pos[0])
        self.posY.set(pos[1])
        self.posZ.set(pos[2])

    def updateHprWidgets(self, hpr):
        self.hprH.set(hpr[0])
        self.hprP.set(hpr[1])
        self.hprR.set(hpr[2])

    def updateScaleWidgets(self, scale):
        self.scaleX.set(scale[0])
        self.scaleY.set(scale[1])
        self.scaleZ.set(scale[2])

    def zeroPos(self):
        self.updatePosWidgets(ZERO_VEC)

    def zeroHpr(self):
        self.updateHprWidgets(ZERO_VEC)

    def unitScale(self):
        self.updateScaleWidgets(UNIT_VEC)

    def updateResetValues(self, nodePath):
        self.initPos.assign(nodePath.getPos())
        self.initHpr.assign(nodePath.getHpr())
        self.initScale.assign(nodePath.getScale())

    def resetAll(self):
        self.resetPos()
        self.resetHpr()
        self.resetScale()

    def resetPos(self):
        self.updatePosWidgets(self.initPos)

    def resetHpr(self):
        self.updateHprWidgets(self.initHpr)

    def resetScale(self):
        self.updateScaleWidgets(self.initScale)

    def pushUndo(self):
        nodePath = self['nodePath']
        if nodePath != None:
            print 'here'
            pos = nodePath.getPos()
            hpr = nodePath.getHpr()
            scale = nodePath.getScale()
            self.undoList.append([nodePath, pos,hpr,scale])

    def undo(self):
        if self.undoList:
            # Get last item off of redo list
            pose = self.undoList[-1]
            # Tack it onto the end of the redo list
            self.redoList.append(pose)
            # Strip it off of the undo list
            self.undoList = self.undoList[:-1]
            self.selectNodePathNamed(pose[0].getName())
            self.updatePosWidgets(pose[1])
            self.updateHprWidgets(pose[2])
            self.updateScaleWidgets(pose[3])

    def redo(self):
        if self.redoList:
            # Pull off last redo list item
            pose = self.redoList[-1]
            # Tack it onto the start of undo list
            self.undoList.append(pose)
            # Strip it off of redo list
            self.redoList = self.redoList[:-1]
            self.selectNodePathNamed(pose[0].getName())
            self.updatePosWidgets(pose[1])
            self.updateHprWidgets(pose[2])
            self.updateScaleWidgets(pose[3])

    def printNodePathInfo(self):
        print 'Print Node Path info here'

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

def place(nodePath):
    return Placer(nodePath = nodePath)

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    widget = Placer()

