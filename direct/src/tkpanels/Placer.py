"DIRECT Nine DoF Placer demonstration"

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
import Pmw
import Dial
import Floater

class Placer(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       'Placer Panel',     None),
            ('nodePath',    None,               None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaToplevel.__init__(self, parent)

        # Initialize state
        self.hotPt = render.attachNewNode(NamedNode('hotPt'))
        # Dictionary keeping track of all node paths manipulated so far
        self.nodePathDict = {}
        self.nodePathDict['camera'] = camera
        self.nodePathDict['hot point'] = self.hotPt
        self.nodePathNames = ['selected', 'hot point', 'camera']

        # Get initial state
        self.initPos = Vec3(0)
        self.initHpr = Vec3(0)
        self.initScale = Vec3(1)

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
        menuBar.addcascademenu('Placer', 'Axis',
                               'Control axis visibility',
                               tearoff = 1)
        self.axisViz = StringVar()
        self.axisViz.set('Show Axis')
        menuBar.component('Axis-menu').add_radiobutton(
            label = 'Show Axis',
            variable = self.axisViz,
            value = 'Show Axis',
            command = lambda s = self: s._updateAxisViz())
        menuBar.component('Axis-menu').add_radiobutton(
            label = 'Hide Axis',
            variable = self.axisViz,
            value = 'Hide Axis',
            command = lambda s = self: s._updateAxisViz())
        menuBar.component('Axis-menu').add_radiobutton(
            label = 'Auto Axis',
            variable = self.axisViz,
            value = 'Auto Axis',
            command = lambda s = self: s._updateAxisViz())
                                                           
        menuBar.addmenuitem('Placer', 'command',
                            'Exit Placer Panel',
                            label = 'Exit',
                            command = self.destroy)
        menuBar.addmenu('NodePath', 'Node Path Operations')
        menuBar.addmenuitem('NodePath', 'command',
                            'Undo Pos/Hpr/Scale',
                            label = 'Undo All',
                            command = self._undoAll)
        menuBar.addmenuitem('NodePath', 'command',
                            'Redo Pos/Hpr/Scale',
                            label = 'Redo All',
                            command = self._redoAll)
        menuBar.addmenuitem('NodePath', 'command',
                            'Reset Node Path',
                            label = 'Reset All',
                            command = self._resetAll)
        menuBar.addmenuitem('NodePath', 'command',
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
            menuFrame,
            labelpos = W,
            label_text = 'Node Path:',
            entry_width = 12,
            selectioncommand = self.selectNodePathNamed,
            scrolledlist_items = self.nodePathNames)
        self.nodePathMenu.selectitem('selected')
        self.nodePathMenuEntry = (
            self.nodePathMenu.component('entryfield_entry'))
        self.nodePathMenuBG = (
            self.nodePathMenuEntry.configure('background')[3])
        self.nodePathMenu.pack(side = 'left', expand = 0)

        mode = StringVar()
        mode.set('Absolute')
        modeMenu = Pmw.OptionMenu(menuFrame,
                                  menubutton_textvariable=mode,
                                  items = ('Drive', 'Orbit',
                                           'Absolute', 'Relative'),
                                  command = self._updateDialLabels,
                                  menubutton_width = 8)
        modeMenu.pack(side = 'left', expand = 0)
        
        self.wrtMenu = Pmw.ComboBox(menuFrame,
                               labelpos = W,
                               label_text = 'WRT:',
                               entry_width = 12,
                               selectioncommand = self.selectWrtNodePathNamed,
                               scrolledlist_items = ('render',
                                                     'selected',
                                                     'camera'))
        self.wrtMenu.selectitem('render')
        self.wrtMenu.pack(side = 'left', expand = 0)

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
	posMenu.add_command(label = 'Undo', command = self._undoPos)
	posMenu.add_command(label = 'Redo', command = self._redoPos)
	posMenu.add_command(label = 'Set to zero', command = self._zeroPos)
	posMenu.add_command(label = 'Restore initial', command = self._resetPos)
	posMenubutton['menu'] = posMenu
	posGroup.pack(side='left',fill = 'both', expand = 1)
        posInterior = posGroup.interior()

        # Create the dials
	self.posX = self.createcomponent('posX', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'X',
                                         initialValue = self.initPos[0],
                                         label_foreground = 'Red')
        self.posX['command'] = self.setX
        self.posX.pack(expand=1,fill='x')
        
	self.posY = self.createcomponent('posY', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Y',
                                         initialValue = self.initPos[1],
                                         label_foreground = '#00A000')
        self.posY['command'] = self.setY
        self.posY.pack(expand=1,fill='x')
        
	self.posZ = self.createcomponent('posZ', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Z',
                                         initialValue = self.initPos[2],
                                         label_foreground = 'Blue')
        self.posZ['command'] = self.setZ
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
	hprMenu.add_command(label = 'Undo', command = self._undoHpr)
	hprMenu.add_command(label = 'Redo', command = self._redoHpr)
	hprMenu.add_command(label = 'Set to zero', command = self._zeroHpr)
	hprMenu.add_command(label = 'Restore initial', command = self._resetHpr)
	hprMenubutton['menu'] = hprMenu
	hprGroup.pack(side='left',fill = 'both', expand = 1)
        hprInterior = hprGroup.interior()
        
	# Create the dials
	self.hprH = self.createcomponent('hprH', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'H', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = self.initHpr[0],
                                         label_foreground = 'blue')
        self.hprH['command'] = self.setH
        self.hprH.pack(expand=1,fill='x')
        
	self.hprP = self.createcomponent('hprP', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'P', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = self.initHpr[1],
                                         label_foreground = 'red')
        self.hprP['command'] = self.setP
        self.hprP.pack(expand=1,fill='x')
        
	self.hprR = self.createcomponent('hprR', (), None,
                                         Dial.Dial, (hprInterior,),
                                         text = 'R', fRollover = 0,
                                         max = 360.0, numTicks = 12,
                                         initialValue = self.initHpr[2],
                                         label_foreground = '#00A000')
        self.hprR['command'] = self.setR
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
	scaleMenu.add_command(label = 'Undo', command = self._undoScale)
	scaleMenu.add_command(label = 'Redo', command = self._redoScale)
	scaleMenu.add_command(label = 'Set to unity',
                              command = self._unitScale)
	scaleMenu.add_command(label = 'Restore initial',
                              command = self._resetScale)
        scaleMenu.add_cascade(label = 'Scaling mode...',
                              menu = scaleModeMenu)
	scaleMenubutton['menu'] = scaleMenu
	scaleGroup.pack(side='left',fill = 'both', expand = 1)
        scaleInterior = scaleGroup.interior()
        
	# Create the dials
	self.scaleX = self.createcomponent('scaleX', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'X Scale',
                                           initialValue = self.initScale[0],
                                           label_foreground = 'Red')
        self.scaleX['command'] = self.setSx
        self.scaleX.pack(expand=1,fill='x')
        
	self.scaleY = self.createcomponent('scaleY', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'Y Scale',
                                           initialValue = self.initScale[1],
                                           label_foreground = '#00A000')
        self.scaleY['command'] = self.setSy
        self.scaleY.pack(expand=1,fill='x')
        
	self.scaleZ = self.createcomponent('scaleZ', (), None,
                                           Dial.Dial, (scaleInterior,),
                                           text = 'Z Scale',
                                           initialValue = self.initScale[2],
                                           label_foreground = 'Blue')
        self.scaleZ['command'] = self.setSz
        self.scaleZ.pack(expand=1,fill='x')

        # Make sure appropriate labels are showing
        self._updateDialLabels('Absolute')

        # Initialize the widgets
        self._initAll()

        # Make sure input variables processed 
        self.initialiseoptions(Placer)

    def setX(self, val):
        self['nodePath'].setX(val)
    def setY(self, val):
        self['nodePath'].setY(val)
    def setZ(self, val):
        self['nodePath'].setZ(val)

    def setH(self, val):
        self['nodePath'].setH(val)
    def setP(self, val):
        self['nodePath'].setP(val)
    def setR(self, val):
        self['nodePath'].setR(val)

    def setSx(self, val):
        self['nodePath'].setSx(val)
    def setSy(self, val):
        self['nodePath'].setSy(val)
    def setSz(self, val):
        self['nodePath'].setSz(val)

    def addNodePath(self, nodePath):
        # Get node path's name
        name = nodePath.getName()
        # Generate a unique name for the dict
        dictName = name + '-' + `nodePath.id().this`
        if not self.nodePathDict.has_key(dictName):
            # Update combo box to include new item
            self.nodePathNames.append(dictName)
            listbox = self.nodePathMenu.component('scrolledlist')
            listbox.setlist(self.nodePathNames)
            # Add new item to dictionary
            self.nodePathDict[dictName] = nodePath
        self.nodePathMenu.selectitem(dictName)

    def manip(self, nodePath):
        self['nodePath'] = nodePath
        # Record initial value and initialize the widgets
        self._initAll()
        
    def selectNodePathNamed(self, name):
        print 'Selected Node Path: ' + name
        nodePath = None
        if name == 'selected':
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
        if nodePath:
            self.manip(nodePath)
            self.nodePathMenuEntry.configure(
                background = self.nodePathMenuBG)
        else:
            # Flash entry
            self.nodePathMenuEntry.configure(background = 'Pink')

    def selectWrtNodePathNamed(self, name):
        print 'Selected Wrt Node Path: ' + name

    def printNodePathInfo(self):
        print 'Print Node Path info here'

    def _updateAxisViz(self):
        self.updateAxisViz(self.axisViz.get())

    def updateAxisViz(self, mode):
        print mode

    def _undoPos(self):
        print 'undo pos'

    def _redoPos(self):
        print 'redo pos'

    def _initPos(self, pos):
        self.posX.set(pos[0])
        self.posY.set(pos[1])
        self.posZ.set(pos[2])

    def _resetPos(self):
        self._initPos(self.initPos)

    def _zeroPos(self):
        self.posX.set(0.0)
        self.posY.set(0.0)
        self.posZ.set(0.0)

    def _undoHpr(self):
        print 'undo hpr'

    def _redoHpr(self):
        print 'redo hpr'

    def _initHpr(self, hpr):
        self.hprH.set(hpr[0])
        self.hprP.set(hpr[1])
        self.hprR.set(hpr[2])

    def _resetHpr(self):
        self._initHpr(self.initHpr)

    def _zeroHpr(self):
        self.hprH.set(0.0)
        self.hprP.set(0.0)
        self.hprR.set(0.0)

    def _initScale(self, scale):
        self.scaleX.set(scale[0])
        self.scaleY.set(scale[1])
        self.scaleZ.set(scale[2])

    def _resetScale(self):
        self._initScale(self.initScale)

    def _undoScale(self):
        print 'undo scale'

    def _redoScale(self):
        print 'redo scale'

    def _unitScale(self):
        self.scaleX.set(1.0)
        self.scaleY.set(1.0)
        self.scaleZ.set(1.0)

    def _undoAll(self):
        self._undoPos()
        self._undoHpr()
        self._undoScale()

    def _redoAll(self):
        self._redoPos()
        self._redoHpr()
        self._redoScale()

    def _initAll(self):
        if self['nodePath']:
            np = self['nodePath']
            if isinstance(np, NodePath):
                self.initPos.assign(np.getPos())
                self.initHpr.assign(np.getHpr())
                self.initScale.assign(np.getScale())
        self._initPos(self.initPos)
        self._initHpr(self.initHpr)
        self._initScale(self.initScale)

    def _resetAll(self):
        self._resetPos()
        self._resetHpr()
        self._resetScale()

    def _updateDialLabels(self, movementMode):
        namePrefix = ''
        self.movementMode = movementMode
        if (movementMode == 'Drive'):
            namePrefix = 'Drive delta '
        elif (movementMode == 'Orbit'):
            namePrefix = 'Orbit '
        elif (movementMode == 'Absolute'):
            namePrefix = 'Absolute '
        elif (movementMode == 'Relative'):
            namePrefix = 'Relative '

        if(movementMode == 'Relative'):
            self.wrtMenu.configure(entry_foreground = 'Black')
            self.wrtMenu.configure(entry_background = 'SystemWindow')
        else:
            self.wrtMenu.configure(entry_foreground = 'gray50')
            self.wrtMenu.configure(entry_background = '#E0E0E0')

        self.posX['text'] = namePrefix + 'X'
        self.posY['text'] = namePrefix + 'Y'
        self.posZ['text'] = namePrefix + 'Z'

        if (movementMode == 'Orbit'):
            namePrefix = 'Orbit delta '

        self.hprH['text'] = namePrefix + 'H'
        self.hprP['text'] = namePrefix + 'P'
        self.hprR['text'] = namePrefix + 'R'

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

