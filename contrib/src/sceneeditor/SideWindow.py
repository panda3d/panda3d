#################################################################
# sideWindow.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
from direct.tkwidgets.AppShell import AppShell
from direct.tkwidgets.VectorWidgets import ColorEntry
from direct.showbase.TkGlobal import spawnTkLoop
import seSceneGraphExplorer

import Pmw, sys

if sys.version_info >= (3, 0):
    from tkinter import Frame, IntVar, Checkbutton, Toplevel
    import tkinter
else:
    from Tkinter import Frame, IntVar, Checkbutton, Toplevel
    import Tkinter as tkinter


class sideWindow(AppShell):
    #################################################################
    # sideWindow(AppShell)
    # This class will open a side window wich contains a scene graph and
    # a world setting page.
    #################################################################
    appversion      = '1.0'
    appname         = 'Navigation Window'
    frameWidth      = 325
    frameHeight     = 580
    frameIniPosX    = 0
    frameIniPosY    = 110
    padx            = 0
    pady            = 0

    lightEnable = 0
    ParticleEnable = 0
    basedriveEnable = 0
    collision = 0
    backface = 0
    texture = 1
    wireframe = 0

    enableBaseUseDrive = 0

    def __init__(self, worldColor,lightEnable,ParticleEnable, basedriveEnable,collision,
                 backface, texture, wireframe, grid, widgetVis, enableAutoCamera, parent = None, nodePath = render, **kw):
        self.worldColor = worldColor
        self.lightEnable = lightEnable
        self.ParticleEnable = ParticleEnable
        self.basedriveEnable = basedriveEnable
        self.collision = collision
        self.backface = backface
        self.texture = texture
        self.wireframe = wireframe
        self.grid = grid
        self.enableAutoCamera = enableAutoCamera
        self.widgetVis = widgetVis

        # Define the megawidget options.
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        if parent == None:
            self.parent = Toplevel()
        else:
            self.parent = parent

        AppShell.__init__(self, self.parent)
        self.parent.geometry('%dx%d+%d+%d' % (self.frameWidth, self.frameHeight,self.frameIniPosX,self.frameIniPosY))

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def appInit(self):
        print('----SideWindow is Initialized!!')

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        mainFrame = Frame(interior)
        ## Creat NoteBook
        self.notebookFrame = Pmw.NoteBook(mainFrame)
        self.notebookFrame.pack(fill=tkinter.BOTH,expand=1)
        sgePage = self.notebookFrame.add('Tree Graph')
        envPage = self.notebookFrame.add('World Setting')
        self.notebookFrame['raisecommand'] = self.updateInfo

        ## Tree Grapgh Page
        self.SGE = seSceneGraphExplorer.seSceneGraphExplorer(
            sgePage, nodePath = render,
            scrolledCanvas_hull_width = 270,
            scrolledCanvas_hull_height = 570)
        self.SGE.pack(fill = tkinter.BOTH, expand = 0)

        ## World Setting Page
        envPage = Frame(envPage)
        pageFrame = Frame(envPage)
        self.LightingVar = IntVar()
        self.LightingVar.set(self.lightEnable)
        self.LightingButton = Checkbutton(
            pageFrame,
            text = 'Enable Lighting',
            variable = self.LightingVar,
            command = self.toggleLights)
        self.LightingButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.CollisionVar = IntVar()
        self.CollisionVar.set(self.collision)
        self.CollisionButton = Checkbutton(
            pageFrame,
            text = 'Show Collision Object',
            variable = self.CollisionVar,
            command = self.showCollision)
        self.CollisionButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.ParticleVar = IntVar()
        self.ParticleVar.set(self.ParticleEnable)
        self.ParticleButton = Checkbutton(
            pageFrame,
            text = 'Show Particle Dummy',
            variable = self.ParticleVar,
            command = self.enableParticle)
        self.ParticleButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.baseUseDriveVar = IntVar()
        self.baseUseDriveVar.set(self.basedriveEnable)
        self.baseUseDriveButton = Checkbutton(
            pageFrame,
            text = 'Enable base.usedrive',
            variable = self.baseUseDriveVar,
            command = self.enablebaseUseDrive)
        self.baseUseDriveButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.backfaceVar = IntVar()
        self.backfaceVar.set(self.backface)
        self.backfaceButton = Checkbutton(
            pageFrame,
            text = 'Enable BackFace',
            variable = self.backfaceVar,
            command = self.toggleBackface)
        self.backfaceButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.textureVar = IntVar()
        self.textureVar.set(self.texture)
        self.textureButton = Checkbutton(
            pageFrame,
            text = 'Enable Texture',
            variable = self.textureVar,
            command = self.toggleTexture)
        self.textureButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.wireframeVar = IntVar()
        self.wireframeVar.set(self.wireframe)
        self.wireframeButton = Checkbutton(
            pageFrame,
            text = 'Enable Wireframe',
            variable = self.wireframeVar,
            command = self.toggleWireframe)
        self.wireframeButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.gridVar = IntVar()
        self.gridVar.set(self.grid)
        self.gridButton = Checkbutton(
            pageFrame,
            text = 'Enable Grid',
            variable = self.gridVar,
            command = self.toggleGrid)
        self.gridButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.widgetVisVar = IntVar()
        self.widgetVisVar.set(self.widgetVis)
        self.widgetVisButton = Checkbutton(
            pageFrame,
            text = 'Enable WidgetVisible',
            variable = self.widgetVisVar,
            command = self.togglewidgetVis)
        self.widgetVisButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.enableAutoCameraVar = IntVar()
        self.enableAutoCameraVar.set(self.enableAutoCamera)
        self.enableAutoCameraButton = Checkbutton(
            pageFrame,
            text = 'Enable Auto Camera Movement for Loading Objects',
            variable = self.enableAutoCameraVar,
            command = self.toggleAutoCamera)
        self.enableAutoCameraButton.pack(side=tkinter.LEFT, expand=False)
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        pageFrame = Frame(envPage)
        self.backgroundColor = ColorEntry(
            pageFrame, text = 'BG Color', value=self.worldColor)
        self.backgroundColor['command'] = self.setBackgroundColorVec
        self.backgroundColor['resetValue'] = [0,0,0,0]
        self.backgroundColor.pack(side=tkinter.LEFT, expand=False)
        self.bind(self.backgroundColor, 'Set background color')
        pageFrame.pack(side=tkinter.TOP, fill=tkinter.X, expand=True)

        envPage.pack(expand=False)

        ## Set all stuff done
        self.notebookFrame.setnaturalsize()
        mainFrame.pack(fill = 'both', expand = 1)


    def createMenuBar(self):
        # We don't need menu bar here.
        self.menuBar.destroy()

    def onDestroy(self, event):
        #################################################################
        # onDestroy(self, event)
        # This function will be called when user closed the side window.
        # Here we will send out a message with whole data set we will need
        # for the next time user open the side window.
        #################################################################
        messenger.send('SW_close',[self.worldColor,
                                   self.lightEnable,
                                   self.ParticleEnable,
                                   self.basedriveEnable,
                                   self.collision,
                                   self.backface,
                                   self.texture,
                                   self.wireframe,
                                   self.grid,
                                   self.widgetVis,
                                   self.enableAutoCamera])
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

    ###############################
    def updateInfo(self, page = 'Tree Graph'):
        #################################################################
        # updateInfo(self, page = 'Tree Graph')
        # This function will be called when each time user change the main
        # page of the window.
        # What it dose is to call right function to restore the data for current selected page.
        #################################################################
        if page=='Tree Graph':
            self.updateTreeGraph()
        elif page == 'World Setting':
            self.updateWorldSetting()

    def updateTreeGraph(self):
        #################################################################
        # updateTreeGraph(self)
        # When scene graoh page has been opend, call sceneGraphExplorer to
        # updata the tree.
        #################################################################
        self.SGE.update()
        pass

    def updateWorldSetting(self):
        #################################################################
        # updateWorldSetting(self)
        # When world setting page has been selected, this function will
        # reset those check box in the page to reflect the current world setting.
        #################################################################
        self.LightingVar.set(self.lightEnable)

        self.CollisionVar.set(self.collision)
        self.ParticleVar.set(self.ParticleEnable)
        self.baseUseDriveVar.set(self.basedriveEnable)
        self.backgroundColor.set(value = self.worldColor)
        pass

    def toggleLights(self):
        #################################################################
        # toggleLights(self)
        # send out a message to let sceneEditor know we need to toggle the light.
        # Then, sceneEditor will pass the message to dataHolder to disable/enable
        # the lights. (lightManager is inside the dataHolder)
        #################################################################
        self.lightEnable = (self.lightEnable+1)%2
        messenger.send('SW_lightToggle')
        pass

    def showCollision(self):
        #################################################################
        # showCollision(self)
        # This function will send out a message to sceneEditor to toggle
        # the visibility of collision objects.
        #################################################################
        self.collision = (self.collision+1)%2
        messenger.send('SW_collisionToggle', [self.collision])
        pass

    def enableParticle(self):
        #################################################################
        # enableParticle(self)
        # This function will send out a message to sceneEditor to toggle
        # the visibility of particle objects.
        #################################################################
        self.ParticleEnable = (self.ParticleEnable+1)%2
        messenger.send('SW_particleToggle', [self.ParticleEnable])
        pass

    def enablebaseUseDrive(self):
        #################################################################
        # enablebaseUseDrive(self)
        # This function will toggle the usage of base.useDrive.
        # Well, it may not usefull at all.
        #
        # We won't send out any message in this time to notice
        # the sceneEditor this event happend.
        # In the other hand, we will restore it back when
        # the side window has been closed.
        #
        #################################################################
        if self.enableBaseUseDrive==0:
            print('Enabled')
            base.useDrive()
            self.enableBaseUseDrive = 1
        else:
            print('disabled')
            #base.useTrackball()
            base.disableMouse()
            self.enableBaseUseDrive = 0
        self.basedriveEnable = (self.basedriveEnable+1)%2
        pass

    def toggleBackface(self):
        #################################################################
        # toggleBackface(self)
        # This function will toggle the back face setting. so it will
        # render the polygon with two sides.
        #################################################################
        base.toggleBackface()
        self.backface = (self.backface+1)%2
        return

    def toggleBackfaceFromMainW(self):
        #################################################################
        # toggleBackfaceFromMainW(self)
        # This function is called by sceneEditor when user used hot key
        # to toggle the back face setting in the main panda window.
        # In here we will only reset the flag and reset the state of
        # check box
        #################################################################
        self.backface = (self.backface+1)%2
        self.backfaceButton.toggle()
        return

    def toggleTexture(self):
        #################################################################
        # toggleTexture(self)
        # This function will toggle the txture using option for the whole scene.
        #################################################################
        base.toggleTexture()
        self.texture = (self.texture+1)%2
        return

    def toggleTextureFromMainW(self):
        #################################################################
        # toggleTextureFromMainW(self)
        # This function is called by sceneEditor when user used hot key
        # to toggle the texture usage from the main panda window.
        # In here we will only reset the flag and reset the state of
        # check box
        #################################################################
        self.texture = (self.texture+1)%2
        self.textureButton.toggle()
        return

    def toggleWireframe(self):
        #################################################################
        # toggleWireframe(self)
        # This function will toggle the wire frame mode.
        #################################################################
        base.toggleWireframe()
        self.wireframe = (self.wireframe+1)%2
        return

    def toggleWireframeFromMainW(self):
        #################################################################
        # toggleWireframeFromMainW(self)
        # This function is called by sceneEditor when user used hot key
        # to toggle the wire frame mode in the main panda window.
        # In here we will only reset the flag and reset the state of
        # check box
        #################################################################
        self.wireframe = (self.wireframe+1)%2
        self.wireframeButton.toggle()
        return

    def toggleGrid(self):
        #################################################################
        # toggleGrid(self)
        # This function will toggle the usage of the grid.
        #################################################################
        self.grid = (self.grid+1)%2
        if self.grid==1:
            SEditor.grid.enable()
        else:
            SEditor.grid.disable()

    def togglewidgetVis(self):
        #################################################################
        # togglewidgetVis(self)
        # This function will toggle the visibility of the widget of the grid.
        #################################################################
        self.widgetVis = (self.widgetVis+1)%2
        SEditor.toggleWidgetVis()
        if SEditor.widget.fActive:
                messenger.send('shift-f')
        return

    def toggleWidgetVisFromMainW(self):
        #################################################################
        # toggleWidgetVisFromMainW(self)
        # This function is called by sceneEditor when user used hot key
        # to toggle the visibility of widgets ('v') from the main panda window.
        # In here we will only reset the flag and reset the state of
        # check box
        #################################################################
        self.widgetVis = (self.widgetVis+1)%2
        self.widgetVisButton.toggle()
        return

    def setBackgroundColorVec(self,color):
        #################################################################
        # setBackgroundColorVec(self,color)
        # Call back function
        # This will be called from the colorEntry on the world setting page.
        # The "color" here is a list containing three integer data, R, G and B.
        #################################################################
        base.setBackgroundColor(color[0]/255.0,
                                color[1]/255.0,
                                color[2]/255.0)
        self.worldColor = [color[0],color[1],color[2],0]

    def toggleAutoCamera(self):
        #################################################################
        # toggleAutoCamera(self)
        # This function will toggle the usage of the auto-camera movement
        # when user loaded model or actor into the scene.
        #################################################################
        self.enableAutoCamera = (self.enableAutoCamera+1)%2
        SEditor.toggleAutoCamera()
        return

    def selectPage(self,page='Tree Graph'):
        #################################################################
        #################################################################
        self.notebookFrame.selectpage(page)

