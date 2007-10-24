"""Undocumented Module"""

__all__ = ['DirectObject']


from MessengerGlobal import messenger
from direct.showbase.PythonUtil import ClassTree

class DirectObject:
    """
    This is the class that all Direct/SAL classes should inherit from
    """
    def __init__(self):
        pass

    #def __del__(self):
        # This next line is useful for debugging leaks
        #print "Destructing: ", self.__class__.__name__

    # Wrapper functions to have a cleaner, more object oriented approach to
    # the messenger functionality.

    def accept(self, event, method, extraArgs=[]):
        return messenger.accept(event, self, method, extraArgs, 1)

    def acceptOnce(self, event, method, extraArgs=[]):
        return messenger.accept(event, self, method, extraArgs, 0)

    def ignore(self, event):
        return messenger.ignore(event, self)

    def ignoreAll(self):
        return messenger.ignoreAll(self)

    def isAccepting(self, event):
        return messenger.isAccepting(event, self)

    def getAllAccepting(self):
        return messenger.getAllAccepting(self)

    def isIgnoring(self, event):
        return messenger.isIgnoring(event, self)

    def classTree(self):
        return ClassTree(self)

    #This function must be used if you want a managed task
    def addTask(self, *args, **kwargs):
        if(not hasattr(self,"_taskList")):
            self._taskList = {}
        kwargs['owner']=self
        task = taskMgr.add(*args, **kwargs)
        self._taskList[task.id] = task
        return task
    
    def doMethodLater(self, *args, **kwargs):
        if(not hasattr(self,"_taskList")):
            self._taskList ={}
        kwargs['owner']=self            
        task = taskMgr.doMethodLater(*args, **kwargs)
        self._taskList[task.id] = task
        return task
    
    def removeTask(self, taskOrName):
        if type(taskOrName) == type(''):
            # we must use a copy, since task.remove will modify self._taskList
            taskListValues = self._taskList.values()
            for task in taskListValues:
                if task.name == taskOrName:
                    task.remove()            
        else:
            taskOrName.remove()

    def removeAllTasks(self):
        if hasattr(self,'_taskList'):
            for task in self._taskList.values():
                task.remove()

    def _clearTask(self, task):
        del self._taskList[task.id]        
        
