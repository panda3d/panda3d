"DIRECT Nine DoF Placer demonstration"

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
import Pmw
import Dial
import Floater

"""
TODO:
Orbital mode
Task to monitor pose
"""

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
        self.orbitFromCS = render.attachNewNode(NamedNode('placerOrbitFromCS'))
        self.orbitToCS = render.attachNewNode(NamedNode('placerOrbitToCS'))
        self.refCS = self.tempCS
        self.undoList = []
        self.redoList = []
        
        # Dictionary keeping track of all node paths manipulated so far
        self.nodePathDict = {}
        self.nodePathDict['camera'] = camera
        self.nodePathNames = ['selected', 'camera']

        self.refNodePathDict = {}
        self.refNodePathDict['parent'] = self['nodePath'].getParent()
        self.refNodePathDict['render'] = render
        self.refNodePathDict['camera'] = camera
        self.refNodePathNames = ['self', 'parent', 'render',
                                 'camera', 'selected', 'coa']

        # Initial state
        self.initPos = Vec3(0)
        self.initHpr = Vec3(0)
        self.initScale = Vec3(1)

        # Offset for orbital mode
        self.posOffset = Vec3(0)

        # Init movement mode
        self.movementMode = 'Relative To:'

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
                            'Reset Node Path',
                            label = 'Reset All',
                            command = self.resetAll)
        menuBar.addmenuitem('Placer', 'command',
                            'Print Node Path Info',
                            label = 'Print Info',
                            command = self.printNodePathInfo)
        menuBar.addmenuitem('Placer', 'command',
                            'Exit Placer Panel',
                            label = 'Exit',
                            command = self.preDestroy)

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
                                  items = ('Relative To:',
                                           'Orbit:'),
                                  initialitem = 'Relative To:',
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

        self.undoButton = Button(menuFrame, text = 'Undo',
                                 state = 'disabled',
                                 command = self.undo)
        self.undoButton.pack(side = 'left', expand = 0)
        self.redoButton = Button(menuFrame, text = 'Redo',
                                 state = 'disabled',
                                 command = self.redo)
        self.redoButton.pack(side = 'left', expand = 0)

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
        self.posX.onReturn = self.pushUndo
        self.posX.onReturnRelease = self.xformStop
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
        self.posY.onReturn = self.pushUndo
        self.posY.onReturnRelease = self.xformStop
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
        self.posZ.onReturn = self.pushUndo
        self.posZ.onReturnRelease = self.xformStop
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
        self.hprH.onReturn = self.pushUndo
        self.hprH.onReturnRelease = self.xformStop
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
        self.hprP.onReturn = self.pushUndo
        self.hprP.onReturnRelease = self.xformStop
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
        self.hprR.onReturn = self.pushUndo
        self.hprR.onReturnRelease = self.xformStop
        self.hprR.onPress = self.xformStart
        self.hprR.onRelease = self.xformStop
        self.hprR.pack(expand=1,fill='x')

        # Create and pack the Scale Controls
	# The available scaling modes
	self.scalingMode = StringVar()
	self.scalingMode.set('Scale Uniform')
        # The scaling widgets
	scaleGroup = Pmw.Group(dialFrame,
                               tag_text = 'Scale Uniform',
                               tag_pyclass = Menubutton,
                               tag_font=('MSSansSerif', 14, 'bold'),
                               tag_activebackground = '#909090',
                               ring_relief = 'raised')
	self.scaleMenubutton = scaleGroup.component('tag')
        self.scaleMenubutton['textvariable'] = self.scalingMode
	
	# Scaling menu
	scaleMenu = Menu(self.scaleMenubutton)
	scaleMenu.add_command(label = 'Set to unity',
                              command = self.unitScale)
	scaleMenu.add_command(label = 'Reset initial',
                              command = self.resetScale)
	scaleMenu.add_radiobutton(label = 'Scale Free',
                                      variable = self.scalingMode)
	scaleMenu.add_radiobutton(label = 'Scale Uniform',
                                      variable = self.scalingMode)
	scaleMenu.add_radiobutton(label = 'Scale Proportional',
                                      variable = self.scalingMode)
	self.scaleMenubutton['menu'] = scaleMenu
        # Pack group widgets
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
        self.scaleX.onReturn = self.pushUndo
        self.scaleX.onReturnRelease = self.xformStop
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
        self.scaleY.onReturn = self.pushUndo
        self.scaleY.onReturnRelease = self.xformStop
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
        self.scaleZ.onReturn = self.pushUndo
        self.scaleZ.onReturnRelease = self.xformStop
        self.scaleZ.onPress = self.xformStart
        self.scaleZ.onRelease = self.xformStop
        self.scaleZ.pack(expand=1,fill='x')

        # Make sure appropriate labels are showing
        self.setMovementMode('Relative To:')
        # Set up placer for inital node path
        self.selectNodePathNamed('init')

        # Make sure input variables processed 
        self.initialiseoptions(Placer)

    def setMovementMode(self, movementMode):
        # Set prefix
        namePrefix = ''
        self.movementMode = movementMode
        if (movementMode == 'Relative To:'):
            namePrefix = 'Relative '
        elif (movementMode == 'Orbit:'):
            namePrefix = 'Orbit '
        # Update pos widgets
        self.posX['text'] = namePrefix + 'X'
        self.posY['text'] = namePrefix + 'Y'
        self.posZ['text'] = namePrefix + 'Z'
        # Update hpr widgets
        if (movementMode == 'Orbit:'):
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
        elif name == 'coa':
            if direct.selected.last:
                nodePath = self.tempCS
            else:
                nodePath = None
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
        if name in ['parent', 'render', 'camera']:
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
            self.updateAuxiliaryCoordinateSystems()
            # Update widgets
            if self.movementMode == 'Orbit:':
                pos.assign(self.posOffset)
                hpr.assign(ZERO_VEC)
                scale.assign(np.getScale())
            elif self.refCS:
                pos.assign(np.getPos(self.refCS))
                hpr.assign(np.getHpr(self.refCS))
                scale.assign(np.getScale())
        self.updatePosWidgets(pos)
        self.updateHprWidgets(hpr)
        self.updateScaleWidgets(scale)

    def updateAuxiliaryCoordinateSystems(self):
        # Temp CS
        if ((self.refNodePathMenuEntry.get() == 'coa') &
            (direct.selected.last != None)):
            pos = Vec3(0)
            hpr = Vec3(0)
            decomposeMatrix(direct.selected.last.mCoa2Dnp,
                            VBase3(0), hpr, pos, CSDefault)
            self.tempCS.setPosHpr(direct.selected.last, pos, hpr)
        else:
            self.tempCS.setPosHpr(self['nodePath'], 0,0,0,0,0,0)
        # Orbit CS
        # At reference
        self.orbitFromCS.setPos(self.refCS, 0,0,0)
        # But aligned with target
        self.orbitFromCS.setHpr(self['nodePath'], 0,0,0)
        # Also update to CS
        self.orbitToCS.setPosHpr(self.orbitFromCS, 0,0,0,0,0,0)
        # Get offset from origin
        self.posOffset.assign(self['nodePath'].getPos(self.orbitFromCS))

    def xform(self, value, axis):
        if axis in ['sx', 'sy', 'sz']:
            self.xformScale(value,axis)
        elif self.movementMode == 'Relative To:':
            self.xformRelative(value, axis)
        elif self.movementMode == 'Orbit:':
            self.xformOrbit(value, axis)
    
    def xformStart(self, data):
        # Record undo point
        self.pushUndo()
        # Record initial scale
        # Update placer to reflect new state
        self.updatePlacer()
        
    def xformStop(self, data):
        # Throw event to signal manipulation done
        messenger.send('manipulateObjectCleanup')
        # Update placer to reflect new state
        self.updatePlacer()

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

    def xformOrbit(self, value, axis):
        nodePath = self['nodePath']
        if ((nodePath != None) & (self.refCS != None) &
            (self.orbitFromCS != None) & (self.orbitToCS != None)):
            if axis == 'x':
                self.posOffset.setX(value)
            elif axis == 'y':
                self.posOffset.setY(value)
            elif axis == 'z':
                self.posOffset.setZ(value)
            elif axis == 'h':
                self.orbitToCS.setH(self.orbitFromCS, value)
            elif axis == 'p':
                self.orbitToCS.setP(self.orbitFromCS, value)
            elif axis == 'r':
                self.orbitToCS.setR(self.orbitFromCS, value)
            nodePath.setPosHpr(self.orbitToCS, self.posOffset, ZERO_VEC)

    def xformScale(self, value, axis):
        if self['nodePath']:
            mode = self.scalingMode.get()
            scale = self['nodePath'].getScale()
            if mode == 'Scale Free':
                if axis == 'sx':
                    scale.setX(value)
                elif axis == 'sy':
                    scale.setY(value)
                elif axis == 'sz':
                    scale.setZ(value)
            elif mode == 'Scale Uniform':
                scale.set(value,value,value)
            elif mode == 'Scale Proportional':
                if axis == 'sx':
                    sf = value/scale[0]
                elif axis == 'sy':
                    sf = value/scale[1]
		elif axis == 'sz':
                    sf = value/scale[2]
                scale = scale * sf
            self['nodePath'].setScale(scale)

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
        if self['nodePath']:
            self['nodePath'].setPosHprScale(
                self.initPos, self.initHpr, self.initScale)
            self.updatePlacer()

    def resetPos(self):
        self['nodePath'].setPos(self.initPos)
        self.updatePlacer()

    def resetHpr(self):
        self['nodePath'].setHpr(self.initHpr)
        self.updatePlacer()

    def resetScale(self):
        self['nodePath'].setScale(self.initScale)
        self.updatePlacer()

    def pushUndo(self, fResetRedo = 1):
        nodePath = self['nodePath']
        if nodePath != None:
            name = self.nodePathMenuEntry.get()
            pos = nodePath.getPos()
            hpr = nodePath.getHpr()
            scale = nodePath.getScale()
            self.undoList.append([name, pos,hpr,scale])
            # Make sure button is reactivated
            self.undoButton.configure(state = 'normal')
            if fResetRedo:
                self.redoList = []
                self.redoButton.configure(state = 'disabled')

    def popUndo(self):
        # Get last item
        pose = self.undoList[-1]
        # Strip last item off of undo list
        self.undoList = self.undoList[:-1]
        # Update state of undo button
        if not self.undoList:
            self.undoButton.configure(state = 'disabled')
        # Return last item
        return pose
        
    def pushRedo(self):
        nodePath = self['nodePath']
        if nodePath != None:
            name = self.nodePathMenuEntry.get()
            pos = nodePath.getPos()
            hpr = nodePath.getHpr()
            scale = nodePath.getScale()
            self.redoList.append([name, pos,hpr,scale])
            # Make sure button is reactivated
            self.redoButton.configure(state = 'normal')

    def popRedo(self):
        # Get last item
        pose = self.redoList[-1]
        # Strip last item off of redo list
        self.redoList = self.redoList[:-1]
        # Update state of redo button
        if not self.redoList:
            self.redoButton.configure(state = 'disabled')
        # Return last item
        return pose
        
    def undo(self):
        if self.undoList:
            # Get last item off of redo list
            pose = self.popUndo()
            # Make that node path active
            self.selectNodePathNamed(pose[0])
            # Make sure combo box is showing the right thing
            self.nodePathMenu.selectitem(pose[0])
            # Record active's current state on redo stack
            self.pushRedo()
            # Undo xform
            self['nodePath'].setPosHprScale(pose[1], pose[2], pose[3])
            # Reflect new changes
            self.updatePlacer()

    def redo(self):
        if self.redoList:
            # Get last item off of redo list
            pose = self.popRedo()
            # Make that node path active
            self.selectNodePathNamed(pose[0])
            # Make sure combo box is showing the right thing
            self.nodePathMenu.selectitem(pose[0])
            # Record active's current state on redo stack
            self.pushUndo(fResetRedo = 0)
            # Redo xform
            self['nodePath'].setPosHprScale(pose[1], pose[2], pose[3])
            # Reflect new changes
            self.updatePlacer()

    def printNodePathInfo(self):
        np = self['nodePath']
        if np:
            name = np.getName()
            pos = np.getPos()
            hpr = np.getHpr()
            scale = np.getScale()
            posString = '%.2f, %.2f, %.2f' % (pos[0], pos[1], pos[2])
            hprString = '%.2f, %.2f, %.2f' % (hpr[0], hpr[1], hpr[2])
            scaleString = '%.2f, %.2f, %.2f' % (scale[0], scale[1], scale[2])
            print 'NodePath: %s' % name
            print 'Pos: %s' % posString
            print 'Hpr: %s' % hprString
            print 'Scale: %s' % scaleString
            print ('%s.setPosHprScale(%s, %s, %s)' %
                   (name, posString, hprString, scaleString))

    def preDestroy(self):
        self.tempCS.removeNode()
        self.orbitFromCS.removeNode()
        self.orbitToCS.removeNode()
        self.destroy()
        
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

