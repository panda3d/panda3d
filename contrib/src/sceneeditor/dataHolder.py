###############################
# TK and PMW INTERFACE MODULES#
###############################
from direct.showbase.TkGlobal import*
import Pmw
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater

if sys.version_info >= (3, 0):
    from tkinter.filedialog import askopenfilename
else:
    from tkFileDialog import askopenfilename


#############################
# Scene Editor Python Files #
#############################
from seLights import * # All the scene editor lighting
from seFileSaver import * # The actual File Saving Module which generates Python code

################################
#Panda Modules                 #
################################
from direct.actor import Actor

###############################
# Core Python Modules         #
###############################
import os
import string
import sys

import seParticleEffect
import seParticles

######################################################################################################
# Data Holder
# This class will actually hold all data in the form of dictionaries
# This is essentially the entire state of the scene
# When saving a scene, the dictionaries will be accessed and code generated for the loaded content
# Dictionaries are used to access the objects in the scene and their properties so code can be generated
# Write/Use Accessor Methods to interface with these dictionaries
######################################################################################################

class dataHolder:

    ModelDic = {} # Name: Model Nodepath; ModelRoot (whatever loader.loadModel() returns)
    ModelRefDic = {} # Name:File Path; (whatever the Open File Dialog Returns)
    ActorDic = {} # Name:Actor Actor; Nodepath, ModelRoot (whatever Actor.Actor() returns)
    ActorRefDic = {} # Name:File Path; (whatever the Open File Dialog Returns)
    curveDict = {}  # Node Name: CurveCollection; The actual curve collection data
    collisionDict = {} # Node Name: collisionNode; collisionNode in which contains a collision object
    blendAnimDict = {} # Node Name: Dictionary of blended animation; User blened animation will be saved in here.
                       # the data structure in the inner dictionary is
                       # {"name of blended animation" : [Animation Name A, Animation Name B, Effect(Float)]}
    collisionVisable = True # A flag used to record that collision objects are visable or not
    dummyDict = {}  # Node Name: Dummy Obj; All Object created as a dummy will be save here.
    particleDict={} # "Effect Name": Effect Object
    particleNodes={} # "Effect Name": Node which is a parent to the effect used to move it aruond easily
    ModelNum = 0 # Count of number of models loaded
    ActorNum = 0 # Count of number of animations loaded
    theScene=None # Global variable to hold a loaded scene
    CollisionHandler = CollisionHandlerEvent() # This object shows what happend when collision appeared
                                               # Now, the default is CollisionHandlerEvent Type, which will just send out message when collision happend

    controlType = 'Keyboard' # default control input setting
    # Default Control setting for keyboard.
    controlTarget = camera
    # This two dictionary set the basic setting for the keyboard contorl
    # Those dictionaries will be passed into controller panel each time it has been opend
    # Do NOT change anything about INDEX in the dictionary!! (But it's OK to change the values)
    keyboardMapDict = {'KeyForward':'arrow_up',
                       'KeyBackward':'arrow_down',
                       'KeyLeft':'arrow_left',
                       'KeyRight':'arrow_right',
                       'KeyUp':'',
                       'KeyDown':'',
                       'KeyTurnRight':'',
                       'KeyTurnLeft':'',
                       'KeyTurnUp':'',
                       'KeyTurnDown':'',
                       'KeyRollRight':'',
                       'KeyRollLeft':'',
                       'KeyScaleUp':'',
                       'KeyScaleDown':'',
                       'KeyScaleXUp':'',
                       'KeyScaleXDown':'',
                       'KeyScaleYUp':'',
                       'KeyScaleYDown':'',
                       'KeyScaleZUp':'',
                       'KeyScaleZDown':''}
    keyboardSpeedDict = {'SpeedForward': 0,
                         'SpeedBackward': 0,
                         'SpeedLeft': 0,
                         'SpeedRight': 0,
                         'SpeedUp': 0,
                         'SpeedDown': 0,
                         'SpeedTurnRight': 0,
                         'SpeedTurnLeft': 0,
                         'SpeedTurnUp': 0,
                         'SpeedTurnDown': 0,
                         'SpeedRollRight':0,
                         'SpeedRollLeft':0,
                         'SpeedScaleUp':0,
                         'SpeedScaleDown':0,
                         'SpeedScaleXUp':0,
                         'SpeedScaleXDown':0,
                         'SpeedScaleYUp':0,
                         'SpeedScaleYDown':0,
                         'SpeedScaleZUp':0,
                         'SpeedScaleZDown':0}

    def __init__(self):

        # Creat light manager to contorl the lighting
        self.lightManager = seLightManager()
        self.lightManager.allOn()

        # Initialize the basic message formate from CollisionHandler
        self.CollisionHandler.setInPattern("%fnenter%in")
        self.CollisionHandler.setOutPattern("%fnexit%in")

        pass


    def resetAll(self):
        #################################################################
        # resetAll(self)
        # This function will reset the whole scene
        #################################################################
        # Delete Everything in the Scene Graph
        for index in self.ModelDic:
            self.ModelDic[index].removeNode()
        for index in self.ActorDic:
            self.ActorDic[index].removeNode()
        for index in self.dummyDict:
            self.dummyDict[index].removeNode()
        for index in self.collisionDict:
            self.collisionDict[index].removeNode()
        for index in self.particleNodes:
            self.particleDict[index].cleanup()
            self.particleNodes[index].removeNode()

        # Clear all data containers in the dataHolder
        self.ModelDic.clear()
        self.ModelRefDic.clear()
        self.ActorDic.clear()
        self.ActorRefDic.clear()
        self.dummyDict.clear()
        self.lightManager.deleteAll()
        self.blendAnimDict.clear()
        self.particleDict.clear()
        self.particleNodes.clear()

        self.ModelNum=0
        self.ActorNum=0
        self.theScene=None
        messenger.send('SGE_Update Explorer',[render])
        print('Scene should be cleaned up!')

    def removeObj(self, nodePath):
        #################################################################
        # removeObj(self, nodePath)
        # This function will take one nodePath obj as a input
        # and will remove this node from scene if it is legal to be removed.
        # Also, this function will remove all children nodes belong to this specific nodePath by recursive call.
        #################################################################
        name = nodePath.getName()

        ## Check if there is any child node, if so, remove it.
        childrenList = nodePath.getChildren()


        if name in self.ModelDic:
            del self.ModelDic[name]
            del self.ModelRefDic[name]
            if len(childrenList) != 0:
                for node in childrenList:
                    self.removeObj(node)
            nodePath.removeNode()
            self.ModelNum -= 1
            pass
        elif name in self.ActorDic:
            del self.ActorDic[name]
            del self.ActorRefDic[name]
            if len(childrenList) != 0:
                for node in childrenList:
                    self.removeObj(node)
            nodePath.removeNode()
            self.ActorNum -= 1
            pass
        elif name in self.collisionDict:
            del self.collisionDict[name]
            if len(childrenList) != 0:
                for node in childrenList:
                    self.removeObj(node)
            nodePath.removeNode()
            pass
        elif name in self.dummyDict:
            del self.dummyDict[name]
            if len(childrenList) != 0:
                for node in childrenList:
                    self.removeObj(node)
            nodePath.removeNode()
            pass
        elif self.lightManager.isLight(name):
            if len(childrenList) != 0:
                for node in childrenList:
                    self.removeObj(node)
            list = self.lightManager.delete(name)
            return list
        elif name in self.particleNodes:
            self.particleNodes[name].removeNode()
            del self.particleNodes[name]
            del self.particleDict[name]
        else:
            print('You cannot remove this NodePath')
            return

        messenger.send('SGE_Update Explorer',[render])
        return

    def duplicateObj(self, nodePath, pos, hpr, scale, num):
        #############################################################################
        # duplicateObj(self, nodePath, pos, hpr, scale, num)
        # This function now only worked for either Actor or Model type node.
        # It won't duplicate lights or others!!!
        #
        # This function will duplicate the input nodePath "num" times.
        # Each time it will use "pos", "hpr" and "scale" as a offset to change the properties of copy.
        # Then, reparent copies to the same parent of origin
        #
        # To Do:
        # Make it work for all kinds of objects....
        #############################################################################
        name = nodePath.getName()
        isModel = True
        cPos = pos
        cHpr = hpr
        cScale = scale
        parent = nodePath.getParent()
        if name in self.ActorDic:
            holder = self.ActorDic
            holderRef = self.ActorRefDic
            isModel = False
        elif name in self.ModelDic:
            holder = self.ModelDic
            holderRef = self.ModelRefDic
        else:
            print('---- DataHolder: Target Obj is not a legal object could be duplicate!')
            return

        FilePath = holderRef[name]
        oPos = holder[name].getPos()+cPos
        oHpr = holder[name].getHpr()+cHpr

        for i in range(num):
            if isModel:
                ### copy model node from modelpool
                newName = name+'_copy_%d'%i
                while self.isInScene(newName):
                    newName = newName + '_1'
                holder[newName] = loader.loadModel(FilePath.getFullpath())
                holderRef[newName] = FilePath
                self.ModelNum += 1
                holder[newName].reparentTo(parent)
                holder[newName].setPos(oPos)
                holder[newName].setHpr(oHpr)
                holder[newName].setScale(cScale)
                holder[newName].setName(newName)
                oPos = oPos + cPos
                oHpr = oHpr + cHpr

            else:
                ### copy the actor- not includ its animations
                '''
                Yeah, Yeah, Yeah, I know I should not reload the Actor but get it from modelpool too.
                I tried, but it caused some error.
                I 'might' be back to fix this problem.
                '''
                newName = name+'_copy_%d'%i
                while self.isInScene(newName):
                    newName = newName + '_1'
                holder[newName] = Actor.Actor(FilePath.getFullpath())
                holderRef[newName] = FilePath
                self.ActorNum += 1
                holder[newName].reparentTo(parent)
                holder[newName].setPos(oPos)
                holder[newName].setHpr(oHpr)
                holder[newName].setScale(cScale)
                holder[newName].setName(newName)
                oPos = oPos + cPos
                oHpr = oHpr + cHpr

        messenger.send('SGE_Update Explorer',[render])
        return

    def loadModel(self, lFilePath, FilePath, Name='Model_'):
        ###########################################################################
        # loadModel(self, lFilePath, FilePath, Name='Model_')
        # This funciton will load a model node into the scene
        # and will keep its reference in the ModelDic dictionary. {"NameOfModel":ModelRoot}
        # Also it will keep the file path in ModelRefDic dictionary.
        #
        # The "lFilePath" parameter now is completely useless,
        # but I still keep it here because maybe some day we will need it...
        # (NOT because I am laze to change the funtion call in the sceneEditor...)
        #
        ###########################################################################
        self.ModelNum += 1
        defaultName = Name + '%d'%self.ModelNum
        while self.isInScene(defaultName):
            defaultName = defaultName + '_1'
        self.ModelDic[defaultName] = loader.loadModel(FilePath)
        if self.ModelDic[defaultName]==None:
            del self.ModelDic[defaultName]
            self.ModelNum -= 1
            return False
        self.ModelRefDic[defaultName] = FilePath
        self.ModelDic[defaultName].setName(defaultName)
        self.ModelDic[defaultName].reparentTo(render)
        messenger.send('SGE_Update Explorer',[render])
        messenger.send('DH_LoadingComplete',[self.ModelDic[defaultName]])
        return True

    def loadActor(self, lFilePath, FilePath, Name='Actor_'):
        ###########################################################################
        # loadActor(self, lFilePath, FilePath, Name='Actor_')
        # This funciton will load an actor node into the scene
        # and will keep its reference in the ActorDic dictionary.{"NameOfActor":Actor}
        # Also it will keep the file path in ActorRefDic dictionary.
        #
        # The "lFilePath" parameter now is completely useless,
        # but I still keep it here because maybe some day we will need it...
        # (NOT because I am laze to change the funtion call in the sceneEditor...)
        #
        ###########################################################################
        self.ActorNum += 1
        defaultName = Name + '%d'%self.ActorNum
        while self.isInScene(defaultName):
            defaultName = defaultName + '_1'
        self.ActorDic[defaultName] = Actor.Actor(FilePath.getFullpath())
        if self.ActorDic[defaultName]==None:
            del self.ActorDic[defaultName]
            self.ActorNum -= 1
            return False
        self.ActorRefDic[defaultName] = FilePath
        self.ActorDic[defaultName].setName(defaultName)
        self.ActorDic[defaultName].reparentTo(render)
        messenger.send('SGE_Update Explorer',[render])
        messenger.send('DH_LoadingComplete',[self.ActorDic[defaultName]])
        return True


    def isActor(self, name):
        ###########################################################################
        # isActor(self, name)
        # This funciton will return True if there is an Actor in the scene named "name"
        # and will return False if not.
        ###########################################################################
        return name in self.ActorDic

    def getActor(self, name):
        ###########################################################################
        # getActor(self, name)
        # This funciton will return an Actor node named "name"
        ###########################################################################
        if self.isActor(name):
            return self.ActorDic[name]
        else:
            print('----No Actor named: ', name)
            return None

    def getModel(self, name):
        ###########################################################################
        # getModel(self, name)
        # This funciton will return a model node named "name"
        ###########################################################################
        if self.isModel(name):
            return self.ModelDic[name]
        else:
            print('----No Model named: ', name)
            return None

    def isModel(self, name):
        ###########################################################################
        # isModel(self, name)
        # This funciton will return True if there is a Model in the scene named "name"
        # and will return False if not.
        ###########################################################################
        return name in self.ModelDic

    def loadAnimation(self,name, Dic):
        ###########################################################################
        # loadAnimation(self,name, Dic)
        # This funciton will load animation into an Actor NOde named "name."
        # All animation data needs to be put into a dictionary "Dic".
        # The formate of this dictionary is {"Name of Animation" : Path to the animation Egg file}
        #
        # Also, it will send out a message after the loading complete.
        # 'DataH_loadFinish'+"the name of actor",
        # this message will be catched by the sub window in the animation penal.
        ###########################################################################
        if self.isActor(name):
            self.ActorDic[name].loadAnims(Dic)
            for anim in Dic:
                self.ActorDic[name].bindAnim(anim)
            messenger.send('DataH_loadFinish'+name)
            return
        else:
            print('------ Error when loading animation for Actor: ', name)

    def removeAnimation(self, name, anim):
        ###########################################################################
        # removeAnimation(self, name, anim)
        # This function will remove the specific animation "anim" from the actor named "name."
        #
        # After remove compelete, it will send out two messages.
        # One is 'DataH_removeAnimFinish'+"name of the actor." It will be caught by Animation Panel of this actor.
        # The other is 'animRemovedFromNode,' this will be caught by property window of this actor.
        ###########################################################################
        if self.isActor(name):
            self.ActorDic[name].unloadAnims([anim])
            AnimDict = self.ActorDic[name].getAnimControlDict()
            del AnimDict['lodRoot']['modelRoot'][anim]
            messenger.send('DataH_removeAnimFinish'+name)
            messenger.send('animRemovedFromNode',[self.ActorDic[name],self.getAnimationDictFromActor(name)])
        return

    def toggleLight(self):
        ###########################################################################
        # toggleLight(self)
        # This function do noting but call a function inside the lightManger to toggle the lighting.
        # If it is on, then it will turn it to off.
        # If it is off, it will turn it on.
        ###########################################################################
        self.lightManager.toggle()
        return

    def isLight(self,name):
        ###########################################################################
        # isLight(self, name)
        # This function will check that there is a light named "name" of not.
        # If it dose have, then return True.
        # If it doesn't, then return False
        ###########################################################################
        return self.lightManager.isLight(name)

    def createLight(self, type = 'ambient',
                   lightcolor=VBase4(0.3,0.3,0.3,1),
                   specularColor = VBase4(1),
                   position = Point3(0,0,0),
                   orientation = Vec3(1,0,0),
                   constant = 1.0,
                   linear = 0.0,
                   quadratic = 0.0,
                   exponent = 0.0):
        ###########################################################################
        # createLight(self, type = 'ambient',
        #            lightcolor=VBase4(0.3,0.3,0.3,1),
        #            specularColor = VBase4(1),
        #            position = Point3(0,0,0),
        #            orientation = Vec3(1,0,0),
        #            constant = 1.0,
        #            linear = 0.0,
        #            quadratic = 0.0,
        #            exponent = 0.0)
        # It will create a light(seLight) into the scene.
        #
        # For more detail about creating light, please look the seLight.py.
        #
        ###########################################################################
        list,lightNode = self.lightManager.create(type, lightcolor, specularColor, position,
                                                  orientation, constant, linear, quadratic, exponent)
        messenger.send('SGE_Update Explorer',[render])
        return list, lightNode

    def getLightList(self):
        ###########################################################################
        # getLightList(self)
        # This function will return the lights(seLight) as a list.
        #
        # For more detail about creating light, please look the seLight.py.
        #
        ###########################################################################
        return self.lightManager.getLightList()

    def getLightNode(self,lightName):
        ###########################################################################
        # getLightNode(self,lightName)
        # This function will return the lightNode(seLigth) named 'lightName' back.
        #
        # For more detail about creating light, please look the seLight.py.
        #
        ###########################################################################
        return self.lightManager.getLightNode(lightName)

    def toggleLightNode(self, lightNode):
        ###########################################################################
        # toggleLightNode(self, lightNode)
        # This function will enable of disable the lightNode user put in.
        #
        # For more detail about creating light, please look the seLight.py.
        #
        ###########################################################################
        if lightNode.active:
            self.lightManager.setOff(lightNode)
        else:
            self.lightManager.setOn(lightNode)
        return

    def rename(self,nodePath,nName):
        ###########################################################################
        # Rename(self,nodePath,nName)
        # First, it will check the target object is legal to rename or not.
        # this function now doesn't support user to rename everything on the scene gragh.
        # If there already has object hase the same name with the target object,
        # the new name will be changed.
        ###########################################################################
        oName = nodePath.getName()
        if oName == nName:
            # If the new name is the same with old name, do nothing.
            return

        while self.isInScene(nName):
            nName = nName + '_1'

        if self.isActor(oName):
            self.ActorDic[nName]= self.ActorDic[oName]
            self.ActorRefDic[nName]= self.ActorRefDic[oName]
            self.ActorDic[nName].setName(nName)
            if oName in self.blendAnimDict:
                self.blendAnimDict[nName] = self.blendAnimDict[oName]
                del self.blendAnimDict[oName]
            del self.ActorDic[oName]
            del self.ActorRefDic[oName]
        elif self.isModel(oName):
            self.ModelDic[nName]= self.ModelDic[oName]
            self.ModelRefDic[nName]= self.ModelRefDic[oName]
            self.ModelDic[nName].setName(nName)
            del self.ModelDic[oName]
            del self.ModelRefDic[oName]
        elif self.lightManager.isLight(oName):
            list, lightNode = self.lightManager.rename(oName, nName)
        elif oName in self.dummyDict:
            self.dummyDict[nName]= self.dummyDict[oName]
            self.dummyDict[nName].setName(nName)
            del self.dummyDict[oName]
        elif oName in self.collisionDict:
            self.collisionDict[nName]= self.collisionDict[oName]
            self.collisionDict[nName].setName(nName)
            del self.collisionDict[oName]

        elif oName in self.particleNodes:
            self.particleNodes[nName]= self.particleNodes[oName]
            self.particleDict[nName]= self.particleDict[oName]
            self.particleDict[nName].setName(nName)
            self.particleNodes[nName].setName(nName)
            del self.particleNodes[oName]
            del self.particleDict[oName]
        else:
            print('----Error: This Object is not allowed to this function!')

        if oName in self.curveDict:
            self.curveDict[nName] = self.curveDict[oName]
            del self.curveDict[oName]

        if self.lightManager.isLight(nName):
            return list, lightNode

    def isInScene(self,name):
        ###########################################################################
        # isInScene(self,name)
        # Return True if there is a Node named "name" inside the scene.
        # This will check the whole scene, including model, actor, dummy, collisionObj...
        ###########################################################################
        if self.isActor(name):
            return True
        elif self.isModel(name):
            return True
        elif self.lightManager.isLight(name):
            return True
        elif name in self.dummyDict:
            return True
        elif name in self.collisionDict:
            return True
        elif name in self.particleNodes:
            return True
        elif (name == 'render')or(name == 'SEditor')or(name == 'Lights')or(name == 'camera'):
            return True

        return False

    def bindCurveToNode(self,node,curveCollection):
        ###########################################################################
        # bindCurveToNode(self,node,curveCollection)
        # This function will maintain the curvesDict
        # using the node name as a reference to assosiate a list which contains all curves related to that node.
        ###########################################################################
        name = node.getName()
        if name in self.curveDict:
            self.curveDict[name].append(curveCollection)
            return
        else:
            self.curveDict[name] = [curveCollection]
            return
        return

    def getCurveList(self, nodePath):
        ###########################################################################
        # getCureveList(self, nodePath)
        # This function will return a list
        # which contains all curves taht have been binded with the inout node
        # If the input node has not been bindedwith any curve, it will return None.
        ###########################################################################
        name = nodePath.getName()
        if name in self.curveDict:
            return self.curveDict[name]
        else:
            return None

    def removeCurveFromNode(self, nodePath, curveName):
        ###########################################################################
        # removeCurveFromNode(self, nodePath, curveName)
        # This function will remove the "curveName" curve(Motion path data) from the nodaPath.
        # After remove, it will send out a message.
        # 'curveRemovedFromNode'
        # This message will be caught by Property Window for this node.
        ###########################################################################
        name =nodePath.getName()
        if name in self.curveDict:
            index = None
            for curve in self.curveDict[name]:
                if curve.getCurve(0).getName() == curveName:
                    index = self.curveDict[name].index(curve)
                    break
            del self.curveDict[name][index]
            if len(self.curveDict[name])!=0:
                messenger.send('curveRemovedFromNode',[nodePath, self.curveDict[name]])
            else:
                del self.curveDict[name]
                messenger.send('curveRemovedFromNode',[nodePath, None])
        return

    def getInfoOfThisNode(self, nodePath):
        ###########################################################################
        # getInfoOfThisNode(self, nodePath)
        # This function will return a list which contains all object properies
        # that will be used in property window.
        ###########################################################################
        type = ''
        info = {}
        name = nodePath.getName()
        if name == 'render':
            type = 'render'
        elif name == 'camera':
            type = 'camera'
            cameraNode = base.cam.node()
            lens = cameraNode.getLens()
            info['lensType'] = lens.getClassType().getName()
            info['far'] = lens.getFar()
            info['near'] = lens.getNear()
            info['FilmSize'] = lens.getFilmSize()
            info['fov'] = lens.getFov()
            info['hFov'] = lens.getHfov()
            info['vFov'] = lens.getVfov()
            info['focalLength'] = lens.getFocalLength()


        elif name == 'SEditor':
            type = 'Special'
        elif self.isActor(name):
            type = 'Actor'
            info['filePath'] = self.ActorRefDic[name]
            info['animDict'] = self.getAnimationDictFromActor(name)
        elif self.isModel(name):
            type = 'Model'
            info['filePath'] = self.ModelRefDic[name]
        elif self.isLight(name):
            type = 'Light'
            info['lightNode'] = self.lightManager.getLightNode(name)
        elif name in self.dummyDict:
            type = 'dummy'
        elif name in self.collisionDict:
            type = 'collisionNode'
            info['collisionNode'] = self.collisionDict[name]
        if name in self.curveDict:
            info['curveList'] = self.getCurveList(nodePath)

        return type, info

    def getAnimationDictFromActor(self, actorName):
        ###########################################################################
        # getAnimationDictFromActor(self, actorName)
        # This function will return a Dictionary which contains the animation data in the actor "actorName".
        # The data inside is get from the actor, so, it can't be wrong...
        ###########################################################################
        animContorlDict = self.ActorDic[actorName].getAnimControlDict()
        animNameList = self.ActorDic[actorName].getAnimNames()
        if len(animNameList)==0:
            return {}
        animDict = {}
        for anim in animNameList:
            animDict[anim] = animContorlDict['lodRoot']['modelRoot'][anim][0]
        return animDict

    def addDummyNode(self,nodePath):
        ###########################################################################
        # addDummyNode(self,nodePath)
        # This function will add a dummy node into the scane and reparent it to nodePath which user put in.
        #
        # This dummy actually is just a default sphere model.
        #
        ###########################################################################
        number = len(self.dummyDict)
        number += 1
        name = 'Dummy%d'%number
        self.dummyModel = loader.loadModel( "models/misc/sphere" )
        self.dummyModel.reparentTo(nodePath)
        while self.isInScene(name):
            name = name + '_1'
        self.dummyModel.setName(name)
        self.dummyDict[name] = self.dummyModel
        messenger.send('SGE_Update Explorer',[render])
        return

    def addCollisionObject(self, collisionObj, nodePath, pointA=None, pointB=None, pointC=None, name = None):
        ###########################################################################
        # addCollisionObject(self, collisionObj, nodePath, pointA=None, pointB=None, pointC=None, name = None)
        # This function will add a collision object into a "CollisionNode" object and put it into scene.
        # The collision object will be reparent to "nodePath" and
        # will be show on the screen if user has enable the "show collision objects" option.
        ###########################################################################
        if name == None:
            name = 'CollisionNode_%d'%len(self.collisionDict)
        while self.isInScene(name):
            name=name + '_1'
        node = CollisionNode(name)
        node.addSolid(collisionObj)
        self.collisionDict[name] = nodePath.attachNewNode(node)

        if pointA!=None:
            self.collisionDict[name].setTag('A_X','%f'%pointA.getX())
            self.collisionDict[name].setTag('A_Y','%f'%pointA.getY())
            self.collisionDict[name].setTag('A_Z','%f'%pointA.getZ())
            self.collisionDict[name].setTag('B_X','%f'%pointB.getX())
            self.collisionDict[name].setTag('B_Y','%f'%pointB.getY())
            self.collisionDict[name].setTag('B_Z','%f'%pointB.getZ())
            self.collisionDict[name].setTag('C_X','%f'%pointC.getX())
            self.collisionDict[name].setTag('C_Y','%f'%pointC.getY())
            self.collisionDict[name].setTag('C_Z','%f'%pointC.getZ())

        if self.collisionVisable:
            self.collisionDict[name].show()
        #Manakel 2/12/2005: replace node by its nodepath
        base.cTrav.addCollider( self.collisionDict[name], self.CollisionHandler)

        messenger.send('SGE_Update Explorer',[render])

        return

    def toggleCollisionVisable(self, visable):
        ###########################################################################
        # toggleCollisionVisable(self, visable)
        # This fucntion will toggle the visibility of all collision node in the scene.
        ###########################################################################
        if visable == 1:
            self.collisionVisable = True
            for name in self.collisionDict:
                if self.collisionDict[name].isHidden():
                    self.collisionDict[name].show()
        else:
            self.collisionVisable = False
            for name in self.collisionDict:
                if not self.collisionDict[name].isHidden():
                    self.collisionDict[name].hide()

    def toggleParticleVisable(self, visable):
        if not visable:
            for name in self.particleNodes:
                self.particleNodes[name].setTransparency(True)
                self.particleNodes[name].setAlphaScale(0)
                self.particleNodes[name].setBin("fixed", 1)
        else:
            for name in self.particleNodes:
                self.particleNodes[name].setTransparency(False)
                self.particleNodes[name].setAlphaScale(1)
                self.particleNodes[name].setBin("default", 1)
        return

    def getBlendAnimAsDict(self, name):
        ###########################################################################
        # getBlendAnimAsDict(self, name)
        # This function will return a dictionry
        # which contains user blended animation data for actor named "name."
        # The formate of thsi dictionary is
        # {"name of Blend Animation" : ["Animation A, Animation B, Effect(Float, 0~1)"]}
        ###########################################################################
        if name in self.blendAnimDict:
            return self.blendAnimDict[name]
        else:
            return {}

    def saveBlendAnim(self, actorName, blendName, animNameA, animNameB, effect):
        ###########################################################################
        # saveBlendAnim(self, actorName, blendName, animNameA, animNameB, effect)
        # This function will save the blended Animation "blendname" for actor "actorNane"
        # and keep the data in the blendAnimDict.
        #
        # Also, if this blend is the first blend animation that the target actor has,
        # this function will add a "Blending" tag on this actor which is "True".
        ###########################################################################
        if actorName in self.blendAnimDict:
            if blendName in self.blendAnimDict[actorName]:
                ### replace the original setting
                self.blendAnimDict[actorName][blendName][0] = animNameA
                self.blendAnimDict[actorName][blendName][1] = animNameB
                self.blendAnimDict[actorName][blendName][2] = effect
            else:
                ### create new blend animation in the dictionary
                self.blendAnimDict[actorName][blendName] = [animNameA, animNameB, effect]
        else:
            self.getActor(actorName).setTag('Blending','True')
            self.blendAnimDict[actorName] = {blendName:[animNameA, animNameB, effect]}
        return self.blendAnimDict[actorName]

    def renameBlendAnim(self, actorName, nName, oName, animNameA, animNameB, effect):
        ###########################################################################
        # renameBlendAnim(self, actorName, nName, oName, animNameA, animNameB, effect)
        # This function is used to rename a exist blended animation named "oName" to "nName."
        # The way it doing this is first remove the original blend fomr the actor
        # and then re-create a new one named "nName" in.
        # Because it is not just simply rename the animation,
        # it will also rewrite the data to the newest one.
        ###########################################################################
        self.removeBlendAnim(actorName,oName)
        print(self.blendAnimDict)
        return self.saveBlendAnim(actorName, nName, animNameA, animNameB, effect)

    def removeBlendAnim(self, actorName, blendName):
        ###########################################################################
        # removeBlendAnim(self, actorName, blendName)
        # This fucntion will remove the record of blended animation named "blendName"
        # from the actor named "actorName".
        #
        # Also, it will check that there is any blended animation remained for this actor,
        # If none, this function will clear the "Blending" tag of this object.
        ###########################################################################
        if actorName in self.blendAnimDict:
            if blendName in self.blendAnimDict[actorName]:
                ### replace the original setting
                del self.blendAnimDict[actorName][blendName]
            if len(self.blendAnimDict[actorName])==0:
                del self.blendAnimDict[actorName]
                self.getActor(actorName).clearTag('Blending')
                return {}
            return self.blendAnimDict[actorName]
        else:
            return {}

    def getAllObjNameAsList(self):
        ###########################################################################
        # getAllObjNameAsList(self)
        # This function will return a list which contains all objects' names in the scene.
        # It means which won't have any kinds of animation, blend animation or Mopath data inside.
        ###########################################################################
        list = ['camera'] # Default object you can select camera
        list = list + self.ModelDic.keys() \
               + self.ActorDic.keys() + self.collisionDict.keys() \
               + self.dummyDict.keys() + self.particleNodes.keys() \
               + self.lightManager.getLightList()
        return list

    def getObjFromSceneByName(self, name):
        ###########################################################################
        # getObjFromSceneByName(self, name)
        # return a reference to the nodePath named "name"
        ###########################################################################
        if name == 'camera':
            return camera
        elif name in self.ModelDic:
            return self.ModelDic[name]
        elif name in self.ActorDic:
            return self.ActorDic[name]
        elif name in self.collisionDict:
            return self.collisionDict[name]
        elif name in self.dummyDict:
            return self.dummyDict[name]
        elif name in self.particleNodes:
            return self.particleNodes[name]
        elif self.lightManager.isLight(name):
            return self.lightManager.getLightNode(name)

        return None

    def getControlSetting(self):
        ###########################################################################
        # getControlSetting(self)
        # return tqwo things.
        # One is the type of the control. The other is the data about that control.
        # Now we only support keyboard control, so it will return a list.
        # The first object in the list is a reference to the target we want to control.
        # The second object in the list is a dictionary which contains a map about
        # which keyboard message should be accepted.
        # The third and the last object here is a dictionary which contains the data
        # indicating that the changing value for each keyboard control event.
        ###########################################################################
        if self.controlType == 'Keyboard':
            return self.controlType, [self.controlTarget, self.keyboardMapDict.copy(), self.keyboardSpeedDict.copy()]
        elif self.controlType == 'Tracker':
            return self.controlType, []
        return

    def saveControlSetting(self, controlType, data):
        ###########################################################################
        # saveControlSetting(self, controlType, data)
        # copy the current control setting into dataHolder
        # Mainly called by sceneEditor.
        ###########################################################################
        if controlType == 'Keyboard':
            self.controlType = controlType
            self.controlTarget = data[0]
            self.keyboardMapDict.clear()
            self.keyboardMapDict = data[1].copy()
            self.keyboardSpeedDict.clear()
            self.keyboardSpeedDict = data[2].copy()
            return

    def loadScene(self):
        ###########################################################################
        # loadScene(self)
        # Opens a dialog box asking for a scene file to load.  It then removes
        # the current scene and opens the new one.
        # It basically proceeds by executig the python file containing the scene
        # and then re-populating the various dictionaries based on the dictionaries
        # in the scene file hence reviving the state for the scene
        ###########################################################################

        ### Ask for a filename
        OpenFilename = askopenfilename(filetypes = [("PY","py")],title = "Load Scene")
        if(not OpenFilename):
            return None
        f=Filename.fromOsSpecific(OpenFilename)
        fileName=f.getBasenameWoExtension()
        dirName=f.getFullpathWoExtension()
        print("DATAHOLDER::" + dirName)
        ############################################################################
        # Append the path to this file to our sys path where python looks for modules
        # We do this so that we can use "import"  on our saved scene code and execute it
        ############################################################################
        sys.path.append(os.path.dirname(f.toOsSpecific()))

        ############################################################################
        # Actually import the scene... this executes the code in the scene
        ############################################################################
        self.theScene=__import__(fileName)
        self.Scene=self.theScene.SavedScene(0,seParticleEffect,seParticles,dirName) # Specify load mode of 0 which will allow us to pass seParticle and seParticleEffect
        messenger.send('SGE_Update Explorer',[render])


        # Lets call some important initialization methods on our scene:
        #self.Scene.starteffects(0,seParticleEffect,seParticles,dirName)    # This special calling of start effect with mode 0 is to use seParticleEffect and seParticles


        ############################################################################
        # Populate Model related Dictionaries
        ############################################################################
        for model in self.Scene.ModelDic:
            self.ModelDic[model]=self.Scene.ModelDic[model]
            #self.ModelRefDic[model]=self.Scene.ModelRefDic[model] # The Old absolute paths way
            self.ModelRefDic[model]=Filename(dirName + "/" + self.Scene.ModelRefDic[model]) # Relative Paths
            self.ModelNum=self.ModelNum+1

        ############################################################################
        # Populate Actor related Dictionaries
        ############################################################################
        for actor in self.Scene.ActorDic:
            self.ActorDic[actor]=self.Scene.ActorDic[actor]
            #self.ActorRefDic[actor]=self.Scene.ActorRefDic[actor] # Old way of doing absolute paths
            self.ActorRefDic[actor]=Filename(dirName + "/" + self.Scene.ActorRefDic[actor]) # Relative Paths
            if(str(actor) in self.Scene.blendAnimDict):
                self.blendAnimDict[actor]=self.Scene.blendAnimDict[actor]
            self.ActorNum=self.ActorNum+1



        ############################################################################
        # Populate Light related Dictionaries
        ############################################################################
        #print self.Scene.LightDict
        for light in self.Scene.LightDict:
            #print light
            alight=self.Scene.LightDict[light]
            type=self.Scene.LightTypes[light]
            thenode=self.Scene.LightNodes[light]
            #print type
            if type == 'ambient':
                self.lightManager.create('ambient',alight.getColor(),name=alight.getName(),tag=thenode.getTag("Metadata"))
            elif type == 'directional':
                #print alight.getPoint()
                #print alight.getDirection()
                self.lightManager.create('directional',alight.getColor(),alight.getSpecularColor(),thenode.getPos(),thenode.getHpr(),name=alight.getName(),tag=thenode.getTag("Metadata"))
            elif type == 'point':
                atten=alight.getAttenuation()
                #print alight.getPoint()
                self.lightManager.create('point',alight.getColor(),alight.getSpecularColor(),thenode.getPos(),Vec3(1,0,0),atten.getX(),atten.getY(),atten.getZ(),name=alight.getName(),tag=thenode.getTag("Metadata"))
            elif type == 'spot':
                atten=alight.getAttenuation()
                self.lightManager.create('spot',alight.getColor(),alight.getSpecularColor(),thenode.getPos(),thenode.getHpr(),atten.getX(),atten.getY(),atten.getZ(),alight.getExponent(),name=alight.getName(),tag=thenode.getTag("Metadata"))
            else:
                print('Invalid light type')

        ############################################################################
        # Populate Dummy related Dictionaries
        ############################################################################
        for dummy in self.Scene.dummyDict:
            self.dummyDict[dummy] = self.Scene.dummyDict[dummy]

        ############################################################################
        # Populate Collision related Dictionaries
        ############################################################################
        for collnode in self.Scene.collisionDict:
            self.collisionDict[collnode]=self.Scene.collisionDict[collnode]

        ############################################################################
        # Populate Mopath related Dictionaries
        ############################################################################
        for node in self.Scene.curveDict:
            curveCollection=self.Scene.curveDict[node]
            for curve in curveCollection:
                curveColl=ParametricCurveCollection()
                nodeP=loader.loadModel(curve)
                curveColl.addCurves(nodeP.node())
                nodeP.removeNode()
                thenode=render.find("**/"+str(node))
                self.bindCurveToNode(thenode,curveColl)

        ############################################################################
        # Populate Particle related Dictionaries
        ############################################################################
        for effect in self.Scene.particleDict:
            theeffect=self.Scene.particleDict[effect]
            emitter=loader.loadModel("sphere")
            emitter.setPosHprScale(theeffect.getX(),theeffect.getY(),theeffect.getZ(),theeffect.getH(),theeffect.getP(),theeffect.getR(),theeffect.getSx(),theeffect.getSy(),theeffect.getSz())
            theeffect.setPos(0,0,0)
            theeffect.setName(str(effect))
            tempparent=theeffect.getParent()
            theeffect.reparentTo(emitter)
            emitter.setName(str(effect))
            emitter.reparentTo(tempparent)
            theeffect.enable()
            self.particleDict[effect]=theeffect
            self.particleNodes[effect]=emitter



        # Clean up things added to scene graph by saved file's code execution
        for light in self.Scene.LightDict:
            vestige=render.find('**/'+light)
            if(vestige != None):
                vestige.removeNode()

        ############################################################################
        # return the filename and update the scenegraph explorer window
        ############################################################################
        messenger.send('SGE_Update Explorer',[render])
        if(OpenFilename):
            return OpenFilename
        else:
            return None

    def getList(self):
        return self.lightManager.getList()
