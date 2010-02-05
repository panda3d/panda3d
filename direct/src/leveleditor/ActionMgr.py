import ObjectGlobals as OG

class ActionBase(Functor):
    """ Base class for user actions """

    def __init__(self, function, *args, **kargs):
        Functor.__init__(self, function, *args, **kargs)
        self.result = None

    def _do__call__(self, *args, **kargs):
        self.saveStatus()
        self.result = Functor._do__call__(self, *args, **kargs)
        return self.result

    def saveStatus(self):
        # save object status for undo here
        pass
        
    def undo(self):
        print "undo method is not defined for this action"

class ActionMgr:
    def __init__(self):
        self.undoList = []
        self.redoList = []

    def reset(self):
        while len(self.undoList) > 0:
            action = self.undoList.pop()
            action.destroy()

        while len(self.redoList) > 0:
            action = self.redoList.pop()
            action.destroy()

    def push(self, action):
        self.undoList.append(action)

    def undo(self):
        if len(self.undoList) < 1:
            print 'No more undo'
        else:
            action = self.undoList.pop()
            self.redoList.append(action)
            action.undo()

    def redo(self):
        if len(self.redoList) < 1:
            print 'No more redo'
        else:
            action = self.redoList.pop()
            self.undoList.append(action)
            action()

class ActionAddNewObj(ActionBase):
    """ Action class for adding new object """
    
    def __init__(self, editor, *args, **kargs):
        self.editor = editor
        function = self.editor.objectMgr.addNewObject
        ActionBase.__init__(self, function, *args, **kargs)

    def undo(self):
        if self.result is None:
            print "Can't undo this"
        else:
            base.direct.deselect(self.result)
            base.direct.removeNodePath(self.result)
            self.result = None

class ActionDeleteObj(ActionBase):
    """ Action class for deleting object """

    def __init__(self, editor, *args, **kargs):
        self.editor = editor
        function = base.direct.removeAllSelected
        ActionBase.__init__(self, function, *args, **kargs)
        self.selectedUIDs = []
        self.hierarchy = {}
        self.objInfos = {}

    def saveStatus(self):
        selectedNPs = base.direct.selected.getSelectedAsList()
        def saveObjStatus(np, isRecursive=True):
            obj = self.editor.objectMgr.findObjectByNodePath(np)
            if obj:
                uid = obj[OG.OBJ_UID]
                if not isRecursive:
                    self.selectedUIDs.append(uid)
                objNP = obj[OG.OBJ_NP]
                self.objInfos[uid] = obj
                parentNP = objNP.getParent()
                if parentNP == render:
                    self.hierarchy[uid] = None
                else:
                    parentObj = self.editor.objectMgr.findObjectByNodePath(parentNP)
                    if parentObj:
                        self.hierarchy[uid] = parentObj[OG.OBJ_UID]

                for child in np.getChildren():
                    if child.hasTag('OBJRoot'):
                        saveObjStatus(child)

        for np in selectedNPs:
            saveObjStatus(np, False)

    def undo(self):
        if len(self.hierarchy.keys()) == 0 or\
           len(self.objInfos.keys()) == 0:
            print "Can't undo this"
        else:
            def restoreObject(uid, parentNP):
                obj = self.objInfos[uid]
                objDef = obj[OG.OBJ_DEF]
                objModel = obj[OG.OBJ_MODEL]
                objProp = obj[OG.OBJ_PROP]
                objRGBA = obj[OG.OBJ_RGBA]
                self.editor.objectMgr.addNewObject(objDef.name,
                                                   uid,
                                                   obj[OG.OBJ_MODEL],
                                                   parentNP)
                self.editor.objectMgr.updateObjectColor(objRGBA[0], objRGBA[1], objRGBA[2], objRGBA[3], uid)
                self.editor.objectMgr.updateObjectProperties(uid, objProp)

            while (len(self.hierarchy.keys()) > 0):
                for uid in self.hierarchy.keys():
                    if self.hierarchy[uid] is None:
                        parentNP = None
                        restoreObject(uid, parentNP)
                        del self.hierarchy[uid]
                    else:
                        parentObj = self.editor.objectMgr.findObjectById(self.hierarchy[uid])
                        if parentObj:
                            parentNP = parentObj[OG.OBJ_NP]
                            restoreObject(uid, parentNP)
                            del self.hierarchy[uid]

            base.direct.deselectAll()
            for uid in self.selectedUIDs:
                obj = self.editor.objectMgr.findObjectById(uid)
                if obj:
                    base.direct.select(obj[OG.OBJ_NP], fMultiSelect=1)

            self.selecteUIDs = []
            self.hierarchy = {}
            self.objInfos = {}            
