""" DIRECT Session Main panel """

__all__ = ['DirectSessionPanel']

# Import Tkinter, Pmw, and the dial code
from direct.showbase.TkGlobal import *
from direct.tkwidgets.AppShell import *
from Tkinter import *
from pandac.PandaModules import *
import Pmw, string
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
from direct.tkwidgets import VectorWidgets
from direct.tkwidgets import SceneGraphExplorer
from TaskManagerPanel import TaskManagerWidget
from direct.tkwidgets import MemoryExplorer

"""
Possible to add:
messenger.clear?
popup panels
taskMgr page
"""

class DirectSessionPanel(AppShell):
    # Override class variables here
    appname = 'Direct Session Panel'
    frameWidth      = 600
    frameHeight     = 365
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        # Call superclass initialization function
        AppShell.__init__(self, parent)

        # Active light
        if len(base.direct.lights) > 0:
            name = base.direct.lights.getNameList()[0]
            self.lightMenu.selectitem(name)
            self.selectLightNamed(name)
        else:
            self.activeLight = None
        # Active display region
        self.drMenu.selectitem('Display Region 0')
        self.selectDisplayRegionNamed('Display Region 0')
        # Make sure we've got valid initial values
        self.updateInfo()

        self.initialiseoptions(DirectSessionPanel)

    def appInit(self):
        # Initialize state
        # Dictionary keeping track of all node paths selected so far
        self.nodePathDict = {}
        self.nodePathDict['widget'] = base.direct.widget
        self.nodePathNames = ['widget']

        # Dictionary keeping track of all jb node paths selected so far
        self.jbNodePathDict = {}
        self.jbNodePathDict['none'] = 'No Node Path'
        self.jbNodePathDict['widget'] = base.direct.widget
        self.jbNodePathDict['camera'] = base.direct.camera
        self.jbNodePathNames = ['camera', 'selected', 'none']

        # Set up event hooks
        self.actionEvents = [
            ('DIRECT_undo', self.undoHook),
            ('DIRECT_pushUndo', self.pushUndoHook),
            ('DIRECT_undoListEmpty', self.undoListEmptyHook),
            ('DIRECT_redo', self.redoHook),
            ('DIRECT_pushRedo', self.pushRedoHook),
            ('DIRECT_redoListEmpty', self.redoListEmptyHook),
            ('DIRECT_selectedNodePath', self.selectedNodePathHook),
            ('DIRECT_addLight', self.addLight),
            ]
        for event, method in self.actionEvents:
            self.accept(event, method)

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        # Add placer commands to menubar
        self.menuBar.addmenu('DIRECT', 'Direct Session Panel Operations')

        self.directEnabled = BooleanVar()
        self.directEnabled.set(1)
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Enabled',
                                 label = 'Enable',
                                 variable = self.directEnabled,
                                 command = self.toggleDirect)

        self.directGridEnabled = BooleanVar()
        self.directGridEnabled.set(base.direct.grid.isEnabled())
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Grid Enabled',
                                 label = 'Enable Grid',
                                 variable = self.directGridEnabled,
                                 command = self.toggleDirectGrid)

        self.menuBar.addmenuitem('DIRECT', 'command',
                                 'Toggle Object Handles Visability',
                                 label = 'Toggle Widget Viz',
                                 command = base.direct.toggleWidgetVis)

        self.menuBar.addmenuitem(
            'DIRECT', 'command',
            'Toggle Widget Move/COA Mode',
            label = 'Toggle Widget Mode',
            command = base.direct.manipulationControl.toggleObjectHandlesMode)

        self.directWidgetOnTop = BooleanVar()
        self.directWidgetOnTop.set(0)
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Widget On Top',
                                 label = 'Widget On Top',
                                 variable = self.directWidgetOnTop,
                                 command = self.toggleWidgetOnTop)

        self.menuBar.addmenuitem('DIRECT', 'command',
                                 'Deselect All',
                                 label = 'Deselect All',
                                 command = base.direct.deselectAll)

        # Get a handle to the menu frame
        menuFrame = self.menuFrame

        # Widget to select node paths (and display list of selected node paths)
        self.nodePathMenu = Pmw.ComboBox(
            menuFrame, labelpos = W, label_text = 'DIRECT Select:',
            entry_width = 20,
            selectioncommand = self.selectNodePathNamed,
            scrolledlist_items = self.nodePathNames)
        self.nodePathMenu.selectitem('widget')
        self.nodePathMenuEntry = (
            self.nodePathMenu.component('entryfield_entry'))
        self.nodePathMenuBG = (
            self.nodePathMenuEntry.configure('background')[3])
        self.nodePathMenu.pack(side = LEFT, fill = X, expand = 1)
        self.bind(self.nodePathMenu, 'Select node path to manipulate')

        self.undoButton = Button(menuFrame, text = 'Undo',
                                 command = base.direct.undo)
        if base.direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = LEFT, expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(menuFrame, text = 'Redo',
                                 command = base.direct.redo)
        if base.direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = LEFT, expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # The master frame for the dials
        mainFrame = Frame(interior)

        # Paned widget for dividing two halves
        framePane = Pmw.PanedWidget(mainFrame, orient = HORIZONTAL)
        sgeFrame = framePane.add('left', min = 250)
        notebookFrame = framePane.add('right', min = 300)

        # Scene Graph Explorer
        self.SGE = SceneGraphExplorer.SceneGraphExplorer(
            sgeFrame, nodePath = render,
            scrolledCanvas_hull_width = 250,
            scrolledCanvas_hull_height = 300)
        self.SGE.pack(fill = BOTH, expand = 0)
        sgeFrame.pack(side = LEFT, fill = 'both', expand = 0)

        # Create the notebook pages
        notebook = Pmw.NoteBook(notebookFrame)
        notebook.pack(fill = BOTH, expand = 1)
        self.createEnvPage(notebook.add('Environment'))
        self.createLightsPage(notebook.add('Lights'))
        self.createGridPage(notebook.add('Grid'))
        self.createDevicePage(notebook.add('Devices'))
        self.createTasksPage(notebook.add('Tasks'))
        self.createMemPage(notebook.add('Memory'))

        notebook.setnaturalsize()

        framePane.pack(expand = 1, fill = BOTH)
        mainFrame.pack(fill = 'both', expand = 1)
        
        # Put this here so it isn't called right away
        notebook['raisecommand'] = self.updateInfo
       
    def createEnvPage(self, envPage):
        bkgrdFrame = Frame(envPage, borderwidth = 2, relief = 'sunken')

        Label(bkgrdFrame, text = 'Background',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        self.backgroundColor = VectorWidgets.ColorEntry(
            bkgrdFrame, text = 'Background Color')
        self.backgroundColor['command'] = self.setBackgroundColorVec
        self.backgroundColor.pack(fill = X, expand = 0)
        self.bind(self.backgroundColor, 'Set background color')
        bkgrdFrame.pack(fill = BOTH, expand = 0)

        drFrame = Frame(envPage, borderwidth = 2, relief = 'sunken')
        Label(drFrame, text = 'Display Region',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        nameList = map(lambda x: 'Display Region ' + `x`,
                       range(len(base.direct.drList)))
        self.drMenu = Pmw.ComboBox(
            drFrame, labelpos = W, label_text = 'Display Region:',
            entry_width = 20,
            selectioncommand = self.selectDisplayRegionNamed,
            scrolledlist_items = nameList)
        self.drMenu.pack(fill = X, expand = 0)
        self.bind(self.drMenu, 'Select display region to configure')

        self.nearPlane = Floater.Floater(
            drFrame,
            text = 'Near Plane',
            min = 0.01)
        self.nearPlane['command'] = self.setNear
        self.nearPlane.pack(fill = X, expand = 0)
        self.bind(self.nearPlane, 'Set near plane distance')

        self.farPlane = Floater.Floater(
            drFrame,
            text = 'Far Plane',
            min = 0.01)
        self.farPlane['command'] = self.setFar
        self.farPlane.pack(fill = X, expand = 0)
        self.bind(self.farPlane, 'Set far plane distance')

        fovFrame = Frame(drFrame)
        fovFloaterFrame = Frame(fovFrame)
        self.hFov = Slider.Slider(
            fovFloaterFrame,
            text = 'Horizontal FOV',
            min = 0.01, max = 170.0)
        self.hFov['command'] = self.setHFov
        self.hFov.pack(fill = X, expand = 0)
        self.bind(self.hFov, 'Set horizontal field of view')

        self.vFov = Slider.Slider(
            fovFloaterFrame,
            text = 'Vertical FOV',
            min = 0.01, max = 170.0)
        self.vFov['command'] = self.setVFov
        self.vFov.pack(fill = X, expand = 0)
        self.bind(self.vFov, 'Set vertical field of view')
        fovFloaterFrame.pack(side = LEFT, fill = X, expand = 1)

        frame = Frame(fovFrame)
        self.lockedFov = BooleanVar()
        self.lockedFov.set(1)
        self.lockedFovButton = Checkbutton(
            frame,
            text = 'Locked',
            anchor = 'w', justify = LEFT,
            variable = self.lockedFov)
        self.lockedFovButton.pack(fill = X, expand = 0)

        self.resetFovButton = Button(
            frame,
            text = 'Reset',
            command = self.resetFov)
        self.resetFovButton.pack(fill = X, expand = 0)
        frame.pack(side = LEFT, fill = X, expand = 0)
        fovFrame.pack(fill = X, expand = 1)

        drFrame.pack(fill = BOTH, expand = 0)

        ## Render Style ##
        toggleFrame = Frame(envPage, borderwidth = 2, relief = 'sunken')
        Label(toggleFrame, text = 'Toggle Render Style',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.toggleBackfaceButton = Button(
            toggleFrame,
            text = 'Backface',
            command = base.toggleBackface)
        self.toggleBackfaceButton.pack(side = LEFT, fill = X, expand = 1)

        self.toggleLightsButton = Button(
            toggleFrame,
            text = 'Lights',
            command = base.direct.lights.toggle)
        self.toggleLightsButton.pack(side = LEFT, fill = X, expand = 1)

        self.toggleTextureButton = Button(
            toggleFrame,
            text = 'Texture',
            command = base.toggleTexture)
        self.toggleTextureButton.pack(side = LEFT, fill = X, expand = 1)

        self.toggleWireframeButton = Button(
            toggleFrame,
            text = 'Wireframe',
            command = base.toggleWireframe)
        self.toggleWireframeButton.pack(fill = X, expand = 1)
        toggleFrame.pack(side = LEFT, fill = X, expand = 1)

    def createLightsPage(self, lightsPage):
        # Lights #
        lightFrame = Frame(lightsPage, borderwidth = 2, relief = 'sunken')
        self.lightsButton = Menubutton(lightFrame, text = 'Lights',
                                       font=('MSSansSerif', 14, 'bold'),
                                       activebackground = '#909090')
        lightsMenu = Menu(self.lightsButton)
        lightsMenu.add_command(label = 'Add Ambient Light',
                            command = self.addAmbient)
        lightsMenu.add_command(label = 'Add Directional Light',
                            command = self.addDirectional)
        lightsMenu.add_command(label = 'Add Point Light',
                            command = self.addPoint)
        lightsMenu.add_command(label = 'Add Spotlight',
                            command = self.addSpot)

        self.lightsButton.pack(expand = 0)
        self.lightsButton['menu'] = lightsMenu

        # Notebook pages for light specific controls
        self.lightNotebook = Pmw.NoteBook(lightFrame, tabpos = None,
                                          borderwidth = 0)
        ambientPage = self.lightNotebook.add('Ambient')
        directionalPage = self.lightNotebook.add('Directional')
        pointPage = self.lightNotebook.add('Point')
        spotPage = self.lightNotebook.add('Spot')
        # Put this here so it isn't called right away
        self.lightNotebook['raisecommand'] = self.updateLightInfo

        # Main light switch
        mainSwitchFrame = Frame(lightFrame)
        Label(mainSwitchFrame,
              text = 'Lighting:').pack(side = LEFT, expand = 0)
        self.enableLights = BooleanVar()
        self.enableLightsButton = Checkbutton(
            mainSwitchFrame,
            text = 'Enabled/Disabled',
            variable = self.enableLights,
            command = self.toggleLights)
        self.enableLightsButton.pack(side = LEFT, fill = X, expand = 0)
        mainSwitchFrame.pack(fill = X, expand = 0)

        # Widget to select a light to configure
        nameList = base.direct.lights.getNameList()
        lightMenuFrame = Frame(lightFrame)

        self.lightMenu = Pmw.ComboBox(
            lightMenuFrame, labelpos = W, label_text = 'Light:',
            entry_width = 20,
            selectioncommand = self.selectLightNamed,
            scrolledlist_items = nameList)
        self.lightMenu.pack(side = LEFT, fill = X, expand = 0)
        self.bind(self.lightMenu, 'Select light to configure')

        self.lightActive = BooleanVar()
        self.lightActiveButton = Checkbutton(
            lightMenuFrame,
            text = 'On/Off',
            variable = self.lightActive,
            command = self.toggleActiveLight)
        self.lightActiveButton.pack(side = LEFT, fill = X, expand = 0)

        # Pack light menu
        lightMenuFrame.pack(fill = X, expand = 0, padx = 2)

        self.lightColor = VectorWidgets.ColorEntry(
            lightFrame, text = 'Light Color')
        self.lightColor['command'] = self.setLightColor
        self.lightColor.pack(fill = X, expand = 0, padx = 4)
        self.bind(self.lightColor, 'Set active light color')

        # Directional light controls
        self.dSpecularColor = VectorWidgets.ColorEntry(
            directionalPage, text = 'Specular Color')
        self.dSpecularColor['command'] = self.setSpecularColor
        self.dSpecularColor.pack(fill = X, expand = 0)
        self.bind(self.dSpecularColor,
                  'Set directional light specular color')

        # Point light controls
        self.pSpecularColor = VectorWidgets.ColorEntry(
            pointPage, text = 'Specular Color')
        self.pSpecularColor['command'] = self.setSpecularColor
        self.pSpecularColor.pack(fill = X, expand = 0)
        self.bind(self.pSpecularColor,
                  'Set point light specular color')

        self.pConstantAttenuation = Slider.Slider(
            pointPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, value = 1.0)
        self.pConstantAttenuation['command'] = self.setConstantAttenuation
        self.pConstantAttenuation.pack(fill = X, expand = 0)
        self.bind(self.pConstantAttenuation,
                  'Set point light constant attenuation')

        self.pLinearAttenuation = Slider.Slider(
            pointPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.pLinearAttenuation['command'] = self.setLinearAttenuation
        self.pLinearAttenuation.pack(fill = X, expand = 0)
        self.bind(self.pLinearAttenuation,
                  'Set point light linear attenuation')

        self.pQuadraticAttenuation = Slider.Slider(
            pointPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.pQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.pQuadraticAttenuation.pack(fill = X, expand = 0)
        self.bind(self.pQuadraticAttenuation,
                  'Set point light quadratic attenuation')

        # Spot light controls
        self.sSpecularColor = VectorWidgets.ColorEntry(
            spotPage, text = 'Specular Color')
        self.sSpecularColor['command'] = self.setSpecularColor
        self.sSpecularColor.pack(fill = X, expand = 0)
        self.bind(self.sSpecularColor,
                  'Set spot light specular color')

        self.sConstantAttenuation = Slider.Slider(
            spotPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, value = 1.0)
        self.sConstantAttenuation['command'] = self.setConstantAttenuation
        self.sConstantAttenuation.pack(fill = X, expand = 0)
        self.bind(self.sConstantAttenuation,
                  'Set spot light constant attenuation')

        self.sLinearAttenuation = Slider.Slider(
            spotPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.sLinearAttenuation['command'] = self.setLinearAttenuation
        self.sLinearAttenuation.pack(fill = X, expand = 0)
        self.bind(self.sLinearAttenuation,
                  'Set spot light linear attenuation')

        self.sQuadraticAttenuation = Slider.Slider(
            spotPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.sQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.sQuadraticAttenuation.pack(fill = X, expand = 0)
        self.bind(self.sQuadraticAttenuation,
                  'Set spot light quadratic attenuation')

        self.sExponent = Slider.Slider(
            spotPage,
            text = 'Exponent',
            min = 0.0, max = 1.0, value = 0.0)
        self.sExponent['command'] = self.setExponent
        self.sExponent.pack(fill = X, expand = 0)
        self.bind(self.sExponent,
                  'Set spot light exponent')

        # MRM: Add frustum controls

        self.lightNotebook.setnaturalsize()
        self.lightNotebook.pack(expand = 1, fill = BOTH)

        lightFrame.pack(expand = 1, fill = BOTH)


    def createGridPage(self, gridPage):
        Label(gridPage, text = 'Grid',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.enableGrid = BooleanVar()
        self.enableGridButton = Checkbutton(
            gridPage,
            text = 'Enabled/Disabled',
            anchor = 'w', justify = LEFT,
            variable = self.enableGrid,
            command = self.toggleGrid)
        self.enableGridButton.pack(fill = X, expand = 0)

        self.xyzSnap = BooleanVar()
        self.xyzSnapButton = Checkbutton(
            gridPage,
            text = 'XYZ Snap',
            anchor = 'w', justify = LEFT,
            variable = self.xyzSnap,
            command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(fill = X, expand = 0)

        self.hprSnap = BooleanVar()
        self.hprSnapButton = Checkbutton(
            gridPage,
            text = 'HPR Snap',
            anchor = 'w', justify = LEFT,
            variable = self.hprSnap,
            command = self.toggleHprSnap)
        self.hprSnapButton.pack(fill = X, expand = 0)

        self.gridSpacing = Floater.Floater(
            gridPage,
            text = 'Grid Spacing',
            min = 0.1,
            value = base.direct.grid.getGridSpacing())
        self.gridSpacing['command'] = base.direct.grid.setGridSpacing
        self.gridSpacing.pack(fill = X, expand = 0)

        self.gridSize = Floater.Floater(
            gridPage,
            text = 'Grid Size',
            min = 1.0,
            value = base.direct.grid.getGridSize())
        self.gridSize['command'] = base.direct.grid.setGridSize
        self.gridSize.pack(fill = X, expand = 0)

        self.gridSnapAngle = Dial.AngleDial(
            gridPage,
            text = 'Snap Angle',
            style = 'mini',
            value = base.direct.grid.getSnapAngle())
        self.gridSnapAngle['command'] = base.direct.grid.setSnapAngle
        self.gridSnapAngle.pack(fill = X, expand = 0)

    def createDevicePage(self, devicePage):
        Label(devicePage, text = 'DEVICES',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        if base.direct.joybox != None:
            joyboxFrame = Frame(devicePage, borderwidth = 2, relief = 'sunken')
            Label(joyboxFrame, text = 'Joybox',
                  font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
            self.enableJoybox = BooleanVar()
            self.enableJoybox.set(1)
            self.enableJoyboxButton = Checkbutton(
                joyboxFrame,
                text = 'Enabled/Disabled',
                anchor = 'w', justify = LEFT,
                variable = self.enableJoybox,
                command = self.toggleJoybox)
            self.enableJoyboxButton.pack(fill = X, expand = 0)
            joyboxFrame.pack(fill = X, expand = 0)

            self.jbModeMenu = Pmw.ComboBox(
                joyboxFrame, labelpos = W, label_text = 'Joybox Mode:',
                label_width = 16, entry_width = 20,
                selectioncommand = self.selectJBModeNamed,
                scrolledlist_items = ['Joe Mode', 'Drive Mode', 'Orbit Mode',
                                      'Look At Mode', 'Look Around Mode',
                                      'Walkthru Mode', 'Demo Mode',
                                      'HPRXYZ Mode'])
            self.jbModeMenu.selectitem('Joe Mode')
            self.jbModeMenu.pack(fill = X, expand = 1)

            self.jbNodePathMenu = Pmw.ComboBox(
                joyboxFrame, labelpos = W, label_text = 'Joybox Node Path:',
                label_width = 16, entry_width = 20,
                selectioncommand = self.selectJBNodePathNamed,
                scrolledlist_items = self.jbNodePathNames)
            self.jbNodePathMenu.selectitem('camera')
            self.jbNodePathMenuEntry = (
                self.jbNodePathMenu.component('entryfield_entry'))
            self.jbNodePathMenuBG = (
                self.jbNodePathMenuEntry.configure('background')[3])
            self.jbNodePathMenu.pack(fill = X, expand = 1)
            self.bind(self.jbNodePathMenu,
                      'Select node path to manipulate using the joybox')

            self.jbXyzSF = Slider.Slider(
                joyboxFrame,
                text = 'XYZ Scale Factor',
                value = 1.0,
                hull_relief = RIDGE, hull_borderwidth = 2,
                min = 1.0, max = 100.0)
            self.jbXyzSF['command'] = (
                lambda v: base.direct.joybox.setXyzMultiplier(v))
            self.jbXyzSF.pack(fill = X, expand = 0)
            self.bind(self.jbXyzSF, 'Set joybox XYZ speed multiplier')

            self.jbHprSF = Slider.Slider(
                joyboxFrame,
                text = 'HPR Scale Factor',
                value = 1.0,
                hull_relief = RIDGE, hull_borderwidth = 2,
                min = 1.0, max = 100.0)
            self.jbHprSF['command'] = (
                lambda v: base.direct.joybox.setHprMultiplier(v))
            self.jbHprSF.pack(fill = X, expand = 0)
            self.bind(self.jbHprSF, 'Set joybox HPR speed multiplier')

    def createTasksPage(self, tasksPage):
        Label(tasksPage, text = 'TASKS',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.taskMgrPanel = TaskManagerWidget(tasksPage, taskMgr)
        self.taskMgrPanel.taskListBox['listbox_height'] = 10

    def createMemPage(self, memPage):
        self.MemExp = MemoryExplorer.MemoryExplorer(
            memPage, nodePath = render,
            scrolledCanvas_hull_width = 250,
            scrolledCanvas_hull_height = 250)
        self.MemExp.pack(fill = BOTH, expand = 1)
            
    def toggleDirect(self):
        if self.directEnabled.get():
            base.direct.enable()
        else:
            base.direct.disable()

    def toggleDirectGrid(self):
        if self.directGridEnabled.get():
            base.direct.grid.enable()
        else:
            base.direct.grid.disable()

    def toggleWidgetOnTop(self):
        if self.directWidgetOnTop.get():
            base.direct.widget.setBin('gui-popup', 0)
            base.direct.widget.setDepthTest(0)
        else:
            base.direct.widget.clearBin()
            base.direct.widget.setDepthTest(1)

    def selectedNodePathHook(self, nodePath):
        # Make sure node path is in nodePathDict
        # MRM: Do we need to truncate list?
        if isinstance(nodePath, NodePath):
            self.addNodePath(nodePath)

    def selectNodePathNamed(self, name):
        # See if node path has already been selected
        nodePath = self.nodePathDict.get(name, None)
        # If not, see if listbox evals into a node path
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
        # Did we finally get something?
        if (nodePath != None):
            # Yes, select it!
            base.direct.select(nodePath)

    def addNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.nodePathNames,
                               self.nodePathMenu, self.nodePathDict)

    def selectJBModeNamed(self, name):
        if name == 'Joe Mode':
            base.direct.joybox.joeMode()
        elif name == 'Drive Mode':
            base.direct.joybox.driveMode()
        elif name == 'Orbit Mode':
            base.direct.joybox.orbitMode()
        elif name == 'Look At Mode':
            base.direct.joybox.lookAtMode()
        elif name == 'Look Around Mode':
            base.direct.joybox.lookAroundMode()
        elif name == 'Walkthru Mode':
            base.direct.joybox.walkthruMode()
        elif name == 'Demo Mode':
            base.direct.joybox.demoMode()
        elif name == 'HPRXYZ Mode':
            base.direct.joybox.hprXyzMode()

    def selectJBNodePathNamed(self, name):
        if name == 'selected':
            nodePath = base.direct.selected.last
            # Add Combo box entry for this selected object
            self.addJBNodePath(nodePath)
        else:
            # See if node path has already been selected
            nodePath = self.jbNodePathDict.get(name, None)
            if (nodePath == None):
                # If not, see if listbox evals into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addJBNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.jbNodePathMenu.component('scrolledlist')
                    listbox.setlist(self.jbNodePathNames)
        # Did we finally get something?
        if (nodePath != None):
            # Yes, select it!
            if (nodePath == 'No Node Path'):
                base.direct.joybox.setNodePath(None)
            else:
                base.direct.joybox.setNodePath(nodePath)

    def addJBNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.jbNodePathNames,
                               self.jbNodePathMenu, self.jbNodePathDict)

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

    ## ENVIRONMENT CONTROLS ##
    # Background #
    def setBackgroundColor(self, r, g, b):
        self.setBackgroundColorVec((r, g, b))
    def setBackgroundColorVec(self, color):
        base.setBackgroundColor(color[0]/255.0,
                                color[1]/255.0,
                                color[2]/255.0)

    def selectDisplayRegionNamed(self, name):
        if (string.find(name, 'Display Region ') >= 0):
            drIndex = string.atoi(name[-1:])
            self.activeDisplayRegion = base.direct.drList[drIndex]
        else:
            self.activeDisplayRegion = None
        # Make sure info is current
        self.updateDisplayRegionInfo()

    def setNear(self, near):
        dr = self.activeDisplayRegion
        if dr:
            dr.camLens.setNear(near)
            cluster('base.camLens.setNear(%f)' % near, 0)

    def setFar(self, far):
        dr = self.activeDisplayRegion
        if dr:
            dr.camLens.setFar(far)
            cluster('base.camLens.setFar(%f)' % far, 0)

    def setHFov(self, hFov):
        dr = self.activeDisplayRegion
        if dr:
            if self.lockedFov.get():
                sf = hFov/dr.getHfov()
                vFov = min(dr.getVfov() * sf, 170.0)
                dr.setFov(hFov, vFov)
                # Update scale
                self.vFov.set(vFov, 0)
            else:
                # Just set horizontal
                dr.setHfov(hFov)

    def setVFov(self, vFov):
        dr = self.activeDisplayRegion
        if dr:
            if self.lockedFov.get():
                sf = vFov/dr.getVfov()
                hFov = min(dr.getHfov() * sf, 170.0)
                dr.setFov(hFov, vFov)
                # Update scale
                self.hFov.set(hFov, 0)
            else:
                # Just set horizontal
                dr.setVfov(vFov)

    def resetFov(self):
        dr = self.activeDisplayRegion
        if dr:
            dr.setFov(45.0, 33.75)
            self.hFov.set(45.0, 0)
            self.vFov.set(33.75, 0)

    # Lights #
    def selectLightNamed(self, name):
        # See if light exists
        self.activeLight = base.direct.lights[name]
        # If not...create new one
        if self.activeLight == None:
            self.activeLight = base.direct.lights.create(name)
        # Do we have a valid light at this point?
        if self.activeLight:
            light = self.activeLight.getLight()
            if isinstance(light, AmbientLight):
                self.lightNotebook.selectpage('Ambient')
            elif isinstance(light, DirectionalLight):
                self.lightNotebook.selectpage('Directional')
            elif isinstance(light, PointLight):
                self.lightNotebook.selectpage('Point')
            elif isinstance(light, Spotlight):
                self.lightNotebook.selectpage('Spot')
        else:
            # Restore valid data
            listbox = self.lightMenu.component('scrolledlist')
            listbox.setlist(base.direct.lights.getNameList())
            if len(base.direct.lights) > 0:
                self.lightMenu.selectitem(base.direct.lights.getNameList()[0])
        # Make sure info is current
        self.updateLightInfo()

    def addAmbient(self):
        return base.direct.lights.create('ambient')

    def addDirectional(self):
        return base.direct.lights.create('directional')

    def addPoint(self):
        return base.direct.lights.create('point')

    def addSpot(self):
        return base.direct.lights.create('spot')

    def addLight(self, light):
        # Make list reflect current list of lights
        listbox = self.lightMenu.component('scrolledlist')
        listbox.setlist(base.direct.lights.getNameList())
        # Select the newly added light
        self.lightMenu.selectitem(light.getName())
        # And show corresponding page
        self.selectLightNamed(light.getName())

    def toggleLights(self):
        if self.enableLights.get():
            base.direct.lights.allOn()
        else:
            base.direct.lights.allOff()

    def toggleActiveLight(self):
        if self.activeLight:
            if self.lightActive.get():
                base.direct.lights.setOn(self.activeLight)
            else:
                base.direct.lights.setOff(self.activeLight)

    def setLightColor(self, color):
        if self.activeLight:
            self.activeLight.getLight().setColor(Vec4(color[0]/255.0,
                                                      color[1]/255.0,
                                                      color[2]/255.0,
                                                      color[3]/255.0))

    def setSpecularColor(self, color):
        if self.activeLight:
            self.activeLight.getLight().setSpecularColor(Vec4(color[0]/255.0,
                                                              color[1]/255.0,
                                                              color[2]/255.0,
                                                              color[3]/255.0))

    def setConstantAttenuation(self, value):
        if self.activeLight:
            self.activeLight.getLight().setConstantAttenuation(value)

    def setLinearAttenuation(self, value):
        if self.activeLight:
            self.activeLight.getLight().setLinearAttenuation(value)

    def setQuadraticAttenuation(self, value):
        if self.activeLight:
            self.activeLight.getLight().setQuadraticAttenuation(value)

    def setExponent(self, value):
        if self.activeLight:
            self.activeLight.getLight().setExponent(value)

    ## GRID CONTROLS ##
    def toggleGrid(self):
        if self.enableGrid.get():
            base.direct.grid.enable()
        else:
            base.direct.grid.disable()

    def toggleXyzSnap(self):
        base.direct.grid.setXyzSnap(self.xyzSnap.get())

    def toggleHprSnap(self):
        base.direct.grid.setHprSnap(self.hprSnap.get())

    ## DEVICE CONTROLS
    def toggleJoybox(self):
        if self.enableJoybox.get():
            base.direct.joybox.enable()
        else:
            base.direct.joybox.disable()

    ## UPDATE INFO ##
    def updateInfo(self, page = 'Environment'):
        if page == 'Environment':
            self.updateEnvironmentInfo()
        elif page == 'Lights':
            self.updateLightInfo()
        elif page == 'Grid':
            self.updateGridInfo()

    def updateEnvironmentInfo(self):
        bkgrdColor = base.getBackgroundColor() * 255.0
        self.backgroundColor.set([bkgrdColor[0],
                                  bkgrdColor[1],
                                  bkgrdColor[2],
                                  bkgrdColor[3]], 0)
        self.updateDisplayRegionInfo()

    def updateDisplayRegionInfo(self):
        if self.activeDisplayRegion:
            self.nearPlane.set(self.activeDisplayRegion.near, 0)
            self.farPlane.set(self.activeDisplayRegion.far, 0)
            self.hFov.set(self.activeDisplayRegion.fovH, 0)
            self.vFov.set(self.activeDisplayRegion.fovV, 0)

    def updateLightInfo(self, page = None):
        # Set main lighting button
        self.enableLights.set(
            render.node().hasAttrib(LightAttrib.getClassType()))

        # Set light specific info
        if self.activeLight:
            l = self.activeLight.getLight()
            self.lightActive.set(render.hasLight(self.activeLight))
            lightColor = l.getColor() * 255.0
            self.lightColor.set([lightColor[0], lightColor[1],
                                 lightColor[2], lightColor[3]], 0)
            if isinstance(l, DirectionalLight):
                specularColor = l.getSpecularColor() * 255.0
                self.dSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
            elif isinstance(l, PointLight):
                specularColor = l.getSpecularColor() * 255.0
                self.pSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
                att = l.getAttenuation()
                self.pConstantAttenuation.set(att[0], 0)
                self.pLinearAttenuation.set(att[1], 0)
                self.pQuadraticAttenuation.set(att[2], 0)
            elif isinstance(l, Spotlight):
                specularColor = l.getSpecularColor() * 255.0
                self.sSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
                att = l.getAttenuation()
                self.pConstantAttenuation.set(att[0], 0)
                self.pLinearAttenuation.set(att[1], 0)
                self.pQuadraticAttenuation.set(att[2], 0)

    def updateGridInfo(self):
        self.enableGrid.set(base.direct.grid.isEnabled())
        self.xyzSnap.set(base.direct.grid.getXyzSnap())
        self.hprSnap.set(base.direct.grid.getHprSnap())
        self.gridSpacing.set(base.direct.grid.getGridSpacing(), 0)
        self.gridSize.set(base.direct.grid.getGridSize(), 0)
        self.gridSnapAngle.set(base.direct.grid.getSnapAngle(), 0)

    # UNDO/REDO
    def pushUndo(self, fResetRedo = 1):
        base.direct.pushUndo([self['nodePath']])

    def undoHook(self, nodePathList = []):
        pass

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        base.direct.pushRedo([self['nodePath']])

    def redoHook(self, nodePathList = []):
        pass

    def pushRedoHook(self):
        # Make sure button is reactivated
        self.redoButton.configure(state = 'normal')

    def redoListEmptyHook(self):
        # Make sure button is deactivated
        self.redoButton.configure(state = 'disabled')

    def onDestroy(self, event):
        # Remove hooks
        for event, method in self.actionEvents:
            self.ignore(event)
        # Destroy SGE hierarchy
        self.SGE._node.destroy()
