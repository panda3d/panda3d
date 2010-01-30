class ActionBase(Functor):
    """ Base class for user actions """

    def __init__(self, function, *args, **kargs):
        Functor.__init__(self, function, *args, **kargs)
        
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
    
    def __init__(self, function, *args, **kargs):
        ActionBase.__init__(self, function, *args, **kargs)
        self.np = None

    def _do__call__(self, *args, **kargs):
        self.np = ActionBase._do__call__(self, *args, **kargs)
        return self.np

    def undo(self):
        if self.np is None:
            print "Can't undo this"
        else:
            base.direct.removeAllSelected()
            base.le.objectMgr.deselectAll()
            base.direct.removeNodePath(self.np)
            self.np = None
