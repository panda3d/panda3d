""" DIRECT Nine DoF Manipulation Panel """

__all__ = ['Placer', 'place']

# Import Tkinter, Pmw, and the dial code from this directory tree.
from pandac.PandaModules import *
from direct.showbase.TkGlobal import *
from direct.tkwidgets.AppShell import *
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater
from direct.directtools.DirectGlobals import ZERO_VEC, UNIT_VEC
from Tkinter import *
import Pmw

"""
TODO:
Task to monitor pose
"""

class Placer(AppShell):
    # Override class variables here
    appname = 'Placer Panel'
    frameWidth      = 625
    frameHeight     = 215
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       self.appname,       None),
            ('nodePath',    base.direct.camera,      None),
            )
        self.defineoptions(kw, optiondefs)

        # Call superclass initialization function
        AppShell.__init__(self)

        self.initialiseoptions(Placer)

    def appInit(self):
        # Initialize state
        self.tempCS = base.direct.group.attachNewNode('placerTempCS')
        self.orbitFromCS = base.direct.group.attachNewNode(
            'placerOrbitFromCS')
        self.orbitToCS = base.direct.group.attachNewNode('placerOrbitToCS')
        self.refCS = self.tempCS

        # Dictionary keeping track of all node paths manipulated so far
        self.nodePathDict = {}
        self.nodePathDict['camera'] = base.direct.camera
        self.nodePathDict['widget'] = base.direct.widget
        self.nodePathNames = ['camera', 'widget', 'selected']

        self.refNodePathDict = {}
        self.refNodePathDict['parent'] = self['nodePath'].getParent()
        self.refNodePathDict['render'] = render
        self.refNodePathDict['camera'] = base.direct.camera
        self.refNodePathDict['widget'] = base.direct.widget
        self.refNodePathNames = ['parent', 'self', 'render',
                                 'camera', 'widget', 'selected']

        # Initial state
        self.initPos = Vec3(0)
        self.initHpr = Vec3(0)
        self.initScale = Vec3(1)
        self.deltaHpr = Vec3(0)

        # Offset for orbital mode
        self.posOffset = Vec3(0)

        # Set up event hooks
        self.undoEvents = [('DIRECT_undo', self.undoHook),
                           ('DIRECT_pushUndo', self.pushUndoHook),
                           ('DIRECT_undoListEmpty', self.undoListEmptyHook),
                           ('DIRECT_redo', self.redoHook),
                           ('DIRECT_pushRedo', self.pushRedoHook),
                           ('DIRECT_redoListEmpty', self.redoListEmptyHook)]
        for event, method in self.undoEvents:
            self.accept(event, method)

        # Init movement mode
        self.movementMode = 'Relative To:'

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        interior['relief'] = FLAT
        # Add placer commands to menubar
        self.menuBar.addmenu('Placer', 'Placer Panel Operations')
        self.menuBar.addmenuitem('Placer', 'command',
                            'Zero Node Path',
                            label = 'Zero All',
                            command = self.zeroAll)
        self.menuBar.addmenuitem('Placer', 'command',
                            'Reset Node Path',
                            label = 'Reset All',
                            command = self.resetAll)
        self.menuBar.addmenuitem('Placer', 'command',
                            'Print Node Path Info',
                            label = 'Print Info',
                            command = self.printNodePathInfo)
        self.menuBar.addmenuitem(
            'Placer', 'command',
            'Toggle widget visability',
            label = 'Toggle Widget Vis',
            command = base.direct.toggleWidgetVis)
        self.menuBar.addmenuitem(
            'Placer', 'command',
            'Toggle widget manipulation mode',
            label = 'Toggle Widget Mode',
            command = base.direct.manipulationControl.toggleObjectHandlesMode)

        # Get a handle to the menu frame
        menuFrame = self.menuFrame
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
        self.nodePathMenu.pack(side = 'left', fill = 'x', expand = 1)
        self.bind(self.nodePathMenu, 'Select node path to manipulate')

        modeMenu = Pmw.OptionMenu(menuFrame,
                                  items = ('Relative To:',
                                           'Orbit:'),
                                  initialitem = 'Relative To:',
                                  command = self.setMovementMode,
                                  menubutton_width = 8)
        modeMenu.pack(side = 'left', expand = 0)
        self.bind(modeMenu, 'Select manipulation mode')

        self.refNodePathMenu = Pmw.ComboBox(
            menuFrame, entry_width = 16,
            selectioncommand = self.selectRefNodePathNamed,
            scrolledlist_items = self.refNodePathNames)
        self.refNodePathMenu.selectitem('parent')
        self.refNodePathMenuEntry = (
            self.refNodePathMenu.component('entryfield_entry'))
        self.refNodePathMenu.pack(side = 'left', fill = 'x', expand = 1)
        self.bind(self.refNodePathMenu, 'Select relative node path')

        self.undoButton = Button(menuFrame, text = 'Undo',
                                 command = base.direct.undo)
        if base.direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = 'left', expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(menuFrame, text = 'Redo',
                                 command = base.direct.redo)
        if base.direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = 'left', expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # Create and pack the Pos Controls
        posGroup = Pmw.Group(interior,
                             tag_pyclass = Menubutton,
                             tag_text = 'Position',
                             tag_font=('MSSansSerif', 14),
                             tag_activebackground = '#909090',
                             ring_relief = RIDGE)
        posMenubutton = posGroup.component('tag')
        self.bind(posMenubutton, 'Position menu operations')
        posMenu = Menu(posMenubutton, tearoff = 0)
        posMenu.add_command(label = 'Set to zero', command = self.zeroPos)
        posMenu.add_command(label = 'Reset initial',
                            command = self.resetPos)
        posMenubutton['menu'] = posMenu
        posGroup.pack(side='left', fill = 'both', expand = 1)
        posInterior = posGroup.interior()

        # Create the dials
        self.posX = self.createcomponent('posX', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'X', relief = FLAT,
                                         value = 0.0,
                                         label_foreground = 'Red')
        self.posX['commandData'] = ['x']
        self.posX['preCallback'] = self.xformStart
        self.posX['postCallback'] = self.xformStop
        self.posX['callbackData'] = ['x']
        self.posX.pack(expand=1, fill='both')

        self.posY = self.createcomponent('posY', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Y', relief = FLAT,
                                         value = 0.0,
                                         label_foreground = '#00A000')
        self.posY['commandData'] = ['y']
        self.posY['preCallback'] = self.xformStart
        self.posY['postCallback'] = self.xformStop
        self.posY['callbackData'] = ['y']
        self.posY.pack(expand=1, fill='both')

        self.posZ = self.createcomponent('posZ', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Z', relief = FLAT,
                                         value = 0.0,
                                         label_foreground = 'Blue')
        self.posZ['commandData'] = ['z']
        self.posZ['preCallback'] = self.xformStart
        self.posZ['postCallback'] = self.xformStop
        self.posZ['callbackData'] = ['z']
        self.posZ.pack(expand=1, fill='both')

        # Create and pack the Hpr Controls
        hprGroup = Pmw.Group(interior,
                             tag_pyclass = Menubutton,
                             tag_text = 'Orientation',
                             tag_font=('MSSansSerif', 14),
                             tag_activebackground = '#909090',
                             ring_relief = RIDGE)
        hprMenubutton = hprGroup.component('tag')
        self.bind(hprMenubutton, 'Orientation menu operations')
        hprMenu = Menu(hprMenubutton, tearoff = 0)
        hprMenu.add_command(label = 'Set to zero', command = self.zeroHpr)
        hprMenu.add_command(label = 'Reset initial', command = self.resetHpr)
        hprMenubutton['menu'] = hprMenu
        hprGroup.pack(side='left', fill = 'both', expand = 1)
        hprInterior = hprGroup.interior()

        # Create the dials
        self.hprH = self.createcomponent('hprH', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'H', value = 0.0,
                                         relief = FLAT,
                                         label_foreground = 'blue')
        self.hprH['commandData'] = ['h']
        self.hprH['preCallback'] = self.xformStart
        self.hprH['postCallback'] = self.xformStop
        self.hprH['callbackData'] = ['h']
        self.hprH.pack(expand=1, fill='both')

        self.hprP = self.createcomponent('hprP', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'P', value = 0.0,
                                         relief = FLAT,
                                         label_foreground = 'red')
        self.hprP['commandData'] = ['p']
        self.hprP['preCallback'] = self.xformStart
        self.hprP['postCallback'] = self.xformStop
        self.hprP['callbackData'] = ['p']
        self.hprP.pack(expand=1, fill='both')

        self.hprR = self.createcomponent('hprR', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'R', value = 0.0,
                                         relief = FLAT,
                                         label_foreground = '#00A000')
        self.hprR['commandData'] = ['r']
        self.hprR['preCallback'] = self.xformStart
        self.hprR['postCallback'] = self.xformStop
        self.hprR['callbackData'] = ['r']
        self.hprR.pack(expand=1, fill='both')

        # Create and pack the Scale Controls
        # The available scaling modes
        self.scalingMode = StringVar()
        self.scalingMode.set('Scale Uniform')
        # The scaling widgets
        scaleGroup = Pmw.Group(interior,
                               tag_text = 'Scale Uniform',
                               tag_pyclass = Menubutton,
                               tag_font=('MSSansSerif', 14),
                               tag_activebackground = '#909090',
                               ring_relief = RIDGE)
        self.scaleMenubutton = scaleGroup.component('tag')
        self.bind(self.scaleMenubutton, 'Scale menu operations')
        self.scaleMenubutton['textvariable'] = self.scalingMode

        # Scaling menu
        scaleMenu = Menu(self.scaleMenubutton, tearoff = 0)
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
        scaleGroup.pack(side='left', fill = 'both', expand = 1)
        scaleInterior = scaleGroup.interior()

        # Create the dials
        self.scaleX = self.createcomponent('scaleX', (), None,
                                           Floater.Floater, (scaleInterior,),
                                           text = 'X Scale',
                                           relief = FLAT,
                                           min = 0.0001, value = 1.0,
                                           resetValue = 1.0,
                                           label_foreground = 'Red')
        self.scaleX['commandData'] = ['sx']
        self.scaleX['callbackData'] = ['sx']
        self.scaleX['preCallback'] = self.xformStart
        self.scaleX['postCallback'] = self.xformStop
        self.scaleX.pack(expand=1, fill='both')

        self.scaleY = self.createcomponent('scaleY', (), None,
                                           Floater.Floater, (scaleInterior,),
                                           text = 'Y Scale',
                                           relief = FLAT,
                                           min = 0.0001, value = 1.0,
                                           resetValue = 1.0,
                                           label_foreground = '#00A000')
        self.scaleY['commandData'] = ['sy']
        self.scaleY['callbackData'] = ['sy']
        self.scaleY['preCallback'] = self.xformStart
        self.scaleY['postCallback'] = self.xformStop
        self.scaleY.pack(expand=1, fill='both')

        self.scaleZ = self.createcomponent('scaleZ', (), None,
                                           Floater.Floater, (scaleInterior,),
                                           text = 'Z Scale',
                                           relief = FLAT,
                                           min = 0.0001, value = 1.0,
                                           resetValue = 1.0,
                                           label_foreground = 'Blue')
        self.scaleZ['commandData'] = ['sz']
        self.scaleZ['callbackData'] = ['sz']
        self.scaleZ['preCallback'] = self.xformStart
        self.scaleZ['postCallback'] = self.xformStop
        self.scaleZ.pack(expand=1, fill='both')

        # Make sure appropriate labels are showing
        self.setMovementMode('Relative To:')
        # Set up placer for inital node path
        self.selectNodePathNamed('init')
        self.selectRefNodePathNamed('parent')
        # Update place to reflect initial state
        self.updatePlacer()
        # Now that you're done setting up, attach commands
        self.posX['command'] = self.xform
        self.posY['command'] = self.xform
        self.posZ['command'] = self.xform
        self.hprH['command'] = self.xform
        self.hprP['command'] = self.xform
        self.hprR['command'] = self.xform
        self.scaleX['command'] = self.xform
        self.scaleY['command'] = self.xform
        self.scaleZ['command'] = self.xform


    ### WIDGET OPERATIONS ###
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

    def setScalingMode(self):
        if self['nodePath']:
            scale = self['nodePath'].getScale()
            if ((scale[0] != scale[1]) or
                (scale[0] != scale[2]) or
                (scale[1] != scale[2])):
                self.scalingMode.set('Scale Free')

    def selectNodePathNamed(self, name):
        nodePath = None
        if name == 'init':
            nodePath = self['nodePath']
            # Add Combo box entry for the initial node path
            self.addNodePath(nodePath)
        elif name == 'selected':
            nodePath = base.direct.selected.last
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
            else:
                if name == 'widget':
                    # Record relationship between selected nodes and widget
                    base.direct.selected.getWrtAll()
        # Update active node path
        self.setActiveNodePath(nodePath)

    def setActiveNodePath(self, nodePath):
        self['nodePath'] = nodePath
        if self['nodePath']:
            self.nodePathMenuEntry.configure(
                background = self.nodePathMenuBG)
            # Check to see if node path and ref node path are the same
            if ((self.refCS != None) and
                (self.refCS.id() == self['nodePath'].id())):
                # Yes they are, use temp CS as ref
                # This calls updatePlacer
                self.setReferenceNodePath(self.tempCS)
                # update listbox accordingly
                self.refNodePathMenu.selectitem('parent')
            else:
                # Record initial value and initialize the widgets
                self.updatePlacer()
            # Record initial position
            self.updateResetValues(self['nodePath'])
            # Set scaling mode based on node path's current scale
            self.setScalingMode()
        else:
            # Flash entry
            self.nodePathMenuEntry.configure(background = 'Pink')

    def selectRefNodePathNamed(self, name):
        nodePath = None
        if name == 'self':
            nodePath = self.tempCS
        elif name == 'selected':
            nodePath = base.direct.selected.last
            # Add Combo box entry for this selected object
            self.addRefNodePath(nodePath)
        elif name == 'parent':
            nodePath = self['nodePath'].getParent()
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
        # Check to see if node path and ref node path are the same
        if (nodePath != None) and (nodePath.id() == self['nodePath'].id()):
            # Yes they are, use temp CS and update listbox accordingly
            nodePath = self.tempCS
            self.refNodePathMenu.selectitem('parent')
        # Update ref node path
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
            dictName = name + '-' + `nodePath.id()`
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
        if (np != None) and isinstance(np, NodePath):
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
        self.tempCS.setPosHpr(self['nodePath'], 0, 0, 0, 0, 0, 0)
        # Orbit CS
        # At reference
        self.orbitFromCS.setPos(self.refCS, 0, 0, 0)
        # But aligned with target
        self.orbitFromCS.setHpr(self['nodePath'], 0, 0, 0)
        # Also update to CS
        self.orbitToCS.setPosHpr(self.orbitFromCS, 0, 0, 0, 0, 0, 0)
        # Get offset from origin
        self.posOffset.assign(self['nodePath'].getPos(self.orbitFromCS))

    ### NODE PATH TRANSFORMATION OPERATIONS ###
    def xform(self, value, axis):
        if axis in ['sx', 'sy', 'sz']:
            self.xformScale(value, axis)
        elif self.movementMode == 'Relative To:':
            self.xformRelative(value, axis)
        elif self.movementMode == 'Orbit:':
            self.xformOrbit(value, axis)
        if self.nodePathMenu.get() == 'widget':
            if base.direct.manipulationControl.fSetCoa:
                # Update coa based on current widget position
                base.direct.selected.last.mCoa2Dnp.assign(
                    base.direct.widget.getMat(base.direct.selected.last))
            else:
                # Move the objects with the widget
                base.direct.selected.moveWrtWidgetAll()

    def xformStart(self, data):
        # Record undo point
        self.pushUndo()
        # If moving widget kill follow task and update wrts
        if self.nodePathMenu.get() == 'widget':
            taskMgr.remove('followSelectedNodePath')
            # Record relationship between selected nodes and widget
            base.direct.selected.getWrtAll()
        # Record initial state
        self.deltaHpr = self['nodePath'].getHpr(self.refCS)
        # Update placer to reflect new state
        self.updatePlacer()

    def xformStop(self, data):
        # Throw event to signal manipulation done
        # Send nodepath as a list
        messenger.send('DIRECT_manipulateObjectCleanup', [[self['nodePath']]])
        # Update placer to reflect new state
        self.updatePlacer()
        # If moving widget restart follow task
        if self.nodePathMenu.get() == 'widget':
            # Restart followSelectedNodePath task
            base.direct.manipulationControl.spawnFollowSelectedNodePathTask()

    def xformRelative(self, value, axis):
        nodePath = self['nodePath']
        if (nodePath != None) and (self.refCS != None):
            if axis == 'x':
                nodePath.setX(self.refCS, value)
            elif axis == 'y':
                nodePath.setY(self.refCS, value)
            elif axis == 'z':
                nodePath.setZ(self.refCS, value)
            else:
                if axis == 'h':
                    self.deltaHpr.setX(value)
                elif axis == 'p':
                    self.deltaHpr.setY(value)
                elif axis == 'r':
                    self.deltaHpr.setZ(value)
                # Put node path at new hpr
                nodePath.setHpr(self.refCS, self.deltaHpr)

    def xformOrbit(self, value, axis):
        nodePath = self['nodePath']
        if ((nodePath != None) and (self.refCS != None) and
            (self.orbitFromCS != None) and (self.orbitToCS != None)):
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
                scale.set(value, value, value)
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

    def zeroAll(self):
        self.xformStart(None)
        self.updatePosWidgets(ZERO_VEC)
        self.updateHprWidgets(ZERO_VEC)
        self.updateScaleWidgets(UNIT_VEC)
        self.xformStop(None)

    def zeroPos(self):
        self.xformStart(None)
        self.updatePosWidgets(ZERO_VEC)
        self.xformStop(None)

    def zeroHpr(self):
        self.xformStart(None)
        self.updateHprWidgets(ZERO_VEC)
        self.xformStop(None)

    def unitScale(self):
        self.xformStart(None)
        self.updateScaleWidgets(UNIT_VEC)
        self.xformStop(None)

    def updateResetValues(self, nodePath):
        self.initPos.assign(nodePath.getPos())
        self.posX['resetValue'] = self.initPos[0]
        self.posY['resetValue'] = self.initPos[1]
        self.posZ['resetValue'] = self.initPos[2]
        self.initHpr.assign(nodePath.getHpr())
        self.hprH['resetValue'] = self.initHpr[0]
        self.hprP['resetValue'] = self.initHpr[1]
        self.hprR['resetValue'] = self.initHpr[2]
        self.initScale.assign(nodePath.getScale())
        self.scaleX['resetValue'] = self.initScale[0]
        self.scaleY['resetValue'] = self.initScale[1]
        self.scaleZ['resetValue'] = self.initScale[2]

    def resetAll(self):
        if self['nodePath']:
            self.xformStart(None)
            self['nodePath'].setPosHprScale(
                self.initPos, self.initHpr, self.initScale)
            self.xformStop(None)

    def resetPos(self):
        if self['nodePath']:
            self.xformStart(None)
            self['nodePath'].setPos(self.initPos)
            self.xformStop(None)

    def resetHpr(self):
        if self['nodePath']:
            self.xformStart(None)
            self['nodePath'].setHpr(self.initHpr)
            self.xformStop(None)

    def resetScale(self):
        if self['nodePath']:
            self.xformStart(None)
            self['nodePath'].setScale(self.initScale)
            self.xformStop(None)

    def pushUndo(self, fResetRedo = 1):
        base.direct.pushUndo([self['nodePath']])

    def undoHook(self, nodePathList = []):
        # Reflect new changes
        self.updatePlacer()

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        base.direct.pushRedo([self['nodePath']])

    def redoHook(self, nodePathList = []):
        # Reflect new changes
        self.updatePlacer()

    def pushRedoHook(self):
        # Make sure button is reactivated
        self.redoButton.configure(state = 'normal')

    def redoListEmptyHook(self):
        # Make sure button is deactivated
        self.redoButton.configure(state = 'disabled')

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

    def onDestroy(self, event):
        # Remove hooks
        for event, method in self.undoEvents:
            self.ignore(event)
        self.tempCS.removeNode()
        self.orbitFromCS.removeNode()
        self.orbitToCS.removeNode()

def place(nodePath):
    return Placer(nodePath = nodePath)

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = Pmw.initialise()
    widget = Placer()
