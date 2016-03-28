"""
Defines ObjectMgrBase
"""

import os, time, copy

from direct.task import Task
from direct.actor.Actor import Actor
from pandac.PandaModules import *
from .ActionMgr import *
from . import ObjectGlobals as OG

# python wrapper around a panda.NodePath object
class PythonNodePath(NodePath):
    def __init__(self,node):
        NodePath.__init__(self,node)

class ObjectMgrBase:
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
        self.currLiveNP = None

        self.Actor = []
        self.findActors(render)
        self.Nodes = []
        self.findNodes(render)

    def reset(self):
        base.direct.deselectAllCB()

        for id in list(self.objects.keys()):
            try:
                self.objects[id][OG.OBJ_NP].removeNode()
            except:
                pass
            del self.objects[id]

        for np in list(self.npIndex.keys()):
            del self.npIndex[np]

        self.objects = {}
        self.npIndex = {}
        self.saveData = []
        self.Actor = []
        self.Nodes = []

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

    def addNewCurveFromFile(self, curveInfo, degree, uid=None, parent=None, fSelectObject=True, nodePath=None):
        """ function to add new curve to the scene from file"""
        curve = []
        curveControl = []

        #transfer the curve information from simple positions into control nodes
        for item in curveInfo:
            controler = render.attachNewNode("controler")
            controler = loader.loadModel('models/misc/smiley')
            controlerPathname = 'controler%d' % item[0]
            controler.setName(controlerPathname)
            controler.setPos(item[1])
            controler.setColor(0, 0, 0, 1)
            controler.setScale(0.2)
            controler.reparentTo(render)
            controler.setTag('OBJRoot','1')
            controler.setTag('Controller','1')
            curve.append((None, item[1]))
            curveControl.append((item[0], controler))

        self.editor.curveEditor.degree = degree
        self.editor.curveEditor.ropeUpdate (curve)
        #add new curve to the scene
        curveObjNP = self.addNewCurve(curveControl, degree, uid, parent, fSelectObject, nodePath = self.editor.curveEditor.currentRope)
        curveObj = self.findObjectByNodePath(curveObjNP)
        self.editor.objectMgr.updateObjectPropValue(curveObj, 'Degree', degree, fSelectObject=False, fUndo=False)

        for item in curveControl:
            item[1].reparentTo(curveObjNP)
            item[1].hide()

        curveControl = []
        curve = []
        self.editor.curveEditor.currentRope = None

        return curveObjNP

    def addNewCurve(self, curveInfo, degree, uid=None, parent=None, fSelectObject=True, nodePath=None):
        """ function to add new curve to the scene"""
        if parent is None:
            parent = self.editor.NPParent

        if uid is None:
            uid = self.genUniqueId()

        if self.editor:
            objDef = self.editor.objectPalette.findItem('__Curve__')

        if nodePath is None:
            # we need to create curve
            # and then create newobj with newly created curve
            pass
        else:
            newobj = nodePath

        newobj.reparentTo(parent)
        newobj.setTag('OBJRoot','1')

        # populate obj data using default values
        properties = {}
        for key in objDef.properties.keys():
            properties[key] = objDef.properties[key][OG.PROP_DEFAULT]

        properties['Degree'] = degree
        properties['curveInfo'] = curveInfo

        # insert obj data to main repository
        self.objects[uid] = [uid, newobj, objDef, None, None, properties, (1,1,1,1)]
        self.npIndex[NodePath(newobj)] = uid

        if self.editor:
            if fSelectObject:
                self.editor.select(newobj, fUndo=0)
            self.editor.ui.sceneGraphUI.add(newobj, parent)
            self.editor.fNeedToSave = True

        return newobj

    def addNewObject(self, typeName, uid = None, model = None, parent=None, anim = None, fSelectObject=True, nodePath=None, nameStr=None):
        """ function to add new obj to the scene """
        if parent is None:
            parent = self.editor.NPParent

        if uid is None:
            uid = self.genUniqueId()

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
            if not hasattr(objDef, 'createFunction'):
                return newobj
            if nodePath is None:
                if objDef.createFunction:
                    funcName = objDef.createFunction[OG.FUNC_NAME]
                    funcArgs = copy.deepcopy(objDef.createFunction[OG.FUNC_ARGS])

                    for pair in list(funcArgs.items()):
                        if pair[1] == OG.ARG_NAME:
                            funcArgs[pair[0]] = nameStr
                        elif pair[1] == OG.ARG_PARENT:
                            funcArgs[pair[0]] = parent

                    if type(funcName) == str:
                        if funcName.startswith('.'):
                            # when it's using default objectHandler
                            if self.editor:
                                func = Functor(getattr(self.editor, "objectHandler%s"%funcName))
                            else: # when loaded outside of LE
                                func = Functor(getattr(base, "objectHandler%s"%funcName))
                        else:
                            # when it's not using default objectHandler, whole name of the handling obj
                            # should be included in function name
                            func = Functor(eval(funcName))
                    else:
                        func = funcName
                    # create new obj using function and keyword arguments defined in ObjectPalette
                    newobj = func(**funcArgs)
                elif objDef.actor:
                    if model is None:
                        model = objDef.model
                    try:
                        newobj = Actor(model)
                    except:
                        newobj = Actor(Filename.fromOsSpecific(model).getFullpath())
                    if hasattr(objDef, 'animDict') and objDef.animDict != {}:
                        objDef.anims = objDef.animDict.get(model)

                elif objDef.model is not None:
                    # since this obj is simple model let's load the model
                    if model is None:
                        model = objDef.model
                    try:
                        newobjModel = loader.loadModel(model)
                    except:
                        newobjModel = loader.loadModel(Filename.fromOsSpecific(model).getFullpath(), okMissing=True)
                    if newobjModel:
                        self.flatten(newobjModel, model, objDef, uid)
                        newobj = PythonNodePath(newobjModel)
                    else:
                        newobj = None

                else:
                    newobj = hidden.attachNewNode(objDef.name)
            else:
                newobj = nodePath

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
                self.editor.ui.sceneGraphUI.add(newobj, parent)
                self.editor.fNeedToSave = True
        return newobj

    def removeObjectById(self, uid):
        obj = self.findObjectById(uid)
        nodePath = obj[OG.OBJ_NP]

        for i in range(0,len(self.Actor)):
            if self.Actor[i] == obj:
                del self.Actor[i]
                break
        for i in range(0,len(self.Nodes)):
            if self.Nodes[i][OG.OBJ_UID] == uid:
                del self.Nodes[i]
                break
        self.editor.animMgr.removeAnimInfo(obj[OG.OBJ_UID])

        del self.objects[uid]
        del self.npIndex[nodePath]

        # remove children also
        for child in nodePath.getChildren():
            if child.hasTag('OBJRoot'):
                self.removeObjectByNodePath(child)
        nodePath.remove()

        self.editor.fNeedToSave = True

    def removeObjectByNodePath(self, nodePath):
        uid = self.npIndex.get(nodePath)
        if uid:
            for i in range(0,len(self.Actor)):
                if self.Actor[i][OG.OBJ_UID] == uid:
                    del self.Actor[i]
                    break
            for i in range(0,len(self.Nodes)):
                if self.Nodes[i][OG.OBJ_UID] == uid:
                    del self.Nodes[i]
                    break
            self.editor.animMgr.removeAnimInfo(uid)

            del self.objects[uid]
            del self.npIndex[nodePath]

        # remove children also
        for child in nodePath.getChildren():
            if child.hasTag('OBJRoot'):
                self.removeObjectByNodePath(child)
        self.editor.fNeedToSave = True

    def findObjectById(self, uid):
        return self.objects.get(uid)

    def findObjectByNodePath(self, nodePath):
        uid = self.npIndex.get(NodePath(nodePath))
        if uid is None:
            return None
        else:
            return self.objects[uid]

    def findObjectByNodePathBelow(self, nodePath):
        for ancestor in nodePath.getAncestors():
            if ancestor.hasTag('OBJRoot'):
                return self.findObjectByNodePath(ancestor)

        return None

    def findObjectsByTypeName(self, typeName):
        results = []
        for uid in self.objects.keys():
            obj = self.objects[uid]
            if obj[OG.OBJ_DEF].name == typeName:
                results.append(obj)

        return results

    def deselectAll(self):
        self.currNodePath = None
        taskMgr.remove('_le_updateObjectUITask')
        self.editor.ui.objectPropertyUI.clearPropUI()
        self.editor.ui.sceneGraphUI.tree.UnselectAll()

    def selectObject(self, nodePath, fLEPane=0):
        obj = self.findObjectByNodePath(nodePath)
        if obj is None:
            return
        self.selectObjectCB(obj, fLEPane)

    def selectObjectCB(self, obj, fLEPane):
        self.currNodePath = obj[OG.OBJ_NP]
        self.objectsLastXform[obj[OG.OBJ_UID]] = Mat4(self.currNodePath.getMat())
        # [gjeon] to connect transform UI with nodepath's transform
        self.spawnUpdateObjectUITask()
        self.updateObjectPropertyUI(obj)
        #import pdb;pdb.set_trace()
        if fLEPane == 0:
           self.editor.ui.sceneGraphUI.select(obj[OG.OBJ_UID])

        if not obj[OG.OBJ_DEF].movable:
            if base.direct.widget.fActive:
                base.direct.widget.toggleWidget()

    def updateObjectPropertyUI(self, obj):
        objDef = obj[OG.OBJ_DEF]
        objProp = obj[OG.OBJ_PROP]
        self.editor.ui.objectPropertyUI.updateProps(obj, objDef.movable)
        self.editor.fNeedToSave = True

    def onEnterObjectPropUI(self, event):
        taskMgr.remove('_le_updateObjectUITask')
        self.editor.ui.bindKeyEvents(False)

    def onLeaveObjectPropUI(self, event):
        self.spawnUpdateObjectUITask()
        self.editor.ui.bindKeyEvents(True)

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
        self.editor.fNeedToSave = True

    def setObjectTransform(self, uid, xformMat):
        obj = self.findObjectById(uid)
        if obj:
            obj[OG.OBJ_NP].setMat(xformMat)
        self.editor.fNeedToSave = True

    def updateObjectColor(self, r, g, b, a, np=None):
        if np is None:
            np = self.currNodePath

        obj = self.findObjectByNodePath(np)
        if not obj:
            return
        obj[OG.OBJ_RGBA] = (r,g,b,a)
        for child in np.getChildren():
            if not child.hasTag('OBJRoot') and\
               not child.hasTag('_le_sys') and\
               child.getName() != 'bboxLines':
                child.setTransparency(1)
                child.setColorScale(r, g, b, a)
        self.editor.fNeedToSave = True

    def updateObjectModel(self, model, obj, fSelectObject=True):
        """ replace object's model """
        if obj[OG.OBJ_MODEL] != model:
            base.direct.deselectAllCB()

            objNP = obj[OG.OBJ_NP]
            objDef = obj[OG.OBJ_DEF]
            objRGBA = obj[OG.OBJ_RGBA]
            uid = obj[OG.OBJ_UID]

            # load new model
            if objDef.actor:
                try:
                    newobj = Actor(model)
                except:
                    newobj = Actor(Filename.fromOsSpecific(model).getFullpath())
            else:
                newobjModel = loader.loadModel(model, okMissing=True)
                if newobjModel is None:
                    print("Can't load model %s"%model)
                    return
                self.flatten(newobjModel, model, objDef, uid)
                newobj = PythonNodePath(newobjModel)
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

            # update scene graph label
            self.editor.ui.sceneGraphUI.changeLabel(obj[OG.OBJ_UID], newobj.getName())

            self.editor.fNeedToSave = True
            # update anim if necessary
            animList = obj[OG.OBJ_DEF].animDict.get(model)
            if animList:
                self.updateObjectAnim(animList[0], obj, fSelectObject=fSelectObject)
            else:
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

            self.editor.fNeedToSave = True

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

        elif propType == OG.PROP_UI_COMBO_DYNAMIC:
            val = event.GetString()

        else:
            # unsupported property type
            return

        # now update object prop value and call update function
        self.updateObjectPropValue(obj, propName, val, \
                                   fSelectObject=(propType != OG.PROP_UI_SLIDE)
                                   )

    def updateObjectPropValue(self, obj, propName, val, fSelectObject=False, fUndo=True):
        """
        Update object property value and
        call update function if defined.
        """
        objDef = obj[OG.OBJ_DEF]
        objProp = obj[OG.OBJ_PROP]

        propDef = objDef.properties[propName]
        propDataType = propDef[OG.PROP_DATATYPE]

        if propDataType != OG.PROP_BLIND:
            val = OG.TYPE_CONV[propDataType](val)
            oldVal = objProp[propName]

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
                    elif funcArgs[key] == OG.ARG_NOLOADING:
                        kwargs[key] = fSelectObject
                        undoKwargs[key] = fSelectObject
                    else:
                        kwargs[key] = funcArgs[key]
                        undoKwargs[key] = funcArgs[key]

                if type(funcName) == str:
                    if funcName.startswith('.'):
                        if self.editor:
                            func = Functor(getattr(self.editor, "objectHandler%s"%funcName), **kwargs)
                            undoFunc = Functor(getattr(self.editor, "objectHandler%s"%funcName), **undoKwargs)
                        else: # when loaded outside of LE
                            func = Functor(getattr(base, "objectHandler%s"%funcName), **kwargs)
                            undoFunc = Functor(getattr(base, ".objectHandler%s"%funcName), **undoKwargs)
                    else:
                        func = Functor(eval(funcName), **kwargs)
                        undoFunc = Functor(eval(funcName), **undoKwargs)
                else:
                    func = Functor(funcName, **kwargs)
                    undoFunc = Functor(funcName, **undoKwargs)

                # finally call update function
                #func(**kwargs)
        else:
            oldVal = objProp[propName]
            func = None
            undoFunc = None
        action = ActionUpdateObjectProp(self.editor, fSelectObject, obj, propName, val, oldVal, func, undoFunc)
        if fUndo:
            self.editor.actionMgr.push(action)
        action()

        if self.editor:
            self.editor.fNeedToSave = True
            if fSelectObject:
                base.direct.select(obj[OG.OBJ_NP], fUndo=0)

    def updateCurve(self, val, obj):
        curve = obj[OG.OBJ_NP]
        degree = int(val)
        curveNode = obj[OG.OBJ_PROP]['curveInfo']
        curveInfor = []
        for item in curveNode:
                curveInfor.append((None, item[1].getPos()))
        curve.setup(degree, curveInfor)

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
            if child.hasTag('OBJRoot') and not child.hasTag('Controller'):
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

                    if objDef.named:
                        nameStr = "'%s'"%np.getName()
                    else:
                        nameStr = "None"

                    if objDef.name == '__Curve__':
                        #transfer the curve information from control nodes into simple positions for file save
                        objCurveInfo = obj[OG.OBJ_PROP]['curveInfo']
                        self.objDegree = obj[OG.OBJ_PROP]['Degree']
                        newobjCurveInfo = []
                        for item in objCurveInfo:
                            newobjCurveInfo.append((item[0], item[1].getPos()))

                        self.saveData.append("\nobjects['%s'] = objectMgr.addNewCurveFromFile(%s, %s, '%s', %s, False, None)"%(uid, newobjCurveInfo, self.objDegree, uid, parentStr))
                    else:
                        self.saveData.append("\nobjects['%s'] = objectMgr.addNewObject('%s', '%s', %s, %s, %s, False, None, %s)"%(uid, objDef.name, uid, modelStr, parentStr, animStr, nameStr))

                    self.saveData.append("if objects['%s']:"%uid)
                    self.saveData.append("    objects['%s'].setPos(%s)"%(uid, np.getPos()))
                    self.saveData.append("    objects['%s'].setHpr(%s)"%(uid, np.getHpr()))
                    self.saveData.append("    objects['%s'].setScale(%s)"%(uid, np.getScale()))
                    self.saveData.append("    objectMgr.updateObjectColor(%f, %f, %f, %f, objects['%s'])"%(objRGBA[0], objRGBA[1], objRGBA[2], objRGBA[3], uid))

                    if objDef.name == '__Curve__':
                        pass
                    else:
                        self.saveData.append("    objectMgr.updateObjectProperties(objects['%s'], %s)"%(uid,objProp))

                self.traverse(child, uid)

    def getSaveData(self):
        self.saveData = []
        self.getPreSaveData()
        self.traverse(render)
        self.getPostSaveData()
        return self.saveData

    def getPreSaveData(self):
        """
        if there are additional data to be saved before main data
        you can override this function to populate data
        """
        pass

    def getPostSaveData(self):
        """
        if there are additional data to be saved after main data
        you can override this function to populate data
        """
        pass

    def duplicateObject(self, nodePath, parent=None):
        obj = self.findObjectByNodePath(nodePath)
        if obj is None:
            return None
        objDef = obj[OG.OBJ_DEF]
        objModel = obj[OG.OBJ_MODEL]
        objAnim = obj[OG.OBJ_ANIM]
        objRGBA = obj[OG.OBJ_RGBA]

        if parent is None:
            parentNP = nodePath.getParent()
            parentObj = self.findObjectByNodePath(parentNP)
            if parentObj is None:
                parent = parentNP
            else:
                parent = parentObj[OG.OBJ_NP]

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

        self.editor.fNeedToSave = True

    def makeSelectedLive(self):
        obj = self.findObjectByNodePath(base.direct.selected.last)
        if obj:
            if self.currLiveNP:
                self.currLiveNP.clearColorScale()
                if self.currLiveNP == obj[OG.OBJ_NP]:
                    self.currLiveNP = None
                    return

            self.currLiveNP = obj[OG.OBJ_NP]
            self.currLiveNP.setColorScale(0, 1, 0, 1)

    def replaceObjectWithTypeName(self, obj, typeName):
        uid = obj[OG.OBJ_UID]
        objNP = obj[OG.OBJ_NP]
        mat = objNP.getMat()
        parentObj = self.findObjectByNodePath(objNP.getParent())
        if parentObj:
            parentNP = parentObj[OG.OBJ_NP]
        else:
            parentNP = None
        self.removeObjectById(uid)
        self.editor.ui.sceneGraphUI.delete(uid)
        newobj = self.addNewObject(typeName, uid, parent=parentNP, fSelectObject=False)
        newobj.setMat(mat)

    def flatten(self, newobjModel, model, objDef, uid):
        # override this to flatten models
        pass

    def findActors(self, parent):
        for child in parent.getChildren():
            if child.hasTag('OBJRoot') and not child.hasTag('Controller'):
                obj = self.findObjectByNodePath(child)

                if obj:
                    if isinstance(obj[OG.OBJ_NP],Actor):
                        self.Actor.append(obj)

                self.findActors(child)


    def findNodes(self, parent):
        for child in parent.getChildren():
            if child.hasTag('OBJRoot') and not child.hasTag('Controller'):
                obj = self.findObjectByNodePath(child)

                if obj:
                    self.Nodes.append(obj)

                self.findActors(child)


