""" DIRECT Session Main panel """

# Import Tkinter, Pmw, and the dial code
from PandaObject import *
from AppShell import *
from Tkinter import *
import string
import Pmw
import Dial
import Floater
import EntryScale
import VectorWidgets
import SceneGraphExplorer

"""
Possible to add:
messenger.clear?
popup panels
taskMgr page
"""

class DirectSessionPanel(AppShell):
    # Override class variables here
    appname = 'Direct Session Panel'
    frameWidth      = 550
    frameHeight     = 502
    usecommandarea = 1
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
        if len(direct.lights) > 0:
            name = direct.lights[0].getName()
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
        self.nodePathDict['widget'] = direct.widget
        self.nodePathNames = ['widget']

        # Set up event hooks
        self.actionEvents = [('undo', self.undoHook),
                             ('pushUndo', self.pushUndoHook),
                             ('undoListEmpty', self.undoListEmptyHook),
                             ('redo', self.redoHook),
                             ('pushRedo', self.pushRedoHook),
                             ('redoListEmpty', self.redoListEmptyHook),
                             ('selectedNodePath', self.selectedNodePathHook),
                             ('DirectLights_addLight', self.addLight),
                             ]
        for event, method in self.actionEvents:
            self.accept(event, method)

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        # Add placer commands to menubar
        self.menuBar.addmenu('DIRECT', 'Direct Session Panel Operations')
        
        self.directEnabled = BooleanVar()
        self.directEnabled.set(direct.isEnabled())
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Enabled',
                                 label = 'Enable',
                                 variable = self.directEnabled,
                                 command = self.toggleDirect)
        
        self.directGridEnabled = BooleanVar()
        self.directGridEnabled.set(direct.grid.isEnabled())
        self.menuBar.addmenuitem('DIRECT', 'checkbutton',
                                 'DIRECT Grid Enabled',
                                 label = 'Enable Grid',
                                 variable = self.directGridEnabled,
                                 command = self.toggleDirectGrid)
        
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
        self.nodePathMenu.pack(side = 'left', fill = 'x', expand = 1)
        self.bind(self.nodePathMenu, 'Select node path to manipulate')

        self.undoButton = Button(menuFrame, text = 'Undo',
                                 command = direct.undo)
        if direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = 'left', expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(menuFrame, text = 'Redo',
                                 command = direct.redo)
        if direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = 'left', expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # The master frame for the dials
	mainFrame = Frame(interior)

        # Scene Graph Explorer
        sgeFrame = Frame(mainFrame)
        self.sgeUpdate = Button(sgeFrame, text = 'Update Explorer')
        self.sgeUpdate.pack(fill = 'x', expand = 0)
        self.SGE = SceneGraphExplorer.SceneGraphExplorer(
            sgeFrame, nodePath = render,
            scrolledCanvas_hull_width = 200,
            scrolledCanvas_hull_height = 400)
        self.SGE.pack(fill = BOTH, expand = 0)
        self.sgeUpdate['command'] = self.SGE.update
        sgeFrame.pack(side = LEFT, fill = 'both', expand = 0)

        # Create the notebook pages
        notebook = Pmw.NoteBook(mainFrame)
        notebook.pack(fill = BOTH, expand = 1)
        envPage = notebook.add('Environment')
        lightsPage = notebook.add('Lights')
        renderPage = notebook.add('Render Style')
        gridPage = notebook.add('Grid')
        scenePage = notebook.add('Scene')
        # Put this here so it isn't called right away
        notebook['raisecommand'] = self.updateInfo

        ## Environment page ##
        # Backgroud color
        bkgrdFrame = Frame(envPage, borderwidth = 2, relief = 'sunken')
        
        Label(bkgrdFrame, text = 'Background',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        
        self.backgroundColor = VectorWidgets.ColorEntry(
            bkgrdFrame, text = 'Background Color')
        self.backgroundColor['command'] = self.setBackgroundColor
        self.backgroundColor.pack(fill = 'x', expand = 0)
        self.bind(self.backgroundColor, 'Set background color')
        bkgrdFrame.pack(fill = BOTH, expand = 0)

        drFrame = Frame(envPage, borderwidth = 2, relief = 'sunken')
        Label(drFrame, text = 'Display Region',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        
        nameList = map(lambda x: 'Display Region ' + `x`,
                       range(len(direct.drList)))
        self.drMenu = Pmw.ComboBox(
            drFrame, labelpos = W, label_text = 'Display Region:',
            entry_width = 20,
            selectioncommand = self.selectDisplayRegionNamed,
            scrolledlist_items = nameList)
        self.drMenu.pack(fill = 'x', expand = 0)
        self.bind(self.drMenu, 'Select display region to configure')
        
        self.nearPlane = Floater.Floater(
            drFrame,
            text = 'Near Plane',
            min = 0.01)
        self.nearPlane['command'] = self.setNear
        self.nearPlane.pack(fill = 'x', expand = 0)
        self.bind(self.nearPlane, 'Set near plane distance')
           
        self.farPlane = Floater.Floater(
            drFrame,
            text = 'Far Plane',
            min = 0.01)
        self.farPlane['command'] = self.setFar
        self.farPlane.pack(fill = 'x', expand = 0)
        self.bind(self.farPlane, 'Set far plane distance')
           
        self.hFov = Floater.Floater(
            drFrame,
            text = 'Horizontal FOV',
            min = 0.01, max = 179.9)
        self.hFov['command'] = self.setHFov
        self.hFov.pack(fill = 'x', expand = 0)
        self.bind(self.hFov, 'Set horizontal field of view')
           
        self.vFov = Floater.Floater(
            drFrame,
            text = 'Vertical FOV',
            min = 0.01, max = 179.9)
        self.vFov['command'] = self.setVFov
        self.vFov.pack(fill = 'x', expand = 0)
        self.bind(self.vFov, 'Set vertical field of view')
           
        self.lockedFov = BooleanVar()
        self.lockedFov.set(1)
        self.lockedFovButton = Checkbutton(
            drFrame,
            text = 'FOV Locked',
            anchor = 'w', justify = 'left',
            variable = self.lockedFov)
        self.lockedFovButton.pack(fill = 'x', expand = 0)

        drFrame.pack(fill = BOTH, expand = 0)

        ## Lights page ##
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
              text = 'Lighting:').pack(side = 'left', expand = 0)
        self.enableLights = BooleanVar()
        self.enableLightsButton = Checkbutton(
            mainSwitchFrame,
            text = 'Enabled/Disabled',
            variable = self.enableLights,
            command = self.toggleLights)
        self.enableLightsButton.pack(side = 'left', fill = 'x', expand = 0)
        mainSwitchFrame.pack(fill = 'x', expand = 0)
        
        # Widget to select a light to configure
        nameList = direct.lights.nameList
        lightMenuFrame = Frame(lightFrame)
        
        self.lightMenu = Pmw.ComboBox(
            lightMenuFrame, labelpos = W, label_text = 'Light:',
            entry_width = 20,
            selectioncommand = self.selectLightNamed,
            scrolledlist_items = nameList)
        self.lightMenu.pack(side = 'left', fill = 'x', expand = 0)
        self.bind(self.lightMenu, 'Select light to configure')
        
        self.lightActive = BooleanVar()
        self.lightActiveButton = Checkbutton(
            lightMenuFrame,
            text = 'On/Off',
            variable = self.lightActive,
            command = self.toggleActiveLight)
        self.lightActiveButton.pack(side = 'left', fill = 'x', expand = 0)

        # Pack light menu
        lightMenuFrame.pack(fill = 'x', expand = 0, padx = 2)

        self.lightColor = VectorWidgets.ColorEntry(
            lightFrame, text = 'Light Color')
        self.lightColor['command'] = self.setLightColor
        self.lightColor.pack(fill = 'x', expand = 0, padx = 4)
        self.bind(self.lightColor, 'Set active light color')

        # Directional light controls
        self.dSpecularColor = VectorWidgets.ColorEntry(
            directionalPage, text = 'Specular Color')
        self.dSpecularColor['command'] = self.setSpecularColor
        self.dSpecularColor.pack(fill = 'x', expand = 0)
        self.bind(self.dSpecularColor,
                  'Set directional light specular color')

        # Point light controls
        self.pSpecularColor = VectorWidgets.ColorEntry(
            pointPage, text = 'Specular Color')
        self.pSpecularColor['command'] = self.setSpecularColor
        self.pSpecularColor.pack(fill = 'x', expand = 0)
        self.bind(self.pSpecularColor,
                  'Set point light specular color')

        self.pConstantAttenuation = EntryScale.EntryScale(
            pointPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, initialValue = 1.0)
        self.pConstantAttenuation['command'] = self.setConstantAttenuation
        self.pConstantAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.pConstantAttenuation,
                  'Set point light constant attenuation')
           
        self.pLinearAttenuation = EntryScale.EntryScale(
            pointPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, initialValue = 0.0)
        self.pLinearAttenuation['command'] = self.setLinearAttenuation
        self.pLinearAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.pLinearAttenuation,
                  'Set point light linear attenuation')
           
        self.pQuadraticAttenuation = EntryScale.EntryScale(
            pointPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, initialValue = 0.0)
        self.pQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.pQuadraticAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.pQuadraticAttenuation,
                  'Set point light quadratic attenuation')
           
        # Spot light controls
        self.sSpecularColor = VectorWidgets.ColorEntry(
            spotPage, text = 'Specular Color')
        self.sSpecularColor['command'] = self.setSpecularColor
        self.sSpecularColor.pack(fill = 'x', expand = 0)
        self.bind(self.sSpecularColor,
                  'Set spot light specular color')

        self.sConstantAttenuation = EntryScale.EntryScale(
            spotPage,
            text = 'Constant Attenuation',
            min = 0.0, max = 1.0, initialValue = 1.0)
        self.sConstantAttenuation['command'] = self.setConstantAttenuation
        self.sConstantAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.sConstantAttenuation,
                  'Set spot light constant attenuation')
           
        self.sLinearAttenuation = EntryScale.EntryScale(
            spotPage,
            text = 'Linear Attenuation',
            min = 0.0, max = 1.0, initialValue = 0.0)
        self.sLinearAttenuation['command'] = self.setLinearAttenuation
        self.sLinearAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.sLinearAttenuation,
                  'Set spot light linear attenuation')
           
        self.sQuadraticAttenuation = EntryScale.EntryScale(
            spotPage,
            text = 'Quadratic Attenuation',
            min = 0.0, max = 1.0, initialValue = 0.0)
        self.sQuadraticAttenuation['command'] = self.setQuadraticAttenuation
        self.sQuadraticAttenuation.pack(fill = 'x', expand = 0)
        self.bind(self.sQuadraticAttenuation,
                  'Set spot light quadratic attenuation')
           
        self.sExponent = EntryScale.EntryScale(
            spotPage,
            text = 'Exponent',
            min = 0.0, max = 1.0, initialValue = 0.0)
        self.sExponent['command'] = self.setExponent
        self.sExponent.pack(fill = 'x', expand = 0)
        self.bind(self.sExponent,
                  'Set spot light exponent')

        # MRM: Add frustum controls
           
        self.lightNotebook.setnaturalsize()
        self.lightNotebook.pack(expand = 1, fill = BOTH)
        
        lightFrame.pack(expand = 1, fill = BOTH)

        ## Render Style ##
        toggleFrame = Frame(renderPage)
        self.toggleBackfaceButton = Button(
            toggleFrame,
            text = 'Toggle backface',
            command = base.toggleBackface)
        self.toggleBackfaceButton.pack(fill = 'x', expand = 0)
        
        self.toggleLightsButton = Button(
            toggleFrame,
            text = 'Toggle lights',
            command = direct.lights.toggle)
        self.toggleLightsButton.pack(fill = 'x', expand = 0)

        self.toggleTextureButton = Button(
            toggleFrame,
            text = 'Toggle texture',
            command = base.toggleTexture)
        self.toggleTextureButton.pack(fill = 'x', expand = 0)
        
        self.toggleWireframeButton = Button(
            toggleFrame,
            text = 'Toggle wireframe',
            command = base.toggleWireframe)
        self.toggleWireframeButton.pack(fill = 'x', expand = 0)
        toggleFrame.pack(fill = BOTH, expand = 0)

        ## GRID PAGE ##
        Label(gridPage, text = 'Grid',
              font=('MSSansSerif', 14, 'bold')).pack(expand = 0)
        self.enableGrid = BooleanVar()
        self.enableGridButton = Checkbutton(
            gridPage,
            text = 'Enabled/Disabled',
            anchor = 'w', justify = 'left',
            variable = self.enableGrid,
            command = self.toggleGrid)
        self.enableGridButton.pack(fill = 'x', expand = 0)

        self.xyzSnap = BooleanVar()
        self.xyzSnapButton = Checkbutton(
            gridPage,
            text = 'XYZ Snap',
            anchor = 'w', justify = 'left',
            variable = self.xyzSnap,
            command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(fill = 'x', expand = 0)

        self.hprSnap = BooleanVar()
        self.hprSnapButton = Checkbutton(
            gridPage,
            text = 'HPR Snap',
            anchor = 'w', justify = 'left',
            variable = self.hprSnap,
            command = self.toggleHprSnap)
        self.hprSnapButton.pack(fill = 'x', expand = 0)

        self.gridSpacing = Floater.Floater(
            gridPage,
            text = 'Grid Spacing',
            min = 0.1,
            initialValue = direct.grid.getGridSpacing())
        self.gridSpacing['command'] = direct.grid.setGridSpacing
        self.gridSpacing.pack(fill = 'x', expand = 0)
        
        self.gridSize = Floater.Floater(
            gridPage,
            text = 'Grid Size',
            min = 1.0,
            initialValue = direct.grid.getGridSize())
        self.gridSize['command'] = direct.grid.setGridSize
        self.gridSize.pack(fill = 'x', expand = 0)

        self.gridSnapAngle = Dial.Dial(
            gridPage,
            text = 'Snap Angle',
            min = 0.0, max = 360.0,
            numTicks = 12,
            fRollover = 0,
            initialValue = direct.grid.getSnapAngle())
        self.gridSnapAngle['command'] = direct.grid.setSnapAngle
        self.gridSnapAngle.pack(fill = 'x', expand = 0)
        
        notebook.setnaturalsize()
        
        mainFrame.pack(fill = 'both', expand = 1)

        # Create some buttons in the bottom tray
        self.createButtons()

        # Clean up things when you destroy the panel
        interior.bind('<Destroy>', self.onDestroy)

    def createButtons(self):
        # Grid: enable/disable, xyz/hpr snap, snap to plane
        # Render mode: wireframe, lights, texture
        self.buttonAdd('Toggle Widget Viz',
                       helpMessage='Toggle Object Handles Visability',
                       statusMessage='Toggle Object Handles Visability',
                       command=direct.toggleWidgetVis)
        self.buttonAdd(
            'Toggle Widget Mode',
            helpMessage='Toggle Widget Move/COA Mode',
            statusMessage='Toggle Widget Move/COA Mode',
            command=direct.manipulationControl.toggleObjectHandlesMode)
        
        # Make all buttons as wide as widest
        self.alignbuttons()

    def toggleDirect(self):
        if self.directEnabled.get():
            direct.enable()
        else:
            direct.disable()

    def toggleDirectGrid(self):
        if self.directGridEnabled.get():
            direct.grid.enable()
        else:
            direct.grid.disable()

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
            direct.select(nodePath)
        
    def addNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.nodePathNames,
                               self.nodePathMenu, self.nodePathDict)

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

    ## ENVIRONMENT CONTROLS ##
    # Background # 
    def setBackgroundColor(self, color):
        base.win.getGsg().setColorClearValue(
            VBase4(color[0]/255.0,
                   color[1]/255.0,
                   color[2]/255.0,
                   1.0))

    def selectDisplayRegionNamed(self, name):
        if (string.find(name, 'Display Region ') >= 0):
            drIndex = string.atoi(name[-1:])
            self.activeDisplayRegion = direct.drList[drIndex]
        else:
            self.activeDisplayRegion = None
        # Make sure info is current
        self.updateDisplayRegionInfo()

    def setNear(self, near):
        dr = self.activeDisplayRegion
        if dr:
            dr.camNode.setNear(near)

    def setFar(self, far):
        dr = self.activeDisplayRegion
        if dr:
            dr.camNode.setFar(far)

    def setHFov(self, hFov):
        dr = self.activeDisplayRegion
        if dr:
            if self.lockedFov.get():
                sf = hFov/dr.camNode.getHfov()
                vFov = dr.camNode.getVfov() * sf
                dr.camNode.setFov(hFov, vFov)
                # Update scale
                self.vFov.set(vFov, 0)
            else:
                # Just set horizontal
                dr.camNode.setHfov(hFov)

    def setVFov(self, vFov):
        dr = self.activeDisplayRegion
        if dr:
            if self.lockedFov.get():
                sf = vFov/dr.camNode.getVfov()
                hFov = dr.camNode.getHfov() * sf
                dr.camNode.setFov(hFov, vFov)
                # Update scale
                self.hFov.set(hFov, 0)
            else:
                # Just set horizontal
                dr.camNode.setVfov(vFov)

    # Lights #
    def selectLightNamed(self, name):
        self.activeLight = None
        for light in direct.lights:
            if name == light.getName():
                self.activeLight = light
                break
        if self.activeLight == None:
            self.activeLight = direct.lights.create(name)
        if self.activeLight:
            if isinstance(self.activeLight, AmbientLight):
                self.lightNotebook.selectpage('Ambient')
            elif isinstance(self.activeLight, DirectionalLight):
                self.lightNotebook.selectpage('Directional')
            elif isinstance(self.activeLight, PointLight):
                self.lightNotebook.selectpage('Point')
            elif isinstance(self.activeLight, Spotlight):
                self.lightNotebook.selectpage('Spot')
        else:
            # Restore valid data
            listbox = self.lightMenu.component('scrolledlist')
            listbox.setlist(direct.lights.nameList)
            if len(direct.lights) > 0:
                self.lightMenu.selectitem(direct.lights[0].getName())
        # Make sure info is current
        self.updateLightInfo()

    def addAmbient(self):
        direct.lights.create('ambient')

    def addDirectional(self):
        direct.lights.create('directional')

    def addPoint(self):
        direct.lights.create('point')

    def addSpot(self):
        direct.lights.create('spot')

    def addLight(self, light):
        # Make list reflect current list of lights
        listbox = self.lightMenu.component('scrolledlist')
        listbox.setlist(direct.lights.nameList)
        # Select the newly added light
        self.lightMenu.selectitem(light.getName())
        # And show corresponding page
        self.selectLightNamed(light.getName())

    def toggleLights(self):
        if self.enableLights.get():
            direct.lights.allOn()
        else:
            direct.lights.allOff()

    def toggleActiveLight(self):
        if self.activeLight:
            if self.lightActive.get():
                direct.lights.setOn(self.activeLight)
            else:
                direct.lights.setOff(self.activeLight)

    def setLightColor(self, color):
        if self.activeLight:
            self.activeLight.setColor(Vec4(color[0]/255.0,
                                           color[1]/255.0,
                                           color[2]/255.0,
                                           color[3]/255.0))

    def setSpecularColor(self, color):
        if self.activeLight:
            self.activeLight.setSpecular(Vec4(color[0]/255.0,
                                              color[1]/255.0,
                                              color[2]/255.0,
                                              color[3]/255.0))

    def setConstantAttenuation(self, value):
        if self.activeLight:
            self.activeLight.setConstantAttenuation(value)

    def setLinearAttenuation(self, value):
        if self.activeLight:
            self.activeLight.setLinearAttenuation(value)
            
    def setQuadraticAttenuation(self, value):
        if self.activeLight:
            self.activeLight.setQuadraticAttenuation(value)

    def setExponent(self, value):
        if self.activeLight:
            self.activeLight.setExponent(value)

    ## GRID CONTROLS ##
    def toggleGrid(self):
        if self.enableGrid.get():
            direct.grid.enable()
        else:
            direct.grid.disable()

    def toggleXyzSnap(self):
        direct.grid.setXyzSnap(self.xyzSnap.get())

    def toggleHprSnap(self):
        direct.grid.setHprSnap(self.hprSnap.get())

    ## UPDATE INFO ##
    def updateInfo(self, page = 'Environment'):
        if page == 'Environment':
            self.updateEnvironmentInfo()
        elif page == 'Lights':
            self.updateLightInfo()
        elif page == 'Grid':
            self.updateGridInfo()
            
    def updateEnvironmentInfo(self):
        bkgrdColor = base.win.getGsg().getColorClearValue() * 255.0
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
            base.initialState.hasAttribute(LightTransition.getClassType()))
        # Set light specific info
        if self.activeLight:
            l = self.activeLight
            self.lightActive.set(direct.lights.la.isOn(l))
            lightColor = l.getColor() * 255.0
            self.lightColor.set([lightColor[0], lightColor[1],
                                 lightColor[2], lightColor[3]], 0)
            if isinstance(l, DirectionalLight):
                specularColor = l.getSpecular() * 255.0
                self.dSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
            elif isinstance(l, PointLight):
                specularColor = l.getSpecular() * 255.0
                self.pSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
                constantAtten = l.getConstantAttenuation()
                self.pConstantAttenuation.set(constantAtten, 0)
                linearAtten = l.getLinearAttenuation()
                self.pLinearAttenuation.set(linearAtten, 0)
                quadraticAtten = l.getQuadraticAttenuation()
                self.pQuadraticAttenuation.set(quadraticAtten, 0)
            elif isinstance(l, Spotlight):
                specularColor = l.getSpecular() * 255.0
                self.sSpecularColor.set([specularColor[0],
                                         specularColor[1],
                                         specularColor[2],
                                         specularColor[3]], 0)
                constantAtten = l.getConstantAttenuation()
                self.sConstantAttenuation.set(constantAtten, 0)
                linearAtten = l.getLinearAttenuation()
                self.sLinearAttenuation.set(linearAtten, 0)
                quadraticAtten = l.getQuadraticAttenuation()
                self.sQuadraticAttenuation.set(quadraticAtten, 0)

    def updateGridInfo(self):
        self.enableGrid.set(direct.grid.isEnabled())
        self.xyzSnap.set(direct.grid.getXyzSnap())
        self.hprSnap.set(direct.grid.getHprSnap())
        self.gridSpacing.set(direct.grid.getGridSpacing(), 0)
        self.gridSize.set(direct.grid.getGridSize(), 0)
        self.gridSnapAngle.set(direct.grid.getSnapAngle(), 0)
            
    # UNDO/REDO
    def pushUndo(self, fResetRedo = 1):
        direct.pushUndo([self['nodePath']])

    def undoHook(self):
        pass

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        direct.pushRedo([self['nodePath']])
        
    def redoHook(self):
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
