
import sys
try: import _tkinter
except: sys.exit("Please install python module 'Tkinter'")

from direct.showbase.ShowBase import ShowBase

ShowBase()

from direct.showbase.TkGlobal import spawnTkLoop

if sys.version_info >= (3, 0):
    from tkinter import *
    from tkinter.filedialog import *
else:
    from Tkinter import *
    from tkFileDialog import *

from direct.directtools.DirectGlobals import *
from direct.tkwidgets.AppShell import*

from SideWindow import*
from duplicateWindow import*
from lightingPanel import *
from seMopathRecorder import *
from seSession import *
from quad import *
from sePlacer import *
from seFileSaver import *
from propertyWindow import *
import seParticlePanel
from collisionWindow import *
from direct.gui.DirectGui import *
from MetadataPanel import *
from seBlendAnimPanel import *
from controllerWindow import *
from AlignTool import *



import os
import string
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
from direct.actor import Actor
import seAnimPanel
from direct.task import Task
import math

#################################################################
# All scene and windows object will be stored in here.
# So, any event which will or need to change contents
# should be wirtten in here or imported into here!
#################################################################
from dataHolder import*  ## Use this thing to Save/load data.
AllScene = dataHolder()



class myLevelEditor(AppShell):
    ## overridden the basic app info ##
    appname = 'Scene Editor - New Scene'
    appversion      = '1.0'
    copyright       = ('Copyright 2004 E.T.C. Carnegie Mellon U.' +
                       ' All Rights Reserved')
    contactname     = 'Jesse Schell, Shalin Shodhan & YiHong Lin'
    contactphone    = '(412) 268-5791'
    contactemail    = 'etc-panda3d@lists.andrew.cmu.edu'
    frameWidth      = 1024
    frameHeight     = 80
    frameIniPosX    = 0
    frameIniPosY    = 0
    usecommandarea = 0
    usestatusarea  = 0
    padx            = 5
    pady            = 5

    sideWindowCount = 0

    ## Basic World default setting (For side window)
    worldColor = [0,0,0,0]
    lightEnable = 1
    ParticleEnable = 1
    basedriveEnable = 0
    collision = 1
    backface = 0
    texture = 1
    wireframe = 0
    grid = 0
    widgetVis = 0
    enableAutoCamera = 1

    enableControl = False
    controlType = 'Keyboard'
    keyboardMapDict = {}
    keyboardSpeedDict = {}

    Scene=None
    isSelect = False
    nodeSelected = None
    undoDic = {}
    redoDic = {}
    animPanel = {}
    animBlendPanel = {}
    propertyWindow = {}
    CurrentFileName=None #Holds the current scene file name
    CurrentDirName=None # Holds the current file name without extension which is the path where file's data gets saved
    Dirty=0 # Keeps track of whether there are any modifications that should be saved



    def __init__(self, parent = None, **kw):

        base.setBackgroundColor(0,0,0)
        self.parent = parent
        ## Check TkTool is activated! ##
        self.wantTK = config.GetBool('want-tk', 0)
        if self.wantTK:
            pass
        else:
            taskMgr.remove('tkloop')
            spawnTkLoop()
        ## Set up window frame
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        AppShell.__init__(self, parent)
        self.parent.geometry('%dx%d+%d+%d' % (self.frameWidth, self.frameHeight,self.frameIniPosX,self.frameIniPosY))


        ###### Put th directLabel on the screen to show the selected object Data
        self.posLabel = DirectLabel(
            relief = None,
            pos = (-1.3, 0, 0.90),
            text = "Position   : X: 00.00 Y: 00.00 Z: 00.00",
            color = Vec4(1, 1, 1, 1),
            text_scale = 0.05,
            text_align = TextNode.ALeft
            )
        self.hprLabel = DirectLabel(
            relief = None,
            pos = (-1.3 , 0, 0.80),
            text = "Orientation: H: 00.00 P: 00.00 R: 00.00",
            color = Vec4(1, 1, 1, 1),
            text_scale = 0.05,
            text_align = TextNode.ALeft
            )
        self.scaleLabel = DirectLabel(
            relief = None,
            pos = (-1.3, 0, 0.70),
            text = "Scale      : X: 00.00 Y: 00.00 Z: 00.00",
            color = Vec4(1, 1, 1, 1),
            text_scale = 0.05,
            text_align = TextNode.ALeft
            )


        self.initialiseoptions(myLevelEditor)

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

        ######### Set the event handler ##########
        self.dataFlowEvents = [
            ## Event from Side Window
            ['SW_lightToggle',self.lightToggle],
            ['SW_collisionToggle',AllScene.toggleCollisionVisable],
            ['SW_particleToggle',self.toggleParticleVisable],
            ['SW_close',self.sideWindowClose],
            ## From Duplication Window
            ['DW_duplicating',self.duplicationObj],
            ## From Animation Panel
            ['AW_AnimationLoad',self.animationLoader],
            ['AW_removeAnim',self.animationRemove],
            ['AW_close',self.animPanelClose],
            ## From Blending Animation Window
            ['BAW_saveBlendAnim',self.animBlendPanelSave],
            ['BAW_removeBlendAnim',self.animBlendPanelRemove],
            ['BAW_renameBlendAnim',self.animBlendPanelRename],
            ['BAW_close',self.animBlendPanelClose],
            ## From Lighting Panel
            ['LP_selectLight', self.lightSelect],
            ['LP_addLight',self.addLight],
            ['LP_rename',self.lightRename],
            ['LP_removeLight',self.removeLight],
            ['LP_close',self.lightingPanelClose],
            ## From MotionPath Panel
            ['mPath_bindPathToNode',AllScene.bindCurveToNode],
            ['mPath_requestCurveList', self.requestCurveList],
            ['mPath_close', self.mopathClosed],
            ## From Property Window
            ['PW_removeCurveFromNode', AllScene.removeCurveFromNode],
            ['PW_removeAnimFromNode', AllScene.removeAnimation],
            ['PW_toggleLight', AllScene.toggleLightNode],
            ['PW_close', self.closePropertyWindow],
            ## From collisionWindow
            ['CW_addCollisionObj', AllScene.addCollisionObject],
            ## From AlignWindow
            ['ALW_close', self.closeAlignPanel],
            ['ALW_align', self.alignObject],
            ## From controllerWindow
            ['ControlW_close', self.closeInputPanel],
            ['ControlW_require', self.requestObjFromControlW],
            ['ControlW_controlSetting', self.setControlSet],
            ['ControlW_controlEnable', self.startControl],
            ['ControlW_controlDisable', self.stopControl],
            ['ControlW_saveSetting', AllScene.saveControlSetting],
            ## From Placer
            ['Placer_close', self.closePlacerPanel],
            ## From Particle Panel
            ['ParticlePanle_close', self.closeParticlePanel],
            ## From SEditor object which is a altered DirectSession
            ['SEditor-ToggleWidgetVis',self.toggleWidgetVis],
            ['SEditor-ToggleBackface',self.toggleBackface],
            ['SEditor-ToggleTexture',self.toggleTexture],
            ['SEditor-ToggleWireframe',self.toggleWireframe],
            ['ParticlePanel_Added_Effect',self.addParticleEffect],
            ['f11',self.loadFromBam],
            ['f12',self.saveAsBam],
            ]


        #################################
        ###  Collision detection
        #################################
        self.cTrav = CollisionTraverser()
        base.cTrav = self.cTrav

        for event in self.dataFlowEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])

        self.actionEvents = [
            # Scene graph explorer functions
            ['SGE_changeName', self.changeName],
            ['SGE_Properties', self.openPropertyPanel],
            ['SGE_Duplicate', self.duplicate],
            ['SGE_Remove', self.remove],
            ['SGE_Add Dummy', self.addDummyNode],
            ['SGE_Add Collision Object', self.addCollisionObj],
            ['SGE_Metadata', self.openMetadataPanel],
            ['SGE_Set as Reparent Target', self.setAsReparentTarget],
            ['SGE_Reparent to Target', self.reparentToNode],
            ['SGE_Animation Panel', self.openAnimPanel],
            ['SGE_Blend Animation Panel', self.openBlendAnimPanel],
            ['SGE_MoPath Panel', self.openMoPathPanel],
            ['SGE_Align Tool', self.openAlignPanel],
            ['SGE_Flash', self.flash],
            ['SGE_madeSelection', self.selectNode],
            ['select',self.selectNode],
            ['deselect', self.deSelectNode],
            ['se_selectedNodePath',self.selectFromScene],
            ['se_deselectedAll',self.deselectFromScene],
            ]
        ''' All messages starting with "SGE_" are generated in seSceneGraphExplorer'''

        for event in self.actionEvents:
            self.accept(event[0], event[1], extraArgs = event[2:])

        if camera.is_hidden():
            camera.show()
        else:
            camera.hide()
        self.selectNode(base.camera) ## Initially, we select camera as the first node...

    def appInit(self):
        #################################################################
        # appInit(self)
        # Initialize the application.
        # This function will be called when you call AppShell's constructor
        #################################################################

        ### Create SceneEditor Ver. DirectSession
        self.seSession = SeSession()
        self.seSession.enable()
        SEditor.camera.setPos(0,-50,10)

        self.placer=None
        self.MopathPanel = None
        self.alignPanelDict = {}
        #self.quadview=QuadView()


        self.lightingPanel = None
        self.controllerPanel = None
        self.particlePanel = None

        ### Create Side Window
        self.sideWindow = sideWindow(worldColor = self.worldColor,
                                     lightEnable = self.lightEnable,
                                     ParticleEnable = self.ParticleEnable,
                                     basedriveEnable = self.basedriveEnable,
                                     collision = self.collision,
                                     backface = self.backface,
                                     texture = self.texture,
                                     wireframe = self.wireframe,
                                     grid = self.grid,
                                     widgetVis = self.widgetVis,
                                     enableAutoCamera = self.enableAutoCamera)
        self.sideWindowCount = 1
        self.sideWindow.selectPage()
        messenger.send('SGE_Update Explorer',[render]) ## Update the Scene Graph

        pass

    def getPhotoImage(self,name):
        modpath = ConfigVariableSearchPath("model-path")
        path = modpath.findFile(Filename(name))
        return PhotoImage(file=path.toOsSpecific())

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()

        #######################################################
        ### Creating the Buttons in the window frame
        #######################################################
        buttonFrame = Frame(interior)
        self.image=[]

        self.image.append(self.getPhotoImage('models/icons/new.gif'))#0
        self.image.append(self.getPhotoImage('models/icons/open.gif'))#1
        self.image.append(self.getPhotoImage('models/icons/save.gif'))#2
        self.image.append(self.getPhotoImage('models/icons/model.gif'))#3
        self.image.append(self.getPhotoImage('models/icons/actor.gif'))#4
        self.image.append(self.getPhotoImage('models/icons/placer.gif'))#5
        self.image.append(self.getPhotoImage('models/icons/mopath.gif'))#6
        self.image.append(self.getPhotoImage('models/icons/lights.gif'))#7
        self.image.append(self.getPhotoImage('models/icons/particles.gif'))#8
        self.image.append(self.getPhotoImage('models/icons/control.gif'))
        self.image.append(self.getPhotoImage('models/icons/help.gif'))#9
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))
        self.image.append(self.getPhotoImage('models/icons/blank.gif'))

        i = 0
        for element in self.image:
            i += 1
            button = Button(buttonFrame, image = element, command=lambda n=i : self.buttonPushed(n))
            button.pack(fill=X, side = LEFT)


        buttonFrame.pack(fill=X, side=LEFT,expand=True)


    def buttonPushed(self, buttonIndex):
        #################################################################
        # buttonPushed(self, buttonNum)
        # This function will handle all button events from top level window
        # Take the button index as a reference to sence which button has been pushed.
        #################################################################
        ####
        ####  Change here to process the button event further.
        ####
        if buttonIndex==1: # New Scene
            self.newScene()
            return
        elif buttonIndex==2: # Open Scene
            self.openScene()
            return
        elif buttonIndex==3: # Save Scene
            self.saveScene()
            return
        elif buttonIndex==4: # Load Model
            self.loadModel()
            return
        elif buttonIndex==5: # Load Actor
            self.loadActor()
            return
        elif buttonIndex==6: # Open Placer
            self.openPlacerPanel()
            return
        elif buttonIndex==7: # Open Mopath Panel
            self.openMoPathPanel()
            return
        elif buttonIndex==8: # Open Lighting Panel
            self.openLightingPanel()
            return
        elif buttonIndex==9: # Open Particle Panel
            self.openParticlePanel()
            return
        elif buttonIndex==10:
            self.openInputPanel()
            return
        elif buttonIndex==11: # Help
            self.showAbout()
            return
        elif buttonIndex==12:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==13:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==14:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==15:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==16:
            print("Your scene will be eliminated within five seconds, Save your world!!!, Number %d."%buttonIndex)
            return
        elif buttonIndex==17:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==18:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==19:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return
        elif buttonIndex==20:
            print("You haven't defined the function for this Button, Number %d."%buttonIndex)
            return

        return

    def createMenuBar(self):
        # Creates default menus.  Can be overridden or simply augmented
        # Using button Add below
        self.menuBar.addmenuitem('Help', 'command',
                                 'Get information on application',
                                 label='About...', command=self.showAbout)
        ## Creat stuff inside the "File"
        self.menuBar.addmenuitem('File', 'command', 'Creat New Scene',
                                label='New Scene',
                                command=self.newScene)

        self.menuBar.addmenuitem('File', 'command', 'Open a Scene',
                                label='Open Scene',
                                command=self.openScene)

        self.menuBar.addmenuitem('File', 'command', 'Save a Scene',
                                label='Save Scene',
                                command=self.saveScene)

        self.menuBar.addmenuitem('File', 'command', 'Save Scene as...',
                                label='Save as...',
                                command=self.saveAsScene)

        self.menuBar.addmenuitem('File', 'separator')

        self.menuBar.addmenuitem('File', 'command', 'Load Model',
                                label='Load Model',
                                command=self.loadModel)

        self.menuBar.addmenuitem('File', 'command', 'Load Actor',
                                label='Load Actor',
                                command=self.loadActor)

        self.menuBar.addmenuitem('File', 'separator')

        self.menuBar.addmenuitem('File', 'command', 'Import a Scene',
                                label='Import...',
                                command=self.importScene)

        self.menuBar.addmenuitem('File', 'separator')

        self.menuBar.addmenuitem('File', 'command', 'Quit this application',
                                label='Exit',
                                command=self.quit)

        ## Creat "Edit" on the menu and its stuff
        self.menuBar.addmenu('Edit', 'Editting tools')
        self.menuBar.addmenuitem('Edit', 'command', 'Un-do',
                                label='Undo...',
                                command=self.unDo)
        self.menuBar.addmenuitem('Edit', 'command', 'Re-do',
                                label='Redo...',
                                command=self.reDo)
        self.menuBar.addmenuitem('Edit', 'separator')
        self.menuBar.addmenuitem('Edit', 'command', 'Deselect nodepath',
                                label='Deselect',
                                command=self.deSelectNode)
        self.menuBar.addmenuitem('Edit', 'separator')
        self.menuBar.addmenuitem('Edit', 'command', 'Add a Dummy',
                                label='Add Dummy',
                                command=self.addDummy)
        self.menuBar.addmenuitem('Edit', 'command', 'Duplicate nodepath',
                                label='Duplicate',
                                command=self.duplicateNode)
        self.menuBar.addmenuitem('Edit', 'command', 'Remove the nodepath',
                                label='Remove',
                                command=self.removeNode)
        self.menuBar.addmenuitem('Edit', 'command', 'Show the object properties',
                                label='Object Properties',
                                command=self.showObjProp)
        self.menuBar.addmenuitem('Edit', 'separator')
        self.menuBar.addmenuitem('Edit', 'command', 'Show the Camera setting',
                                label='Camera Setting',
                                command=self.showCameraSetting)
        self.menuBar.addmenuitem('Edit', 'command', 'Render setting',
                                label='Render Setting',
                                command=self.showRenderSetting)

        ## Creat "Panel" on the menu and its stuff
        self.menuBar.addmenu('Panel', 'Panel tools')
        self.menuBar.addmenuitem('Panel', 'command', 'Open Side Window',
                                label='Side Window',
                                command=self.openSideWindow)
        self.menuBar.addmenuitem('Panel', 'command', 'Placer Panel',
                                label='Placer Panel',
                                command=self.openPlacerPanel)
        self.menuBar.addmenuitem('Panel', 'command', 'Animation Panel',
                                label='Animation Panel',
                                command=self.openAnimationPanel)
        self.menuBar.addmenuitem('Panel', 'command', 'Motion Path Panel',
                                label='Mopath Panel',
                                command=self.openMopathPanel)
        self.menuBar.addmenuitem('Panel', 'command', 'Lighting Panel',
                                label='Lighting Panel',
                                command=self.openLightingPanel)
        self.menuBar.addmenuitem('Panel', 'command', 'Particle Panel',
                                label='Particle Panel',
                                command=self.openParticlePanel)
        self.menuBar.addmenuitem('Panel', 'separator')
        self.menuBar.addmenuitem('Panel', 'command', 'Input control Panel',
                                label='Input device panel',
                                command=self.openInputPanel)

        self.menuBar.pack(fill=X, side = LEFT)

        ## get "Menu" items in order to control the entry status
        self.menuFile = self.menuBar.component('File-menu')
        self.menuEdit = self.menuBar.component('Edit-menu')
        self.menuPanel = self.menuBar.component('Panel-menu')

        ## Disable entries when user doesn't select anything
        if not self.isSelect:
            self.menuEdit.entryconfig('Deselect', state=DISABLED)
            self.menuEdit.entryconfig('Add Dummy', state=DISABLED)
            self.menuEdit.entryconfig('Duplicate', state=DISABLED)
            self.menuEdit.entryconfig('Remove', state=DISABLED)
            self.menuEdit.entryconfig('Object Properties', state=DISABLED)
            self.menuPanel.entryconfig('Animation Panel', state=DISABLED)
            self.menuPanel.entryconfig('Side Window', state=DISABLED)


    def onDestroy(self, event):
        #################################################################
        # If you have open any thing, please rewrite here!
        #################################################################
        if taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                taskMgr.remove('seMonitorSelectedNode')
        pass

    def closeAllSubWindows(self):
        #################################################################
        # closeAllSubWindows(self)
        # except side window. this function will close all sub window if there is any.
        #################################################################
        if self.lightingPanel != None:
            self.lightingPanel.quit()
        if self.placer != None:
            self.placer.quit()
        if self.MopathPanel != None:
            self.MopathPanel.quit()

        if self.particlePanel != None:
            self.particlePanel.quit()

        if self.controllerPanel != None:
            self.controllerPanel.quit()

        list = self.animPanel.keys()
        for index in list:
            self.animPanel[index].quit()
        list = self.animBlendPanel.keys()
        for index in list:
            self.animBlendPanel[index].quit()
        list = self.propertyWindow.keys()
        for index in list:
            self.propertyWindow[index].quit()
        list = self.alignPanelDict.keys()
        for index in list:
            self.alignPanelDict[index].quit()
        self.animPanel.clear()
        self.animBlendPanel.clear()
        self.propertyWindow.clear()
        self.alignPanelDict.clear()

        return

    ## Processing message events

    def makeDirty(self):
        self.Dirty=1

    def removeLight(self, lightNode):
        #################################################################
        # removeLight(self, lightNode)
        # This function will be called when user try to remove the light from lightingPanel
        # (by sending out the message)
        # So, in here we will call dataHolder(AllScene) to remove the light
        # and return a list contains the newest data of lights in he scene.
        # Then, this function will reset the lighting list in the lightingPanel
        #################################################################
        list = AllScene.removeObj(lightNode)
        if self.lightingPanel != None:
            self.lightingPanel.updateList(list)
        return

    def lightRename(self,oName, nName):
        #################################################################
        # lightRename(self,oName, nName)
        # This function will be called when user try to rename the light from lightingPanel
        # (by sending out the message)
        # So, in here we will call dataHolder(AllScene) to rename the light
        # and return a list contains the newest data of lights in he scene.
        # Then, this function will reset the lighting list in the lightingPanel
        #################################################################
        list, lightNode = AllScene.rename(oName, nName)
        if self.lightingPanel != None:
            self.lightingPanel.updateList(list,lightNode)
        return

    def lightSelect(self,lightName):
        #################################################################
        # lightSelect(self,lightName)
        # This function will be called when user try to select the light from lightingPanel
        # (by sending out the message)
        # So, in here we will call dataHolder(AllScene) to get the target light node
        # Then, this function will put this light node back into lighting
        # panel and update the data on the panel.
        #################################################################
        lightNode = AllScene.getLightNode(lightName)
        if self.lightingPanel != None:
            self.lightingPanel.updateDisplay(lightNode)
        return

    def addLight(self, type):
        #################################################################
        # addLight(self, type)
        # This function will be called when user try to add a light from lightingPanel
        # (by sending out the message)
        # So, in here we will call dataHolder(AllScene) to create a default light node
        # by the type that user assigned.
        # Then, this function will put this light node back into lighting
        # panel with the newest lighting list and update the data on the panel.
        #################################################################
        list, lightNode = AllScene.createLight(type = type)
        if self.lightingPanel != None:
            self.lightingPanel.updateList(list,lightNode)
        self.makeDirty()
        return

    def lightingPanelClose(self):
        #################################################################
        # lightingPanelClose(self)
        # This function will be called when user try to close the lighting panel
        # This function will re-config the state of the lighting panel button on the top screen
        # And it will set the self.lightingPanel to None
        #################################################################
        self.menuPanel.entryconfig('Lighting Panel', state=NORMAL)
        self.lightingPanel = None
        return

    def openPropertyPanel(self, nodePath = None):
        #################################################################
        # openPropertyPanel(self, nodePath = None)
        # This function will be called when user try to open a property window
        # for one specific node in the scene.
        # Here we will call dataHolder to get the basic properties
        # we would like to let user to see and cange.
        # And then we pass those information into propertyWindow
        #################################################################
        type, info = AllScene.getInfoOfThisNode(nodePath)
        name = nodePath.getName()
        if name not in self.propertyWindow:
            self.propertyWindow[name] = propertyWindow(nodePath, type,info )
        pass

    def closePropertyWindow(self, name):
        if name in self.propertyWindow:
            del self.propertyWindow[name]
        return

    def openMetadataPanel(self,nodePath=None):
        print(nodePath)
        self.MetadataPanel=MetadataPanel(nodePath)
        pass

    def duplicate(self, nodePath = None):
        #################################################################
        # duplicate(self, nodePath = None)
        # This function will be called when user try to open the duplication window
        #################################################################
        print('----Duplication!!')
        if nodePath != None:
            self.duplicateWindow = duplicateWindow(nodePath = nodePath)
        pass

    def remove(self, nodePath = None):
        #################################################################
        # remove(self, nodePath = None)
        # This function will be called when user try to delete a node from scene
        #
        # For safty issue,
        # we will do deselect first then remove the certain node.
        #
        #################################################################
        if nodePath==None:
            if self.nodeSelected == None:
                return
            nodePath = self.nodeSelected
        self.deSelectNode()
        if AllScene.isLight(nodePath.getName()):
            self.removeLight(nodePath)
        else:
            AllScene.removeObj(nodePath)
        pass

    def addDummyNode(self, nodepath = None):
        #################################################################
        # addDummyNode(self, nodepath = None)
        # This function will be called when user try to create a dummy node into scene
        #
        # Here we will call dataHolder to create a dummy node
        # and reparent it to the nodePath that user has assigned.
        #
        #################################################################
        AllScene.addDummyNode(nodepath)
        self.makeDirty()
        pass

    def addCollisionObj(self, nodepath = None):
        #################################################################
        # addCollisionObj(self, nodepath = None)
        # This function will be called when user try to create a collision object into the scene
        #
        # Here we will call collisionWindow to ask user what kind of collision objects they want to have.
        # Then, send the information and generated collision object to dataHolder to finish the whole process
        # and reparent it to the nodePath that user has assigned.
        #
        #################################################################
        self.collisionWindow = collisionWindow(nodepath)
        pass

    def setAsReparentTarget(self, nodepath = None):
        #################################################################
        # setAsReparentTarget(self, nodepath = None)
        # This function will be called when user select a nodePaht
        # and want to reparent other node under it. (Drom side window pop-up nemu)
        #################################################################
        SEditor.setActiveParent(nodepath)
        return

    def reparentToNode(self, nodepath = None):
        #################################################################
        # reparentToNode(self, nodepath = None)
        # This function will be call when user try to reparent a node to
        # that node he selected as a reparent target before.
        #
        # The whole reparent process is handled by seSession,
        # which is tunned from DirectSession
        #
        #################################################################
        SEditor.reparent(nodepath, fWrt = 1)
        return

    def openPlacerPanel(self, nodePath = None):
        #################################################################
        # openPlacerPanel(self, nodePath = None)
        # This function will be call when user try to open a placer panel.
        # This call will only success if there is no other placer panel been activated
        #################################################################
        if(self.placer==None):
            self.placer = Placer()
            self.menuPanel.entryconfig('Placer Panel', state=DISABLED)
        return

    def closePlacerPanel(self):
        #################################################################
        # closePlacerPanel(self)
        # This function will be called when user close the placer panel.
        # Here we will reset the self.placer back to None.
        # (You can think this is just like a reference count)
        #################################################################
        self.placer = None
        self.menuPanel.entryconfig('Placer Panel', state=NORMAL)
        return

    def openAnimPanel(self, nodePath = None):
        #################################################################
        # openAnimPanel(self, nodePath = None)
        # This function will be called when user tries to open an Animation Panel
        # This will generated a panel and put it
        # into a dictionary using the actor's name as an index.
        # So, if there already has an animation panel for the target actor,
        # it won't allow user to open another one.
        #################################################################
        name = nodePath.getName()
        if AllScene.isActor(name):
            if name in self.animPanel:
                print('---- You already have an animation panel for this Actor!')
                return
            else:
                Actor = AllScene.getActor(name)
                self.animPanel[name] = seAnimPanel.AnimPanel(aNode=Actor)
                pass

    def openMoPathPanel(self, nodepath = None):
        #################################################################
        # openMoPathPanel(self, nodepath = None)
        # This function will open a Motion Path Recorder for you.
        #################################################################
        if self.MopathPanel == None:
            self.MopathPanel = MopathRecorder()
        pass

    def mopathClosed(self):
        self.MopathPanel = None
        return

    def changeName(self, nodePath, nName):
        #################################################################
        # changeName(self, nodePath, nName)
        # This function will be called when user tries to change the name of the node
        #################################################################
        oName = nodePath.getName() # I need this line in order to check the obj name in the control panel.
        AllScene.rename(nodePath,nName)

        # reset the list in the controller panel if it has been opened.
        if (self.controllerPanel) != None:
            list = AllScene.getAllObjNameAsList()
            self.controllerPanel.resetNameList(list = list, name = oName, nodePath = nodePath)
        return

    # Take care things under File menu
    def newScene(self):
        #################################################################
        # newScene(self)
        # This function will clear whole stuff in the scene
        # and will reset the application title to "New Scene"
        #################################################################
        self.closeAllSubWindows() ## Close all sub window
        if(self.CurrentFileName):
            currentF=Filename(self.CurrentFileName)
            self.CurrentFileName=None
            AllScene.resetAll()
            currentModName=currentF.getBasenameWoExtension()
            # Let us actually remove the scene from sys modules... this is done because every scene is loaded as a module
            # And if we reload a scene python wont reload since its already in sys.modules... and hence we delete it
            # If there is ever a garbage colleciton bug..this might be a point to look at
            if currentModName in sys.modules:
                del sys.modules[currentModName]
                print(sys.getrefcount(AllScene.theScene))
                del AllScene.theScene
        else:
            AllScene.resetAll()
        self.parent.title('Scene Editor - New Scene')
        pass

    def openScene(self):
        #################################################################
        # openScene(self)
        #################################################################
        # In the future try and provide merging of two scenes

        if(self.CurrentFileName or self.Dirty):
            saveScene = tkMessageBox._show("Load scene","Save the current scene?",icon = tkMessageBox.QUESTION,type = tkMessageBox.YESNOCANCEL)
            if (saveScene == "yes"):
                self.saveScene()
            elif (saveScene == "cancel"):
                return

        self.closeAllSubWindows() ## Close all sub window
        if(self.CurrentFileName):
            currentF=Filename(self.CurrentFileName)
            AllScene.resetAll()
            currentModName=currentF.getBasenameWoExtension()
            # Let us actually remove the scene from sys modules... this is done because every scene is loaded as a module
            # And if we reload a scene python wont reload since its already in sys.modules... and hence we delete it
            # If there is ever a garbage colleciton bug..this might be a point to look at
            if currentModName in sys.modules:
                del sys.modules[currentModName]
                print(sys.getrefcount(AllScene.theScene))
                del AllScene.theScene
        else:
            AllScene.resetAll()

        self.CurrentFileName = AllScene.loadScene()

        if(self.CurrentFileName==None):
            return

        thefile=Filename(self.CurrentFileName)
        thedir=thefile.getFullpathWoExtension()
        print("SCENE EDITOR::" + thedir)
        self.CurrentDirName=thedir
        if self.CurrentFileName != None:
            self.parent.title('Scene Editor - '+ Filename.fromOsSpecific(self.CurrentFileName).getBasenameWoExtension())
        if self.lightingPanel !=None:
            lightList=AllScene.getList()
            self.lightingPanel.updateList(lightList)
        messenger.send('SGE_Update Explorer',[render])


        # Close the side window in order to reset all world settings to fit the scene we have loaded.
        self.sideWindow.quit()

        # Try to re-open the side window again
        while self.sideWindow == None:
            wColor = base.getBackgroundColor()
            self.worldColor[0] = wColor.getX()
            self.worldColor[1] = wColor.getY()
            self.worldColor[2] = wColor.getZ()
            self.worldColor[3] = wColor.getW()
            self.lightEnable = 1
            self.ParticleEnable = 1
            self.collision = 1
            self.openSideWindow()

    def saveScene(self):
        #################################################################
        # saveScene(self)
        # If this is an open file call saveAsScene
        # or else instantiate FileSaver from seFileSaver.py and pass it the filename
        # If this filename exists in sys.modules you cannot use it
        #################################################################

        if(self.CurrentFileName):
            f=FileSaver()
            f.SaveFile(AllScene,self.CurrentFileName,self.CurrentDirName,1)
            self.Dirty=0
        else:
            self.saveAsScene()
        pass

    def saveAsBam(self):
        fileName = tkFileDialog.asksaveasfilename(filetypes = [("BAM",".bam")],title = "Save Scenegraph as Bam file")
        theScene=render.find("**/Scene")
        if not theScene is None:
            theScene.writeBamFile(fileName)
        else:
            render.writeBamFile(fileName+".bad")
        print(" Scenegraph saved as :" +str(fileName))

    def loadFromBam(self):
        fileName = tkFileDialog.askopenfilename(filetypes = [("BAM",".bam")],title = "Load Scenegraph from Bam file")
        if not fileName is None:
            d=path(fileName)
            scene=loader.loadModel(d.relpath())
            scene.reparentTo(render)

    def saveAsScene(self):
        #################################################################
        # saveAsScene(self)
        # Ask for filename using a file save dialog
        # If this filename exists in sys.modules you cannot use it
        # Instantiate FileSaver from seFileSaver.py and pass it the filename
        #################################################################

        fileName = tkFileDialog.asksaveasfilename(filetypes = [("PY","py")],title = "Save Scene")
        if(not fileName):
            return
        fCheck=Filename(fileName)
        #print fCheck.getBasenameWoExtension()
        ###############################################################################
        # !!!!! See if a module exists by this name... if it does you cannot use this filename !!!!!
        ###############################################################################
        if(fCheck.getBasenameWoExtension() in sys.modules):
            tkMessageBox.showwarning(
            "Save file",
            "Cannot save with this name because there is a system module with the same name. Please resave as something else."
                )

            return
        self.CurrentDirName=fileName
        fileName=fileName+".py"
        f=FileSaver()
        self.CurrentFileName=fileName
        f.SaveFile(AllScene,fileName,self.CurrentDirName,0)
        self.Dirty=0
        self.parent.title('Scene Editor - '+ Filename.fromOsSpecific(self.CurrentFileName).getBasenameWoExtension())
        pass

    def loadModel(self):
        #################################################################
        # loadModel(self)
        # This function will be called when user tries to load a model into the scene.
        # Here we will pop-up a dialog to ask user which model file should be loaded in.
        # Then, pass the path to dataHolder to load the model in.
        #################################################################
        modelFilename = askopenfilename(
            defaultextension = '.egg',
            filetypes = (('Egg Files', '*.egg'),
                         ('Bam Files', '*.bam'),
                         ('All files', '*')),
            initialdir = '.',
            title = 'Load New Model',
            parent = self.parent)
        if modelFilename:
            self.makeDirty()
            if not AllScene.loadModel(modelFilename, Filename.fromOsSpecific(modelFilename)):
                print('----Error! No Such Model File!')
        pass

    def loadActor(self):
        #################################################################
        # loadActor(self)
        # This function will be called when user tries to load an Actor into the scene.
        # Here we will pop-up a dialog to ask user which Actor file should be loaded in.
        # Then, pass the path to dataHolder to load the Actor in.
        #################################################################
        ActorFilename = askopenfilename(
            defaultextension = '.egg',
            filetypes = (('Egg Files', '*.egg'),
                         ('Bam Files', '*.bam'),
                         ('All files', '*')),
            initialdir = '.',
            title = 'Load New Actor',
            parent = self.parent)


        if ActorFilename:
            self.makeDirty()
            if not AllScene.loadActor(ActorFilename, Filename.fromOsSpecific(ActorFilename)):
                print('----Error! No Such Model File!')
        pass

    def importScene(self):
        self.makeDirty()
        print('----God bless you Please Import!')
        pass


    ## Take care those things under Edit nemu
    def unDo(self):
        pass

    def reDo(self):
        pass

    def deSelectNode(self, nodePath=None):
        #################################################################
        # deSelectNode(self, nodePath=None)
        # This function will deselect the node which we have selected currently.
        # This will also remove the monitor task which monitor selected object's
        # position, orientation and scale each frame.
        #################################################################
        if nodePath != None:
            self.seSession.deselect(nodePath)
        if self.isSelect:
            self.isSelect = False
            #if self.nodeSelected != None:
            #    self.nodeSelected.hideBounds()
            self.nodeSelected =None
            self.menuEdit.entryconfig('Deselect', state=DISABLED)
            self.menuEdit.entryconfig('Add Dummy', state=DISABLED)
            self.menuEdit.entryconfig('Duplicate', state=DISABLED)
            self.menuEdit.entryconfig('Remove', state=DISABLED)
            self.menuEdit.entryconfig('Object Properties', state=DISABLED)
            if self.sideWindowCount==1:
                self.sideWindow.SGE.deSelectTree()
            if taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                taskMgr.remove('seMonitorSelectedNode')
            return
        pass

    def addDummy(self):
        #################################################################
        # addDummy(self)
        # This function will do nothing but call other function
        # to add a dummy into the scene.
        #
        # Ok... this is really redundancy...
        #
        #################################################################
        self.addDummyNode(self.nodeSelected)
        pass

    def duplicateNode(self):
        #################################################################
        # duplicateNode(self)
        # This function will do nothing but call other function
        # to open the duplication window.
        #
        # Ok... this is really redundancy...
        #
        #################################################################
        if self.nodeSelected!=None:
            self.duplicate(self.nodeSelected)
        pass

    def removeNode(self):
        #################################################################
        # removeNode(self)
        # This function will do nothing but call other function
        # to remove the current selected node..
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        self.remove(self.nodeSelected)
        pass

    def showObjProp(self):
        ################################################################
        # showObjProp(self)
        # This function will do nothing but call other function
        # to open the property window of current selected node..
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        self.openPropertyPanel(self.nodeSelected)
        pass

    def showCameraSetting(self):
        ################################################################
        # showCameraSetting(self)
        # This function will do nothing but call other function
        # to open the property window of camera..
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        self.openPropertyPanel(camera)
        pass

    def showRenderSetting(self):
        '''Currently, no idea what gonna pop-out here...'''
        pass

    ## Take care those thins under Edit nemu
    def openSideWindow(self):
        ################################################################
        # openSideWindow(self)
        # This function will open the side window and set the reference number
        # so that we can make sure there won't have two or more side windows in the same time.
        ################################################################
        if self.sideWindowCount==0:
            self.sideWindow = sideWindow(worldColor = self.worldColor,
                                         lightEnable = self.lightEnable,
                                         ParticleEnable = self.ParticleEnable,
                                         basedriveEnable = self.basedriveEnable,
                                         collision = self.collision,
                                         backface = self.backface,
                                         texture = self.texture,
                                         wireframe = self.wireframe,
                                         grid = self.grid,
                                         widgetVis = self.widgetVis,
                                         enableAutoCamera = self.enableAutoCamera)
            self.sideWindowCount = 1
            self.menuPanel.entryconfig('Side Window', state=DISABLED)
        return

    def openAnimationPanel(self):
        ################################################################
        # openAnimationPanel(self)
        # This function will do nothing but call other function
        # to open the animation window for selected node(if it is an Actor)..
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        if AllScene.isActor(self.nodeSelected):
            self.openAnimPanel(self.nodeSelected)
        pass

    def openMopathPanel(self):
        ################################################################
        # openMopathPanel(self)
        # This function will create a Motion Path Recorder
        ################################################################
        MopathPanel = MopathRecorder()
        pass

    def toggleParticleVisable(self, visable):
        ################################################################
        # toggleParticleVisable(self, visable)
        # This function will be called each time user has toggled
        # the check box of Particle visibility in the side window.
        # The reason we keep track this is because
        # we have to know we should show/hide the model on the new-created particle
        ################################################################
        self.ParticleEnable = visable
        AllScene.toggleParticleVisable(visable)
        return

    def openLightingPanel(self):
        ################################################################
        # openLightingPanel(self)
        # open the lighting panel here.
        # If there is already exist a lighting panel, then do nothing
        ################################################################
        if self.lightingPanel==None:
            self.lightingPanel = lightingPanel(AllScene.getLightList())
            self.menuPanel.entryconfig('Lighting Panel', state=DISABLED)
        return

    def addParticleEffect(self,effect_name,effect,node):
        AllScene.particleDict[effect_name]=effect
        AllScene.particleNodes[effect_name]=node
        if not self.ParticleEnable:
            AllScene.particleNodes[effect_name].setTransparency(True)
            AllScene.particleNodes[effect_name].setAlphaScale(0)
            AllScene.particleNodes[effect_name].setBin("fixed",1)
        return

    def openParticlePanel(self):
        if self.particlePanel != None:
            ## There already has a Particle panel!
            return
        if(len(AllScene.particleDict)==0):
            self.particlePanel=seParticlePanel.ParticlePanel()
        else:
            for effect in AllScene.particleDict:
                theeffect=AllScene.particleDict[effect]
            self.particlePanel=seParticlePanel.ParticlePanel(particleEffect=theeffect,effectsDict=AllScene.particleDict)

        pass

    def closeParticlePanel(self):
        self.particlePanel = None
        return

    def openInputPanel(self):
        if self.controllerPanel==None:
            list = AllScene.getAllObjNameAsList()
            type, dataList = AllScene.getControlSetting()
            self.controllerPanel = controllerWindow(listOfObj = list, controlType = type, dataList = dataList)
        pass

    def closeInputPanel(self):
        self.controllerPanel = None
        return

    def requestObjFromControlW(self, name):
        ################################################################
        # requestObjFromControlW(self, name)
        # Call back function
        # Each time when user selects a node from Control Panel,
        # this function will be called.
        # This function will get the actual nodePath from dataHolder and then
        # set it back into controller panel
        ################################################################
        node = AllScene.getObjFromSceneByName(name)

        if (self.controllerPanel) != None and (node!=None):
            self.controllerPanel.setNodePathIn(node)

        return

    def setControlSet(self, controlType, dataList):
        if controlType == 'Keyboard':
            self.controlTarget = dataList[0]
            self.keyboardMapDict.clear()
            self.keyboardMapDict = dataList[1].copy()
            self.keyboardSpeedDict.clear()
            self.keyboardSpeedDict = dataList[2].copy()
        return

    def startControl(self, controlType, dataList):
        if not self.enableControl:
            self.enableControl = True
        else:
            # Stop the current control setting first
            # Also this will make sure we won't catch wrong keyboard message
            self.stopControl(controlType)
            self.enableControl = True

        self.setControlSet(controlType, dataList)
        self.lastContorlTimer = globalClock.getFrameTime()
        if controlType == 'Keyboard':
            self.controlType = 'Keyboard'
            self.keyControlEventDict = {}
            self.transNodeKeyboard = self.controlTarget.attachNewNode('transformNode')
            self.transNodeKeyboard.hide()
            for index in self.keyboardMapDict:
                self.keyControlEventDict[index] = 0
                self.accept(self.keyboardMapDict[index], lambda a = index:self.keyboardPushed(a))
                self.accept(self.keyboardMapDict[index]+'-up', lambda a = index:self.keyboardReleased(a))
        return

    def stopControl(self, controlType):
        if not self.enableControl:
            return
        if controlType == 'Keyboard':
            self.enableControl = False
            for index in self.keyboardMapDict:
                self.ignore(self.keyboardMapDict[index])
                self.ignore(self.keyboardMapDict[index]+'-up')
            taskMgr.remove("KeyboardControlTask")
            self.transNodeKeyboard.removeNode()
        return

    def keyboardPushed(self, key):
        self.keyControlEventDict[key] = 1
        if not taskMgr.hasTaskNamed("KeyboardControlTask"):
            self.keyboardLastTimer = globalClock.getFrameTime()
            taskMgr.add(self.keyboardControlTask, "KeyboardControlTask")
        return

    def keyboardReleased(self, key):
        self.keyControlEventDict[key] = 0
        for index in self.keyControlEventDict:
            if self.keyControlEventDict[index] == 1:
                return
        if taskMgr.hasTaskNamed("KeyboardControlTask"):
            taskMgr.remove("KeyboardControlTask")
        return

    def keyboardControlTask(self, task):
        newTimer = globalClock.getFrameTime()
        delta = newTimer - self.keyboardLastTimer
        self.keyboardLastTimer = newTimer
        pos = self.controlTarget.getPos()
        hpr = self.controlTarget.getHpr()
        scale = self.controlTarget.getScale()
        self.transNodeKeyboard.setPosHpr((self.keyControlEventDict['KeyRight']*self.keyboardSpeedDict['SpeedRight']-self.keyControlEventDict['KeyLeft']*self.keyboardSpeedDict['SpeedLeft'])*delta,
                                         (self.keyControlEventDict['KeyForward']*self.keyboardSpeedDict['SpeedForward']-self.keyControlEventDict['KeyBackward']*self.keyboardSpeedDict['SpeedBackward'])*delta,
                                         (self.keyControlEventDict['KeyUp']*self.keyboardSpeedDict['SpeedUp']-self.keyControlEventDict['KeyDown']*self.keyboardSpeedDict['SpeedDown'])*delta,
                                         (self.keyControlEventDict['KeyTurnLeft']*self.keyboardSpeedDict['SpeedTurnLeft']-self.keyControlEventDict['KeyTurnRight']*self.keyboardSpeedDict['SpeedTurnRight'])*delta,
                                         (self.keyControlEventDict['KeyTurnUp']*self.keyboardSpeedDict['SpeedTurnUp']-self.keyControlEventDict['KeyTurnDown']*self.keyboardSpeedDict['SpeedTurnDown'])*delta,
                                         (self.keyControlEventDict['KeyRollLeft']*self.keyboardSpeedDict['SpeedRollLeft']-self.keyControlEventDict['KeyRollRight']*self.keyboardSpeedDict['SpeedRollRight'])*delta)
        newPos = self.transNodeKeyboard.getPos(self.controlTarget.getParent())
        newHpr = self.transNodeKeyboard.getHpr(self.controlTarget.getParent())
        overAllScale = self.keyControlEventDict['KeyScaleUp']*self.keyboardSpeedDict['SpeedScaleUp']-self.keyControlEventDict['KeyScaleDown']*self.keyboardSpeedDict['SpeedScaleDown']
        newScale = Point3(scale.getX() + (overAllScale + self.keyControlEventDict['KeyScaleXUp']*self.keyboardSpeedDict['SpeedScaleXUp'] - self.keyControlEventDict['KeyScaleXDown']*self.keyboardSpeedDict['SpeedScaleXDown'])*delta,
                          scale.getY() + (overAllScale + self.keyControlEventDict['KeyScaleYUp']*self.keyboardSpeedDict['SpeedScaleYUp'] - self.keyControlEventDict['KeyScaleYDown']*self.keyboardSpeedDict['SpeedScaleYDown'])*delta,
                          scale.getZ() + (overAllScale + self.keyControlEventDict['KeyScaleZUp']*self.keyboardSpeedDict['SpeedScaleZUp'] - self.keyControlEventDict['KeyScaleZDown']*self.keyboardSpeedDict['SpeedScaleZDown'])*delta
                          )
        self.controlTarget.setPos(newPos.getX(), newPos.getY() , newPos.getZ())
        self.controlTarget.setHpr(newHpr.getX(), newHpr.getY() , newHpr.getZ())
        self.controlTarget.setScale(newScale.getX(),newScale.getY(),newScale.getZ())
        self.transNodeKeyboard.setPosHpr(0,0,0,0,0,0)
        return Task.cont

    ## Misc
    ##### This one get the event from SGE (Scene Graph Explorer) and Picking
    def selectNode(self, nodePath=None, callBack = True):
        ################################################################
        # selectNode(self, nodePath=None, callBack = True)
        # This will be called when user try to select nodes from the
        # side window.
        # It will also call seSession to select this node in order to keep data's consistency
        ################################################################
        if nodePath==None:
            self.isSelect = False
            self.nodeSelected =None
            if taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                taskMgr.remove('seMonitorSelectedNode')
            return
        else:
            self.isSelect = True
            #if self.nodeSelected != None:
            #    self.nodeSelected.hideBounds()
            self.nodeSelected = nodePath
            #self.nodeSelected.showBounds()
            self.menuEdit.entryconfig('Deselect', state=NORMAL)
            self.menuEdit.entryconfig('Add Dummy', state=NORMAL)
            self.menuEdit.entryconfig('Duplicate', state=NORMAL)
            self.menuEdit.entryconfig('Remove', state=NORMAL)
            self.menuEdit.entryconfig('Object Properties', state=NORMAL)
            if callBack:
                self.seSession.select(nodePath,fResetAncestry=1)
            messenger.send('SGE_Update Explorer',[render])
            if not taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                self.oPos = self.nodeSelected.getPos()
                self.oHpr = self.nodeSelected.getHpr()
                self.oScale = self.nodeSelected.getScale()
                taskMgr.add(self.monitorSelectedNodeTask, 'seMonitorSelectedNode')
            return
        pass

    def selectFromScene(self, nodePath=None, callBack=True):
        ################################################################
        # selectFromScene(self, nodePath=None, callBack = True)
        # This will be called when user try to select nodes from the
        # scene. (By picking)
        # Actually this will be called by seSession
        # The reason we make two selections is we don't want they call each other and never stop...
        ################################################################
        if nodePath==None:
            self.isSelect = False
            self.nodeSelected =None
            if taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                taskMgr.remove('seMonitorSelectedNode')
            return
        else:
            self.isSelect = True
            #if self.nodeSelected != None:
            #    self.nodeSelected.hideBounds()
            self.nodeSelected = nodePath
            #self.nodeSelected.showBounds()
            self.menuEdit.entryconfig('Deselect', state=NORMAL)
            self.menuEdit.entryconfig('Add Dummy', state=NORMAL)
            self.menuEdit.entryconfig('Duplicate', state=NORMAL)
            self.menuEdit.entryconfig('Remove', state=NORMAL)
            self.menuEdit.entryconfig('Object Properties', state=NORMAL)
            self.sideWindow.SGE.selectNodePath(nodePath,callBack)
            messenger.send('SGE_Update Explorer',[render])
            if not taskMgr.hasTaskNamed('seMonitorSelectedNode'):
                self.oPos = self.nodeSelected.getPos()
                self.oHpr = self.nodeSelected.getHpr()
                self.oScale = self.nodeSelected.getScale()
                taskMgr.add(self.monitorSelectedNodeTask, 'seMonitorSelectedNode')
            return
        pass

    def monitorSelectedNodeTask(self, task):
        ################################################################
        # monitorSelectedNodeTask(self, task)
        # This is a function which will keep tracking
        # the position, orientation and scale data of selected node and update the display on the screen.
        # Alos, it will send out message to sychronize the data in the placer and property window.
        ################################################################
        if self.nodeSelected != None:
            pos = self.nodeSelected.getPos()
            hpr = self.nodeSelected.getHpr()
            scale = self.nodeSelected.getScale()
            if ((self.oPos != pos )or(self.oScale != scale)or(self.oHpr != hpr)):
                messenger.send('forPorpertyWindow'+self.nodeSelected.getName(),[pos, hpr, scale])
                messenger.send('placerUpdate')
                self.oPos = pos
                self.oScale = scale
                self.oHpr = hpr
                self.posLabel['text'] = "Position   : X: %2.2f Y: %2.2f Z: %2.2f"%(pos.getX(), pos.getY(),pos.getZ())
                self.hprLabel['text'] = "Orientation: H: %2.2f P: %2.2f R: %2.2f"%(hpr.getX(), hpr.getY(),hpr.getZ())
                self.scaleLabel['text'] = "Scale      : X: %2.2f Y: %2.2f Z: %2.2f"%(scale.getX(), scale.getY(),scale.getZ())
        return Task.cont

    def deselectFromScene(self):
        ################################################################
        # deselectFromScene(self)
        # This function will do nothing but call other function
        # to delete selected node...
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        self.deSelectNode(self.nodeSelected)
        messenger.send('SGE_Update Explorer',[render])

    ##### Take care the even quest from Side Window
    def lightToggle(self):
        ################################################################
        # lightToggle(self)
        # This function will do nothing but call other function
        # to toggle the light...
        ################################################################
        self.makeDirty()
        AllScene.toggleLight()
        return

    def sideWindowClose(self,worldColor,lightEnable,ParticleEnable, basedriveEnable,collision,
                        backface, texture, wireframe, grid, widgetVis, enableAutoCamera):
        ################################################################
        # sideWindowClose(self,worldColor,lightEnable,ParticleEnable, basedriveEnable,collision,
        #                 backface, texture, wireframe, grid, widgetVis, enableAutoCamera):
        # This function will be called when user close the side window.
        # Here we will restore all parameters about world setting back in the sceneEditor.
        # So, when next time people recall the side window, it will still keep the same world setting.
        ################################################################
        if self.sideWindowCount==1:
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
            self.sideWindowCount=0
            self.sideWindow = None
            self.menuPanel.entryconfig('Side Window', state=NORMAL)
            return

    ## Process message from Duplication Window
    def duplicationObj(self, nodePath, pos, hpr, scale, num):
        ################################################################
        # duplicationObj(self, nodePath, pos, hpr, scale, num)
        # This function will do nothing but call other function
        # to duplicate selected node...
        #
        # Ok... this is really redundancy...
        #
        ################################################################
        AllScene.duplicateObj(nodePath, pos, hpr, scale, num)
        return

    ## Process message from Animation Panel
    def animationLoader(self, nodePath, Dic):
        name = nodePath.getName()
        AllScene.loadAnimation(name, Dic)
        return

    def animationRemove(self, nodePath, name):
        AllScene.removeAnimation(nodePath.getName(),name)
        return

    def animPanelClose(self, name):
        if name in self.animPanel:
            del self.animPanel[name]
        return

    ### Blend Animation Panel
    def openBlendAnimPanel(self, nodePath=None):
        ################################################################
        # openBlendAnimPanel(self, nodePath=None)
        # This function will get the user defined blending animation data from dataHolder.
        # And then open a blendAnimPanel by passing those data in.
        ################################################################
        name = nodePath.getName()
        if AllScene.isActor(name):
            if name in self.animBlendPanel:
                print('---- You already have an Blend Animation Panel for this Actor!')
                return
            else:
                Actor = AllScene.getActor(name)
                Dict = AllScene.getBlendAnimAsDict(name)
                self.animBlendPanel[name] = BlendAnimPanel(aNode=Actor, blendDict=Dict)
                pass
        return

    def animBlendPanelSave(self, actorName, blendName, animNameA, animNameB, effect):
        ################################################################
        # animBlendPanelSave(self, actorName, blendName, animNameA, animNameB, effect)
        # This function will call dataHolder to save the blended animation.
        # Then, it will reset the newest blended animation list back to animBlendPanel
        ################################################################
        dict = AllScene.saveBlendAnim(actorName, blendName, animNameA, animNameB, effect)
        self.animBlendPanel[actorName].setBlendAnimList(dict)
        return

    def animBlendPanelRemove(self, actorName, blendName):
        ################################################################
        # animBlendPanelRemove(self, actorName, blendName)
        # This function will call dataHolder to remove the blended animation.
        # Then, it will reset the newest blended animation list back to animBlendPanel
        ################################################################
        dict = AllScene.removeBlendAnim(actorName, blendName)
        self.animBlendPanel[actorName].setBlendAnimList(dict, True)
        return

    def animBlendPanelRename(self, actorName, nName, oName, animNameA, animNameB, effect):
        ################################################################
        # animBlendPanelRename(self, actorName, nName, oName, animNameA, animNameB, effect)
        # This function will call dataHolder to rename the blended animation.
        # Then, it will reset the newest blended animation list back to animBlendPanel
        ################################################################
        dict = AllScene.renameBlendAnim(actorName, nName, oName, animNameA, animNameB, effect)
        self.animBlendPanel[actorName].setBlendAnimList(dict)
        return

    def animBlendPanelClose(self, name):
        ################################################################
        # animBlendPanelClose(self, name)
        # This function will be called when Blend panel has been closed.
        # Here we will reset the reference dictionary so it can be open again.
        ################################################################
        if name in self.animBlendPanel:
            del self.animBlendPanel[name]
        return

    ## Process message from SEditor object
    def toggleWidgetVis(self):
        ################################################################
        # toggleWidgetVis(self)
        # This function will be called when user use the hot-key to change the
        # world setting. (From seSession)
        # In this function we will restore the change and let side window know
        # the hot-key ahs been pushed.
        ################################################################
        if self.sideWindow != None:
            self.sideWindow.toggleWidgetVisFromMainW()
        else:
            self.widgetVis = (self.widgetVis+1)%2

    def toggleBackface(self):
        ################################################################
        # toggleBackface(self)
        # This function will be called when user use the hot-key to change the
        # world setting. (From seSession)
        # In this function we will restore the change and let side window know
        # the hot-key ahs been pushed.
        ################################################################
        if self.sideWindow != None:
            self.sideWindow.toggleBackfaceFromMainW()
        else:
            self.backface = (self.backface+1)%2

    def toggleTexture(self):
        ################################################################
        # toggleTexture(self)
        # This function will be called when user use the hot-key to change the
        # world setting. (From seSession)
        # In this function we will restore the change and let side window know
        # the hot-key ahs been pushed.
        ################################################################
        if self.sideWindow != None:
            self.sideWindow.toggleTextureFromMainW()
        else:
            self.texture = (self.texture+1)%2

    def toggleWireframe(self):
        ################################################################
        # toggleWireframe(self)
        # This function will be called when user use the hot-key to change the
        # world setting. (From seSession)
        # In this function we will restore the change and let side window know
        # the hot-key ahs been pushed.
        ################################################################
        if self.sideWindow != None:
            self.sideWindow.toggleWireframeFromMainW()
        else:
            self.wireframe = (self.wireframe+1)%2

    def openAlignPanel(self, nodePath=None):
        name = nodePath.getName()
        if name not in self.alignPanelDict:
            list = AllScene.getAllObjNameAsList()
            if name in list:
                list.remove(name)
            else:
                return
            self.alignPanelDict[name] = AlignTool(nodePath = nodePath, list = list)
        return

    def closeAlignPanel(self, name=None):
        if name in self.alignPanelDict:
            del self.alignPanelDict[name]

    def alignObject(self, nodePath, name, list):
        target = AllScene.getObjFromSceneByName(name)
        pos = target.getPos()
        hpr = target.getHpr()
        scale = target.getScale()
        if list[0]: # Align X
            nodePath.setX(pos.getX())
        if list[1]: # Align Y
            nodePath.setY(pos.getY())
        if list[2]: # Align Z
            nodePath.setZ(pos.getZ())
        if list[3]: # Align H
            nodePath.setH(hpr.getX())
        if list[4]: # Align P
            nodePath.setP(hpr.getY())
        if list[5]: # Align R
            nodePath.setR(hpr.getZ())
        if list[6]: # Scale X
            nodePath.setSx(scale.getX())
        if list[7]: # Scale Y
            nodePath.setSy(scale.getY())
        if list[8]: # Scale Z
            nodePath.setSz(scale.getZ())
        return

    ### Event from Motion Path Panel
    def requestCurveList(self, nodePath,name):
        curveList = AllScene.getCurveList(nodePath)
        messenger.send('curveListFor'+name, [curveList])


    ## Steal from DirectSession...
    def flash(self, nodePath = 'None Given'):
        """ Highlight an object by setting it red for a few seconds """
        # Clean up any existing task
        taskMgr.remove('flashNodePath')
        # Spawn new task if appropriate
        if nodePath == 'None Given':
            # If nothing specified, try selected node path
            nodePath = self.selected.last
        if nodePath:
            if nodePath.hasColor():
                doneColor = nodePath.getColor()
                flashColor = VBase4(1) - doneColor
                flashColor.setW(1)
            else:
                doneColor = None
                flashColor = VBase4(1,0,0,1)
            # Temporarily set node path color
            nodePath.setColor(flashColor)
            # Clean up color in a few seconds
            t = taskMgr.doMethodLater(1.5,
                                      # This is just a dummy task
                                      self.flashDummy,
                                      'flashNodePath')
            t.nodePath = nodePath
            t.doneColor = doneColor
            # This really does all the work
            t.uponDeath = self.flashDone

    def flashDummy(self, state):
        # Real work is done in upon death function
        return Task.done

    def flashDone(self,state):
        # Return node Path to original state
        if state.nodePath.isEmpty():
            # Node path doesn't exist anymore, bail
            return
        if state.doneColor:
            state.nodePath.setColor(state.doneColor)
        else:
            state.nodePath.clearColor()




editor = myLevelEditor(parent = base.tkRoot)

base.run()
