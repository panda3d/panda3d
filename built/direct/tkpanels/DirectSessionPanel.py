""" DIRECT Session Main panel """

__all__ = ['DirectSessionPanel']

# Import Tkinter, Pmw, and the dial code
from panda3d.core import (
    AmbientLight,
    DirectionalLight,
    LightAttrib,
    NodePath,
    PointLight,
    Spotlight,
    Vec4,
)
from direct.tkwidgets.AppShell import AppShell
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
from direct.tkwidgets import VectorWidgets
from direct.tkwidgets import SceneGraphExplorer
from direct.tkwidgets import MemoryExplorer
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase import ShowBaseGlobal
from .TaskManagerPanel import TaskManagerWidget
import Pmw
import tkinter as tk


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
        if len(ShowBaseGlobal.direct.lights) > 0:
            name = ShowBaseGlobal.direct.lights.getNameList()[0]
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
        self.nodePathDict['widget'] = ShowBaseGlobal.direct.widget
        self.nodePathNames = ['widget']

        # Dictionary keeping track of all jb node paths selected so far
        self.jbNodePathDict = {}
        self.jbNodePathDict['none'] = 'No Node Path'
        self.jbNodePathDict['widget'] = ShowBaseGlobal.direct.widget
        self.jbNodePathDict['camera'] = ShowBaseGlobal.direct.camera
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

        self.directEnabled = tk.BooleanVar()
        self.directEnabled.set(True)
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Enabled',
                                 label = 'Enable',
                                 variable = self.directEnabled,
                                 command = self.toggleDirect)

        self.directGridEnabled = tk.BooleanVar()
        self.directGridEnabled.set(ShowBaseGlobal.direct.grid.isEnabled())
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Grid Enabled',
                                 label = 'Enable Grid',
                                 variable = self.directGridEnabled,
                                 command = self.toggleDirectGrid)

        self.menuBar.addmenuitem('DIRECT', 'command',
                                 'Toggle Object Handles Visability',
                                 label = 'Toggle Widget Viz',
                                 command = ShowBaseGlobal.direct.toggleWidgetVis)

        self.menuBar.addmenuitem(
            'DIRECT', 'command',
            'Toggle Widget Move/COA Mode',
            label = 'Toggle Widget Mode',
            command = ShowBaseGlobal.direct.manipulationControl.toggleObjectHandlesMode)

        self.directWidgetOnTop = tk.BooleanVar()
        self.directWidgetOnTop.set(False)
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Widget On Top',
                                 label = 'Widget On Top',
                                 variable = self.directWidgetOnTop,
                                 command = self.toggleWidgetOnTop)

        self.menuBar.addmenuitem('DIRECT', 'command',
                                 'Deselect All',
                                 label = 'Deselect All',
                                 command = ShowBaseGlobal.direct.deselectAll)

        # Get a handle to the menu frame
        menuFrame = self.menuFrame

        # Widget to select node paths (and display list of selected node paths)
        self.nodePathMenu = Pmw.ComboBox(
            menuFrame, labelpos = tk.W, label_text = 'DIRECT Select:',
            entry_width = 20,
            selectioncommand = self.selectNodePathNamed,
            scrolledlist_items = self.nodePathNames)
        self.nodePathMenu.selectitem('widget')
        self.nodePathMenuEntry = (
            self.nodePathMenu.component('entryfield_entry'))
        self.nodePathMenuBG = (
            self.nodePathMenuEntry.configure('background')[3])
        self.nodePathMenu.pack(side = tk.LEFT, fill = tk.X, expand = 1)
        self.bind(self.nodePathMenu, 'Select node path to manipulate')

        self.undoButton = tk.Button(menuFrame, text = 'Undo',
                                    command = ShowBaseGlobal.direct.undo)
        if ShowBaseGlobal.direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = tk.LEFT, expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = tk.Button(menuFrame, text = 'Redo',
                                    command = ShowBaseGlobal.direct.redo)
        if ShowBaseGlobal.direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = tk.LEFT, expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # The master frame for the dials
        mainFrame = tk.Frame(interior)

        # Paned widget for dividing two halves
        framePane = Pmw.PanedWidget(mainFrame, orient = tk.HORIZONTAL)
        sgeFrame = framePane.add('left', min = 250)
        notebookFrame = framePane.add('right', min = 300)

        # Scene Graph Explorer
        self.SGE = SceneGraphExplorer.SceneGraphExplorer(
            sgeFrame, nodePath = ShowBaseGlobal.base.render,
            scrolledCanvas_hull_width = 250,
            scrolledCanvas_hull_height = 300)
        self.SGE.pack(fill = tk.BOTH, expand = 1)
        sgeFrame.pack(side = tk.LEFT, fill = 'both', expand = 1)

        # Create the notebook pages
        notebook = Pmw.NoteBook(notebookFrame)
        notebook.pack(fill = tk.BOTH, expand = 1)
        self.createEnvPage(notebook.add('Environment'))
        self.createLightsPage(notebook.add('Lights'))
        self.createGridPage(notebook.add('Grid'))
        self.createDevicePage(notebook.add('Devices'))
        self.createTasksPage(notebook.add('Tasks'))
        self.createMemPage(notebook.add('Memory'))

        notebook.setnaturalsize()

        framePane.pack(expand = 1, fill = tk.BOTH)
        mainFrame.pack(fill = 'both', expand = 1)

        # Put this here so it isn't called right away
        notebook['raisecommand'] = self.updateInfo

    def createEnvPage(self, envPage):
        bkgrdFrame = tk.Frame(envPage, borderwidth = 2, relief = 'sunken')

        tk.Label(bkgrdFrame, text = 'Background',
                 font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        self.backgroundColor = VectorWidgets.ColorEntry(
            bkgrdFrame, text = 'Background Color')
        self.backgroundColor['command'] = self.setBackgroundColorVec
        self.backgroundColor.pack(fill = tk.X, expand = 0)
        self.bind(self.backgroundColor, 'Set background color')
        bkgrdFrame.pack(fill = tk.BOTH, expand = 0)

        drFrame = tk.Frame(envPage, borderwidth = 2, relief = 'sunken')
        tk.Label(drFrame, text = 'Display Region',
                 font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        nameList = ['Display Region ' + repr(x) for x in range(len(ShowBaseGlobal.direct.drList))]
        self.drMenu = Pmw.ComboBox(
            drFrame, labelpos = tk.W, label_text = 'Display Region:',
            entry_width = 20,
            selectioncommand = self.selectDisplayRegionNamed,
            scrolledlist_items = nameList)
        self.drMenu.pack(fill = tk.X, expand = 0)
        self.bind(self.drMenu, 'Select display region to configure')

        self.nearPlane = Floater.Floater(
            drFrame,
            text = 'Near Plane',
            min = 0.01)
        self.nearPlane['command'] = self.setNear
        self.nearPlane.pack(fill = tk.X, expand = 0)
        self.bind(self.nearPlane, 'Set near plane distance')

        self.farPlane = Floater.Floater(
            drFrame,
            text = 'Far Plane',
            min = 0.01)
        self.farPlane['command'] = self.setFar
        self.farPlane.pack(fill = tk.X, expand = 0)
        self.bind(self.farPlane, 'Set far plane distance')

        fovFrame = tk.Frame(drFrame)
        fovFloaterFrame = tk.Frame(fovFrame)
        self.hFov = Slider.Slider(
            fovFloaterFrame,
            text = 'Horizontal FOV',
            min = 0.01, max = 170.0)
        self.hFov['command'] = self.setHFov
        self.hFov.pack(fill = tk.X, expand = 0)
        self.bind(self.hFov, 'Set horizontal field of view')

        self.vFov = Slider.Slider(
            fovFloaterFrame,
            text = 'Vertical FOV',
            min = 0.01, max = 170.0)
        self.vFov['command'] = self.setVFov
        self.vFov.pack(fill = tk.X, expand = 0)
        self.bind(self.vFov, 'Set vertical field of view')
        fovFloaterFrame.pack(side = tk.LEFT, fill = tk.X, expand = 1)

        frame = tk.Frame(fovFrame)
        self.lockedFov = tk.BooleanVar()
        self.lockedFov.set(True)
        self.lockedFovButton = tk.Checkbutton(
            frame,
            text = 'Locked',
            anchor = 'w', justify = tk.LEFT,
            variable = self.lockedFov)
        self.lockedFovButton.pack(fill = tk.X, expand = 0)

        self.resetFovButton = tk.Button(
            frame,
            text = 'Reset',
            command = self.resetFov)
        self.resetFovButton.pack(fill = tk.X, expand = 0)
        frame.pack(side = tk.LEFT, fill = tk.X, expand = 0)
        fovFrame.pack(fill = tk.X, expand = 1)

        drFrame.pack(fill = tk.BOTH, expand = 0)

        ## Render Style ##
        toggleFrame = tk.Frame(envPage, borderwidth = 2, relief = 'sunken')
        tk.Label(toggleFrame, text = 'Toggle Render Style',
                 font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.toggleBackfaceButton = tk.Button(
            toggleFrame,
            text = 'Backface',
            command = ShowBaseGlobal.base.toggleBackface)
        self.toggleBackfaceButton.pack(side = tk.LEFT, fill = tk.X, expand = 1)

        self.toggleLightsButton = tk.Button(
            toggleFrame,
            text = 'Lights',
            command = ShowBaseGlobal.direct.lights.toggle)
        self.toggleLightsButton.pack(side = tk.LEFT, fill = tk.X, expand = 1)

        self.toggleTextureButton = tk.Button(
            toggleFrame,
            text = 'Texture',
            command = ShowBaseGlobal.base.toggleTexture)
        self.toggleTextureButton.pack(side = tk.LEFT, fill = tk.X, expand = 1)

        self.toggleWireframeButton = tk.Button(
            toggleFrame,
            text = 'Wireframe',
            command = ShowBaseGlobal.base.toggleWireframe)
        self.toggleWireframeButton.pack(fill = tk.X, expand = 1)
        toggleFrame.pack(side = tk.LEFT, fill = tk.X, expand = 1)

    def createLightsPage(self, lightsPage):
        # Lights #
        lightFrame = tk.Frame(lightsPage, borderwidth = 2, relief = 'sunken')
        self.lightsButton = tk.Menubutton(lightFrame, text = 'Lights',
                                          font=('MSSansSerif', 14, 'bold'),
                                          activebackground = '#909090')
        lightsMenu = tk.Menu(self.lightsButton)
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
        mainSwitchFrame = tk.Frame(lightFrame)
        tk.Label(mainSwitchFrame,
                 text = 'Lighting:').pack(side = tk.LEFT, expand = 0)
        self.enableLights = tk.BooleanVar()
        self.enableLightsButton = tk.Checkbutton(
            mainSwitchFrame,
            text = 'Enabled/Disabled',
            variable = self.enableLights,
            command = self.toggleLights)
        self.enableLightsButton.pack(side = tk.LEFT, fill = tk.X, expand = 0)
        mainSwitchFrame.pack(fill = tk.X, expand = 0)

        # Widget to select a light to configure
        nameList = ShowBaseGlobal.direct.lights.getNameList()
        lightMenuFrame = tk.Frame(lightFrame)

        self.lightMenu = Pmw.ComboBox(
            lightMenuFrame, labelpos = tk.W, label_text = 'Light:',
            entry_width = 20,
            selectioncommand = self.selectLightNamed,
            scrolledlist_items = nameList)
        self.lightMenu.pack(side = tk.LEFT, fill = tk.X, expand = 0)
        self.bind(self.lightMenu, 'Select light to configure')

        self.lightActive = tk.BooleanVar()
        self.lightActiveButton = tk.Checkbutton(
            lightMenuFrame,
            text = 'On/Off',
            variable = self.lightActive,
            command = self.toggleActiveLight)
        self.lightActiveButton.pack(side = tk.LEFT, fill = tk.X, expand = 0)

        # Pack light menu
        lightMenuFrame.pack(fill = tk.X, expand = 0, padx = 2)

        self.lightColor = VectorWidgets.ColorEntry(
            lightFrame, text = 'Light Color')
        self.lightColor['command'] = self.setLightColor
        self.lightColor.pack(fill = tk.X, expand = 0, padx = 4)
        self.bind(self.lightColor, 'Set active light color')

        # Directional light controls
        self.dSpecularColor = VectorWidgets.ColorEntry(
            directionalPage, text = 'Specular Color')
        self.dSpecularColor['command'] = self.setSpecularColor
        self.dSpecularColor.pack(fill = tk.X, expand = 0)
        self.bind(self.dSpecularColor,
                  'Set directional light specular color')

        # Point light controls
        self.pSpecularColor = VectorWidgets.ColorEntry(
            pointPage, text = 'Specular Color')
        self.pSpecularColor['command'] = self.setSpecularColor
        self.pSpecularColor.pack(fill = tk.X, expand = 0)
        self.bind(self.pSpecularColor,
                  'Set point light specular color')

        self.pConstantAttenuation = Slider.Slider(
            pointPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, value = 1.0)
        self.pConstantAttenuation['command'] = self.setConstantAttenuation
        self.pConstantAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.pConstantAttenuation,
                  'Set point light constant attenuation')

        self.pLinearAttenuation = Slider.Slider(
            pointPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.pLinearAttenuation['command'] = self.setLinearAttenuation
        self.pLinearAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.pLinearAttenuation,
                  'Set point light linear attenuation')

        self.pQuadraticAttenuation = Slider.Slider(
            pointPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.pQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.pQuadraticAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.pQuadraticAttenuation,
                  'Set point light quadratic attenuation')

        # Spot light controls
        self.sSpecularColor = VectorWidgets.ColorEntry(
            spotPage, text = 'Specular Color')
        self.sSpecularColor['command'] = self.setSpecularColor
        self.sSpecularColor.pack(fill = tk.X, expand = 0)
        self.bind(self.sSpecularColor,
                  'Set spot light specular color')

        self.sConstantAttenuation = Slider.Slider(
            spotPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, value = 1.0)
        self.sConstantAttenuation['command'] = self.setConstantAttenuation
        self.sConstantAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.sConstantAttenuation,
                  'Set spot light constant attenuation')

        self.sLinearAttenuation = Slider.Slider(
            spotPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.sLinearAttenuation['command'] = self.setLinearAttenuation
        self.sLinearAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.sLinearAttenuation,
                  'Set spot light linear attenuation')

        self.sQuadraticAttenuation = Slider.Slider(
            spotPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, value = 0.0)
        self.sQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.sQuadraticAttenuation.pack(fill = tk.X, expand = 0)
        self.bind(self.sQuadraticAttenuation,
                  'Set spot light quadratic attenuation')

        self.sExponent = Slider.Slider(
            spotPage,
            text = 'Exponent',
            min = 0.0, max = 1.0, value = 0.0)
        self.sExponent['command'] = self.setExponent
        self.sExponent.pack(fill = tk.X, expand = 0)
        self.bind(self.sExponent,
                  'Set spot light exponent')

        # MRM: Add frustum controls

        self.lightNotebook.setnaturalsize()
        self.lightNotebook.pack(expand = 1, fill = tk.BOTH)

        lightFrame.pack(expand = 1, fill = tk.BOTH)

    def createGridPage(self, gridPage):
        tk.Label(gridPage, text = 'Grid',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.enableGrid = tk.BooleanVar()
        self.enableGridButton = tk.Checkbutton(
            gridPage,
            text = 'Enabled/Disabled',
            anchor = 'w', justify = tk.LEFT,
            variable = self.enableGrid,
            command = self.toggleGrid)
        self.enableGridButton.pack(fill = tk.X, expand = 0)

        self.xyzSnap = tk.BooleanVar()
        self.xyzSnapButton = tk.Checkbutton(
            gridPage,
            text = 'XYZ Snap',
            anchor = 'w', justify = tk.LEFT,
            variable = self.xyzSnap,
            command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(fill = tk.X, expand = 0)

        self.hprSnap = tk.BooleanVar()
        self.hprSnapButton = tk.Checkbutton(
            gridPage,
            text = 'HPR Snap',
            anchor = 'w', justify = tk.LEFT,
            variable = self.hprSnap,
            command = self.toggleHprSnap)
        self.hprSnapButton.pack(fill = tk.X, expand = 0)

        self.gridSpacing = Floater.Floater(
            gridPage,
            text = 'Grid Spacing',
            min = 0.1,
            value = ShowBaseGlobal.direct.grid.getGridSpacing())
        self.gridSpacing['command'] = ShowBaseGlobal.direct.grid.setGridSpacing
        self.gridSpacing.pack(fill = tk.X, expand = 0)

        self.gridSize = Floater.Floater(
            gridPage,
            text = 'Grid Size',
            min = 1.0,
            value = ShowBaseGlobal.direct.grid.getGridSize())
        self.gridSize['command'] = ShowBaseGlobal.direct.grid.setGridSize
        self.gridSize.pack(fill = tk.X, expand = 0)

        self.gridSnapAngle = Dial.AngleDial(
            gridPage,
            text = 'Snap Angle',
            style = 'mini',
            value = ShowBaseGlobal.direct.grid.getSnapAngle())
        self.gridSnapAngle['command'] = ShowBaseGlobal.direct.grid.setSnapAngle
        self.gridSnapAngle.pack(fill = tk.X, expand = 0)

    def createDevicePage(self, devicePage):
        tk.Label(devicePage, text = 'DEVICES',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)

        if ShowBaseGlobal.direct.joybox is not None:
            joyboxFrame = tk.Frame(devicePage, borderwidth = 2, relief = 'sunken')
            tk.Label(joyboxFrame, text = 'Joybox',
                     font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
            self.enableJoybox = tk.BooleanVar()
            self.enableJoybox.set(True)
            self.enableJoyboxButton = tk.Checkbutton(
                joyboxFrame,
                text = 'Enabled/Disabled',
                anchor = 'w', justify = tk.LEFT,
                variable = self.enableJoybox,
                command = self.toggleJoybox)
            self.enableJoyboxButton.pack(fill = tk.X, expand = 0)
            joyboxFrame.pack(fill = tk.X, expand = 0)

            self.jbModeMenu = Pmw.ComboBox(
                joyboxFrame, labelpos = tk.W, label_text = 'Joybox Mode:',
                label_width = 16, entry_width = 20,
                selectioncommand = self.selectJBModeNamed,
                scrolledlist_items = ['Joe Mode', 'Drive Mode', 'Orbit Mode',
                                      'Look At Mode', 'Look Around Mode',
                                      'Walkthru Mode', 'Demo Mode',
                                      'HPRXYZ Mode'])
            self.jbModeMenu.selectitem('Joe Mode')
            self.jbModeMenu.pack(fill = tk.X, expand = 1)

            self.jbNodePathMenu = Pmw.ComboBox(
                joyboxFrame, labelpos = tk.W, label_text = 'Joybox Node Path:',
                label_width = 16, entry_width = 20,
                selectioncommand = self.selectJBNodePathNamed,
                scrolledlist_items = self.jbNodePathNames)
            self.jbNodePathMenu.selectitem('camera')
            self.jbNodePathMenuEntry = (
                self.jbNodePathMenu.component('entryfield_entry'))
            self.jbNodePathMenuBG = (
                self.jbNodePathMenuEntry.configure('background')[3])
            self.jbNodePathMenu.pack(fill = tk.X, expand = 1)
            self.bind(self.jbNodePathMenu,
                      'Select node path to manipulate using the joybox')

            self.jbXyzSF = Slider.Slider(
                joyboxFrame,
                text = 'XYZ Scale Factor',
                value = 1.0,
                hull_relief = tk.RIDGE, hull_borderwidth = 2,
                min = 1.0, max = 100.0)
            self.jbXyzSF['command'] = (
                lambda v: ShowBaseGlobal.direct.joybox.setXyzMultiplier(v))
            self.jbXyzSF.pack(fill = tk.X, expand = 0)
            self.bind(self.jbXyzSF, 'Set joybox XYZ speed multiplier')

            self.jbHprSF = Slider.Slider(
                joyboxFrame,
                text = 'HPR Scale Factor',
                value = 1.0,
                hull_relief = tk.RIDGE, hull_borderwidth = 2,
                min = 1.0, max = 100.0)
            self.jbHprSF['command'] = (
                lambda v: ShowBaseGlobal.direct.joybox.setHprMultiplier(v))
            self.jbHprSF.pack(fill = tk.X, expand = 0)
            self.bind(self.jbHprSF, 'Set joybox HPR speed multiplier')

    def createTasksPage(self, tasksPage):
        tk.Label(tasksPage, text = 'TASKS',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.taskMgrPanel = TaskManagerWidget(tasksPage, taskMgr)
        self.taskMgrPanel.taskListBox['listbox_height'] = 10

    def createMemPage(self, memPage):
        self.MemExp = MemoryExplorer.MemoryExplorer(
            memPage, nodePath = ShowBaseGlobal.base.render,
            scrolledCanvas_hull_width = 250,
            scrolledCanvas_hull_height = 250)
        self.MemExp.pack(fill = tk.BOTH, expand = 1)

    def toggleDirect(self):
        if self.directEnabled.get():
            ShowBaseGlobal.direct.enable()
        else:
            ShowBaseGlobal.direct.disable()

    def toggleDirectGrid(self):
        if self.directGridEnabled.get():
            ShowBaseGlobal.direct.grid.enable()
        else:
            ShowBaseGlobal.direct.grid.disable()

    def toggleWidgetOnTop(self):
        if self.directWidgetOnTop.get():
            ShowBaseGlobal.direct.widget.setBin('gui-popup', 0)
            ShowBaseGlobal.direct.widget.setDepthTest(0)
        else:
            ShowBaseGlobal.direct.widget.clearBin()
            ShowBaseGlobal.direct.widget.setDepthTest(1)

    def selectedNodePathHook(self, nodePath):
        # Make sure node path is in nodePathDict
        # MRM: Do we need to truncate list?
        if isinstance(nodePath, NodePath):
            self.addNodePath(nodePath)

    def selectNodePathNamed(self, name):
        # See if node path has already been selected
        nodePath = self.nodePathDict.get(name, None)
        # If not, see if listbox evals into a node path
        if nodePath is None:
            # See if this evaluates into a node path
            try:
                nodePath = eval(name)
                if isinstance(nodePath, NodePath):
                    self.addNodePath(nodePath)
                else:
                    # Good eval but not a node path, give up
                    nodePath = None
            except Exception:
                # Bogus eval
                nodePath = None
                # Clear bogus entry from listbox
                listbox = self.nodePathMenu.component('scrolledlist')
                listbox.setlist(self.nodePathNames)
        # Did we finally get something?
        if nodePath is not None:
            # Yes, select it!
            ShowBaseGlobal.direct.select(nodePath)

    def addNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.nodePathNames,
                               self.nodePathMenu, self.nodePathDict)

    def selectJBModeNamed(self, name):
        if name == 'Joe Mode':
            ShowBaseGlobal.direct.joybox.joeMode()
        elif name == 'Drive Mode':
            ShowBaseGlobal.direct.joybox.driveMode()
        elif name == 'Orbit Mode':
            ShowBaseGlobal.direct.joybox.orbitMode()
        elif name == 'Look At Mode':
            ShowBaseGlobal.direct.joybox.lookAtMode()
        elif name == 'Look Around Mode':
            ShowBaseGlobal.direct.joybox.lookAroundMode()
        elif name == 'Walkthru Mode':
            ShowBaseGlobal.direct.joybox.walkthruMode()
        elif name == 'Demo Mode':
            ShowBaseGlobal.direct.joybox.demoMode()
        elif name == 'HPRXYZ Mode':
            ShowBaseGlobal.direct.joybox.hprXyzMode()

    def selectJBNodePathNamed(self, name):
        if name == 'selected':
            nodePath = ShowBaseGlobal.direct.selected.last
            # Add Combo box entry for this selected object
            self.addJBNodePath(nodePath)
        else:
            # See if node path has already been selected
            nodePath = self.jbNodePathDict.get(name, None)
            if nodePath is None:
                # If not, see if listbox evals into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addJBNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except Exception:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.jbNodePathMenu.component('scrolledlist')
                    listbox.setlist(self.jbNodePathNames)
        # Did we finally get something?
        if nodePath is not None:
            # Yes, select it!
            if nodePath == 'No Node Path':
                ShowBaseGlobal.direct.joybox.setNodePath(None)
            else:
                ShowBaseGlobal.direct.joybox.setNodePath(nodePath)

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
            dictName = name + '-' + repr(hash(nodePath))
        if dictName not in dict:
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
        ShowBaseGlobal.base.setBackgroundColor(
            color[0] / 255.0, color[1] / 255.0, color[2] / 255.0)

    def selectDisplayRegionNamed(self, name):
        if name.find('Display Region ') >= 0:
            drIndex = int(name[-1:])
            self.activeDisplayRegion = ShowBaseGlobal.direct.drList[drIndex]
        else:
            self.activeDisplayRegion = None
        # Make sure info is current
        self.updateDisplayRegionInfo()

    def setNear(self, near):
        dr = self.activeDisplayRegion
        if dr:
            dr.camLens.setNear(near)
            ShowBaseGlobal.direct.cluster('base.camLens.setNear(%f)' % near, 0)

    def setFar(self, far):
        dr = self.activeDisplayRegion
        if dr:
            dr.camLens.setFar(far)
            ShowBaseGlobal.direct.cluster('base.camLens.setFar(%f)' % far, 0)

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
        self.activeLight = ShowBaseGlobal.direct.lights[name]
        # If not...create new one
        if self.activeLight is None:
            self.activeLight = ShowBaseGlobal.direct.lights.create(name)
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
            listbox.setlist(ShowBaseGlobal.direct.lights.getNameList())
            if len(ShowBaseGlobal.direct.lights) > 0:
                self.lightMenu.selectitem(ShowBaseGlobal.direct.lights.getNameList()[0])
        # Make sure info is current
        self.updateLightInfo()

    def addAmbient(self):
        return ShowBaseGlobal.direct.lights.create('ambient')

    def addDirectional(self):
        return ShowBaseGlobal.direct.lights.create('directional')

    def addPoint(self):
        return ShowBaseGlobal.direct.lights.create('point')

    def addSpot(self):
        return ShowBaseGlobal.direct.lights.create('spot')

    def addLight(self, light):
        # Make list reflect current list of lights
        listbox = self.lightMenu.component('scrolledlist')
        listbox.setlist(ShowBaseGlobal.direct.lights.getNameList())
        # Select the newly added light
        self.lightMenu.selectitem(light.getName())
        # And show corresponding page
        self.selectLightNamed(light.getName())

    def toggleLights(self):
        if self.enableLights.get():
            ShowBaseGlobal.direct.lights.allOn()
        else:
            ShowBaseGlobal.direct.lights.allOff()

    def toggleActiveLight(self):
        if self.activeLight:
            if self.lightActive.get():
                ShowBaseGlobal.direct.lights.setOn(self.activeLight)
            else:
                ShowBaseGlobal.direct.lights.setOff(self.activeLight)

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
            ShowBaseGlobal.direct.grid.enable()
        else:
            ShowBaseGlobal.direct.grid.disable()

    def toggleXyzSnap(self):
        ShowBaseGlobal.direct.grid.setXyzSnap(self.xyzSnap.get())

    def toggleHprSnap(self):
        ShowBaseGlobal.direct.grid.setHprSnap(self.hprSnap.get())

    ## DEVICE CONTROLS
    def toggleJoybox(self):
        if self.enableJoybox.get():
            ShowBaseGlobal.direct.joybox.enable()
        else:
            ShowBaseGlobal.direct.joybox.disable()

    ## UPDATE INFO ##
    def updateInfo(self, page = 'Environment'):
        if page == 'Environment':
            self.updateEnvironmentInfo()
        elif page == 'Lights':
            self.updateLightInfo()
        elif page == 'Grid':
            self.updateGridInfo()

    def updateEnvironmentInfo(self):
        bkgrdColor = ShowBaseGlobal.base.getBackgroundColor() * 255.0
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
        render = ShowBaseGlobal.base.render
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
        self.enableGrid.set(ShowBaseGlobal.direct.grid.isEnabled())
        self.xyzSnap.set(ShowBaseGlobal.direct.grid.getXyzSnap())
        self.hprSnap.set(ShowBaseGlobal.direct.grid.getHprSnap())
        self.gridSpacing.set(ShowBaseGlobal.direct.grid.getGridSpacing(), 0)
        self.gridSize.set(ShowBaseGlobal.direct.grid.getGridSize(), 0)
        self.gridSnapAngle.set(ShowBaseGlobal.direct.grid.getSnapAngle(), 0)

    # UNDO/REDO
    def pushUndo(self, fResetRedo = 1):
        ShowBaseGlobal.direct.pushUndo([self['nodePath']])

    def undoHook(self, nodePathList = []):
        pass

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        ShowBaseGlobal.direct.pushRedo([self['nodePath']])

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
