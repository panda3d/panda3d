"""
Defines ObjectMgr
"""

import os, time, wx

from direct.task import Task
from direct.actor.Actor import Actor
from pandac.PandaModules import *
from ActionMgr import *
import ObjectGlobals as OG

class ObjectMgr:
    """ ObjectMgr will create, manage, update objects in the scene """
    
    def __init__(self, editor):
        self.editor = editor

        # main obj repository of objects in the scene
        self.objects = {}
        self.npIndex = {}
        self.saveData = []
        self.objectsLastXform = {}

        self.lastUid = ''
        self.lastUidMode = 0
        self.currNodePath = None   

    def reset(self):
        base.direct.deselectAllCB()

        for id in self.objects.keys():
            self.objects[id][OG.OBJ_NP].removeNode()
            del self.objects[id]

        for np in self.npIndex.keys():
            del self.npIndex[np]
               
        self.objects = {}
        self.npIndex = {}
        self.saveData = []

    def genUniqueId(self):
        # [gjeon] to solve the problem of unproper $USERNAME
        userId = os.path.basename(os.path.expandvars('$USERNAME'))
        if userId == '':
            userId = base.config.GetString("le-user-id")
        if userId == '':
            userId = 'unknown'
        newUid = str(time.time()) + userId
        # prevent duplicates from being generated in the same frame (this can
        # happen when creating several new objects at once)
        if (self.lastUid == newUid):
            # append a value to the end to uniquify the id
            newUid = newUid + str(self.lastUidMod)
            self.lastUidMod = self.lastUidMod + 1
        else:
            self.lastUid = newUid
            self.lastUidMod = 0
        return newUid

    def addNewObject(self, typeName, uid = None, model = None, parent=None, anim = None, fSelectObject=True):
        """ function to add new obj to the scene """
        if parent is None:
            parent = render

        if self.editor:
            objDef = self.editor.objectPalette.findItem(typeName)
            if objDef is None:
                objDef = self.editor.protoPalette.findItem(typeName)
        else: # when loaded outside of LE
            objDef = base.objectPalette.findItem(typeName)
            if objDef is None:
                objDef = base.protoPalette.findItem(typeName)
        newobj = None
        if objDef and type(objDef) != dict:
            if objDef.createFunction:
                funcName = objDef.createFunction[OG.FUNC_NAME]
                funcArgs = objDef.createFunction[OG.FUNC_ARGS]
                if funcName.startswith('.'):
                    # when it's using default objectHandler
                    if self.editor:
                        func = Functor(eval("base.le.objectHandler%s"%funcName))
                    else: # when loaded outside of LE
                        func = Functor(eval("base.objectHandler%s"%funcName))                        
                else:
                    # when it's not using default objectHandler, whole name of the handling obj
                    # should be included in function name
                    func = Functor(eval(funcName))

                # create new obj using function and keyword arguments defined in ObjectPalette
                newobj = func(**funcArgs)
            elif objDef.actor:
                if model is None:
                    model = objDef.model
                try:
                    newobj = Actor(model)
                except:
                    newobj = Actor(Filename.fromOsSpecific(model).getFullpath())
            elif objDef.model is not None:
                # since this obj is simple model let's load the model
                if model is None:
                    model = objDef.model
                try:
                    newobj = loader.loadModel(model)
                except:
                    newobj = loader.loadModel(Filename.fromOsSpecific(model).getFullpath())

            i = 0
            for i in range(len(objDef.anims)):
                animFile = objDef.anims[i]
                # load new anim
                animName = os.path.basename(animFile)
                if i < len(objDef.animNames):
                    animName = objDef.animNames[i]
                newAnim = newobj.loadAnims({animName:animFile})

                if anim:
                    if anim == animFile:
                        newobj.loop(animName)
                else:
                    if i == 0:
                        anim = animFile
                        newobj.loop(animName)

            if newobj is None:
                return None

            newobj.reparentTo(parent)
            newobj.setTag('OBJRoot','1')

            if uid is None:
                uid = self.genUniqueId()

            # populate obj data using default values
            properties = {}
            for key in objDef.properties.keys():
                properties[key] = objDef.properties[key][OG.PROP_DEFAULT]

            # insert obj data to main repository
            self.objects[uid] = [uid, newobj, objDef, model, anim, properties, (1,1,1,1)]
            self.npIndex[NodePath(newobj)] = uid

            if self.editor:
                if fSelectObject:
                    self.editor.select(newobj, fUndo=0)
                self.editor.ui.sceneGraphUI.add(newobj)

        return newobj

    def removeObjectByNodePath(self, nodePath):
        uid = self.npIndex.get(nodePath)
        if uid:
            del self.objects[uid]
            del self.npIndex[nodePath]

        # remove children also
        for child in nodePath.getChildren():
            if child.hasTag('OBJRoot'):
                self.removeObjectByNodePath(child)

    def findObjectById(self, uid):
        return self.objects.get(uid)

    def findObjectByNodePath(self, nodePath):
        uid = self.npIndex.get(NodePath(nodePath))
        if uid is None:
            return None
        else:
            return self.objects[uid]

    def deselectAll(self):
        self.currNodePath = None
        taskMgr.remove('_le_updateObjectUITask')
        self.editor.ui.objectPropertyUI.clearPropUI()
        self.editor.ui.sceneGraphUI.tree.UnselectAll()

    def selectObject(self, nodePath, fLEPane=0):
        obj = self.findObjectByNodePath(nodePath)
        if obj is None:
            return

        self.currNodePath = obj[OG.OBJ_NP]
        self.objectsLastXform[obj[OG.OBJ_UID]] = Mat4(self.currNodePath.getMat())
        # [gjeon] to connect transform UI with nodepath's transform
        self.spawnUpdateObjectUITask()
        self.updateObjectPropertyUI(obj)
        #import pdb;pdb.set_trace()
        if fLEPane == 0:
           self.editor.ui.sceneGraphUI.select(obj[OG.OBJ_UID])

    def updateObjectPropertyUI(self, obj):
        objDef = obj[OG.OBJ_DEF]
        objProp = obj[OG.OBJ_PROP]
        self.editor.ui.objectPropertyUI.updateProps(obj)

    def onEnterObjectPropUI(self, event):
        taskMgr.remove('_le_updateObjectUITask')        

    def onLeaveObjectPropUI(self, event):
        self.spawnUpdateObjectUITask()
        
    def spawnUpdateObjectUITask(self):
        if self.currNodePath is None:
            return

        taskMgr.remove('_le_updateObjectUITask')
        t = Task.Task(self.updateObjectUITask)
        t.np = self.currNodePath
        taskMgr.add(t, '_le_updateObjectUITask')
        
    def updateObjectUITask(self, state):
        self.editor.ui.objectPropertyUI.propX.setValue(state.np.getX())
        self.editor.ui.objectPropertyUI.propY.setValue(state.np.getY())
        self.editor.ui.objectPropertyUI.propZ.setValue(state.np.getZ())

        h = state.np.getH()
        while h < 0:
            h = h + 360.0

        while h > 360:
            h = h - 360.0

        p = state.np.getP()
        while p < 0:
            p = p + 360.0

        while p > 360:
            p = p - 360.0

        r = state.np.getR()
        while r < 0:
            r = r + 360.0

        while r > 360:
            r = r - 360.0 
            
        self.editor.ui.objectPropertyUI.propH.setValue(h)
        self.editor.ui.objectPropertyUI.propP.setValue(p)
        self.editor.ui.objectPropertyUI.propR.setValue(r)        

        self.editor.ui.objectPropertyUI.propSX.setValue(state.np.getSx())
        self.editor.ui.objectPropertyUI.propSY.setValue(state.np.getSy())
        self.editor.ui.objectPropertyUI.propSZ.setValue(state.np.getSz())
        
        return Task.cont
        
    def updateObjectTransform(self, event):
        if self.currNodePath is None:
            return

        np = hidden.attachNewNode('temp')
        np.setX(float(self.editor.ui.objectPropertyUI.propX.getValue()))
        np.setY(float(self.editor.ui.objectPropertyUI.propY.getValue()))
        np.setZ(float(self.editor.ui.objectPropertyUI.propZ.getValue()))

        h = float(self.editor.ui.objectPropertyUI.propH.getValue())
        while h < 0:
            h = h + 360.0

        while h > 360:
            h = h - 360.0

        p = float(self.editor.ui.objectPropertyUI.propP.getValue())
        while p < 0:
            p = p + 360.0

        while p > 360:
            p = p - 360.0

        r = float(self.editor.ui.objectPropertyUI.propR.getValue())
        while r < 0:
            r = r + 360.0

        while r > 360:
            r = r - 360.0 
            
        np.setH(h)
        np.setP(p)
        np.setR(r)

        np.setSx(float(self.editor.ui.objectPropertyUI.propSX.getValue()))
        np.setSy(float(self.editor.ui.objectPropertyUI.propSY.getValue()))
        np.setSz(float(self.editor.ui.objectPropertyUI.propSZ.getValue()))        

        obj = self.findObjectByNodePath(self.currNodePath)
        action = ActionTransformObj(self.editor, obj[OG.OBJ_UID], Mat4(np.getMat()))
        self.editor.actionMgr.push(action)
        np.remove()
        action()

    def setObjectTransform(self, uid, xformMat):
        obj = self.findObjectById(uid)
        if obj:
            obj[OG.OBJ_NP].setMat(xformMat)
        
    def updateObjectColor(self, r, g, b, a, np=None):
        if np is None:
            np = self.currNodePath

        obj = self.findObjectByNodePath(np)
        if not obj:
            return
        obj[OG.OBJ_RGBA] = (r,g,b,a)
        for child in np.getChildren():
            if not child.hasTag('OBJRoot') and\
               child.getName() != 'bboxLines':
                child.setTransparency(1)
                child.setColorScale(r, g, b, a)

    def updateObjectModel(self, model, obj, fSelectObject=True):
        """ replace object's model """
        if obj[OG.OBJ_MODEL] != model:
            base.direct.deselectAllCB()

            objNP = obj[OG.OBJ_NP]
            objRGBA = obj[OG.OBJ_RGBA]
            
            # load new model
            newobj = loader.loadModel(model)
            newobj.setTag('OBJRoot','1')

            # reparent children
            objNP.findAllMatches("=OBJRoot").reparentTo(newobj)
            
            # reparent to parent
            newobj.reparentTo(objNP.getParent())

            # copy transform
            newobj.setPos(objNP.getPos())
            newobj.setHpr(objNP.getHpr())
            newobj.setScale(objNP.getScale())

            # copy RGBA data
            self.updateObjectColor(objRGBA[0], objRGBA[1], objRGBA[2], objRGBA[3], newobj)

            # delete old geom
            del self.npIndex[NodePath(objNP)]
            objNP.removeNode()

            # register new geom
            obj[OG.OBJ_NP] = newobj
            obj[OG.OBJ_MODEL] = model
            self.npIndex[NodePath(newobj)] = obj[OG.OBJ_UID]

            if fSelectObject:
                base.direct.select(newobj, fUndo=0)        

    def updateObjectAnim(self, anim, obj, fSelectObject=True):
        """ replace object's anim """
        if obj[OG.OBJ_ANIM] != anim:
            base.direct.deselectAllCB()
            objNP = obj[OG.OBJ_NP]

            # load new anim
            animName = os.path.basename(anim)
            newAnim = objNP.loadAnims({animName:anim})
            objNP.loop(animName)
            obj[OG.OBJ_ANIM] = anim
            if fSelectObject:
                base.direct.select(objNP, fUndo=0)

    def updateObjectModelFromUI(self, event, obj):
        """ replace object's model with one selected from UI """
        model = event.GetString()
        if model is not None:
            self.updateObjectModel(model, obj)

    def updateObjectAnimFromUI(self, event, obj):
        """ replace object's anim with one selected from UI """
        anim = event.GetString()
        if anim is not None:
            self.updateObjectAnim(anim, obj)

    def updateObjectProperty(self, event, obj, propName):
        """
        When an obj's property is updated in UI,
        this will update it's value in data structure.
        And call update function if defined.        
        """
        
        objDef = obj[OG.OBJ_DEF]
        objProp = obj[OG.OBJ_PROP]
        
        propDef = objDef.properties[propName]
        if propDef is None:
            return

        propType = propDef[OG.PROP_TYPE]
        propDataType = propDef[OG.PROP_DATATYPE]
        
        if propType == OG.PROP_UI_SLIDE:
            if len(propDef) <= OG.PROP_RANGE:
                return

            strVal = event.GetString()
            if strVal == '':
                min = float(propDef[OG.PROP_RANGE][OG.RANGE_MIN])
                max = float(propDef[OG.PROP_RANGE][OG.RANGE_MAX])
                intVal = event.GetInt()
                if intVal is None:
                    return
                val = intVal / 100.0 * (max - min) + min
            else:
                val = strVal

        elif propType == OG.PROP_UI_ENTRY:
            val = event.GetString()

        elif propType == OG.PROP_UI_SPIN:
            val = event.GetInt()

        elif propType == OG.PROP_UI_CHECK:
            if event.GetInt():
                val = True
            else:
                val = False

        elif propType == OG.PROP_UI_RADIO:
            val = event.GetString()

        elif propType == OG.PROP_UI_COMBO:
            val = event.GetString()

        else:
            # unsupported property type
            return

        # now update object prop value and call update function
        self.updateObjectPropValue(obj, propName, val, \
                                   fSelectObject=(propType != OG.PROP_UI_SLIDE)
                                   )

    def updateObjectPropValue(self, obj, propName, val, fSelectObject=False):
        """
        Update object property value and
        call update function if defined.         
        """
        
        objDef = obj[OG.OBJ_DEF]
        objProp = obj[OG.OBJ_PROP]
        
        propDef = objDef.properties[propName]
        propDataType = propDef[OG.PROP_DATATYPE]

        val = OG.TYPE_CONV[propDataType](val)
        oldVal = objProp[propName]
        #objProp[propName] = val
        
        if propDef[OG.PROP_FUNC] is None:
            func = None
            undoFunc = None
        else:
            funcName = propDef[OG.PROP_FUNC][OG.FUNC_NAME]
            funcArgs = propDef[OG.PROP_FUNC][OG.FUNC_ARGS]

            # populate keyword arguments
            kwargs = {}
            undoKwargs = {}
            for key in funcArgs.keys():
                if funcArgs[key] == OG.ARG_VAL:
                    kwargs[key] = val
                    undoKwargs[key] = oldVal
                elif funcArgs[key] == OG.ARG_OBJ:
                    undoKwargs[key] = obj
                    objProp[propName] = val
                    kwargs[key] = obj
                else:
                    kwargs[key] = funcArgs[key]
                    undoKwargs[key] = funcArgs[key]

            if funcName.startswith('.'):
                if self.editor:
                    func = Functor(eval("base.le.objectHandler%s"%funcName), **kwargs)
                    undoFunc = Functor(eval("base.le.objectHandler%s"%funcName), **undoKwargs)
                else: # when loaded outside of LE
                    func = Functor(eval("base.objectHandler%s"%funcName), **kwargs)
                    undoFunc = Functor(eval("base.objectHandler%s"%funcName), **undoKwargs)                    
            else:
                func = Functor(eval(funcName), **kwargs)
                undoFunc = Functor(eval(funcName), **undoKwargs)
                
            # finally call update function
            #func(**kwargs)
        action = ActionUpdateObjectProp(self.editor, fSelectObject, obj, propName, val, oldVal, func, undoFunc)
        self.editor.actionMgr.push(action)
        action()

        if self.editor and fSelectObject:
            base.direct.select(obj[OG.OBJ_NP], fUndo=0)

    def updateObjectProperties(self, nodePath, propValues):
        """
        When a saved level is loaded,
        update an object's properties
        And call update function if defined.
        """

        obj = self.findObjectByNodePath(nodePath)
        if obj:
            for propName in propValues:
                self.updateObjectPropValue(obj, propName, propValues[propName])

    def traverse(self, parent, parentId = None):
        """
        Trasverse scene graph to gather data for saving
        """
        
        for child in parent.getChildren():
            if child.hasTag('OBJRoot'):
                obj = self.findObjectByNodePath(child)

                if obj:
                    uid = obj[OG.OBJ_UID]
                    np = obj[OG.OBJ_NP]
                    objDef = obj[OG.OBJ_DEF]
                    objModel = obj[OG.OBJ_MODEL]
                    objAnim = obj[OG.OBJ_ANIM]
                    objProp = obj[OG.OBJ_PROP]
                    objRGBA = obj[OG.OBJ_RGBA]

                    if parentId:
                        parentStr = "objects['%s']"%parentId
                    else:
                        parentStr = "None"

                    if objModel:
                        modelStr = "'%s'"%objModel
                    else:
                        modelStr = "None"

                    if objAnim:
                        animStr = "'%s'"%objAnim
                    else:
                        animStr = "None"

                    self.saveData.append("\nobjects['%s'] = objectMgr.addNewObject('%s', '%s', %s, %s, %s, fSelectObject=False)"%(uid, objDef.name, uid, modelStr, parentStr, animStr))
                    self.saveData.append("if objects['%s']:"%uid)
                    self.saveData.append("    objects['%s'].setPos(%s)"%(uid, np.getPos()))
                    self.saveData.append("    objects['%s'].setHpr(%s)"%(uid, np.getHpr()))
                    self.saveData.append("    objects['%s'].setScale(%s)"%(uid, np.getScale()))
                    self.saveData.append("    objectMgr.updateObjectColor(%f, %f, %f, %f, objects['%s'])"%(objRGBA[0], objRGBA[1], objRGBA[2], objRGBA[3], uid))
                    self.saveData.append("    objectMgr.updateObjectProperties(objects['%s'], %s)"%(uid,objProp))
                    
                self.traverse(child, uid)

    def getSaveData(self):
        self.saveData = []
        self.traverse(render)
        return self.saveData

    def duplicateObject(self, nodePath, parent=None):
        obj = self.findObjectByNodePath(nodePath)
        if obj is None:
            return None
        objDef = obj[OG.OBJ_DEF]
        objModel = obj[OG.OBJ_MODEL]
        objAnim = obj[OG.OBJ_ANIM]
        objRGBA = obj[OG.OBJ_RGBA]

        if parent is None:
            parent = nodePath.getParent()

        newObjNP = self.addNewObject(objDef.name, parent=parent, fSelectObject = False)

        # copy transform data
        newObjNP.setPos(obj[OG.OBJ_NP].getPos())
        newObjNP.setHpr(obj[OG.OBJ_NP].getHpr())
        newObjNP.setScale(obj[OG.OBJ_NP].getScale())

        newObj = self.findObjectByNodePath(NodePath(newObjNP))
        if newObj is None:
            return None
        # copy model info
        self.updateObjectModel(obj[OG.OBJ_MODEL], newObj, fSelectObject=False)

        # copy anim info
        self.updateObjectAnim(obj[OG.OBJ_ANIM], newObj, fSelectObject=False)

        # copy other properties
        for key in obj[OG.OBJ_PROP]:
            self.updateObjectPropValue(newObj, key, obj[OG.OBJ_PROP][key])

        return newObjNP

    def duplicateChild(self, nodePath, parent):
        children = nodePath.findAllMatches('=OBJRoot')
        for childNP in children:
            newChildObjNP = self.duplicateObject(childNP, parent)
            if newChildObjNP is not None:
                self.duplicateChild(childNP, newChildObjNP)
    
    def duplicateSelected(self):
        selectedNPs = base.direct.selected.getSelectedAsList()
        duplicatedNPs = []
        for nodePath in selectedNPs:
            newObjNP = self.duplicateObject(nodePath)
            if newObjNP is not None:
                self.duplicateChild(nodePath, newObjNP)
                duplicatedNPs.append(newObjNP)

        base.direct.deselectAllCB()
        for newNodePath in duplicatedNPs:
            base.direct.select(newNodePath, fMultiSelect = 1, fUndo=0)
