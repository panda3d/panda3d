"""Defines the DirectObject class, a convenient class to inherit from if the
object needs to be able to respond to events."""

__all__ = ['DirectObject']


from direct.directnotify.DirectNotifyGlobal import directNotify
from .MessengerGlobal import messenger

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

    #This function must be used if you want a managed task
    def addTask(self, *args, **kwargs):
        from direct.task.TaskManagerGlobal import taskMgr
        if not hasattr(self, "_taskList"):
            self._taskList = {}
        kwargs['owner'] = self
        task = taskMgr.add(*args, **kwargs)
        return task

    def doMethodLater(self, *args, **kwargs):
        from direct.task.TaskManagerGlobal import taskMgr
        if not hasattr(self, "_taskList"):
            self._taskList = {}
        kwargs['owner'] = self
        task = taskMgr.doMethodLater(*args, **kwargs)
        return task

    def removeTask(self, taskOrName):
        if type(taskOrName) == type(''):
            # we must use a copy, since task.remove will modify self._taskList
            if hasattr(self, '_taskList'):
                taskListValues = list(self._taskList.values())
                for task in taskListValues:
                    if task.name == taskOrName:
                        task.remove()
        else:
            taskOrName.remove()

    def removeAllTasks(self):
        if hasattr(self, '_taskList'):
            for task in list(self._taskList.values()):
                task.remove()

    def _addTask(self, task):
        self._taskList[task.id] = task

    def _clearTask(self, task):
        del self._taskList[task.id]

    def detectLeaks(self):
        if not __dev__:
            return

        # call this after the DirectObject instance has been destroyed
        # if it's leaking, will notify user

        # make sure we're not still listening for messenger events
        events = messenger.getAllAccepting(self)
        # make sure we're not leaking tasks
        # TODO: include tasks that were added directly to the taskMgr
        tasks = []
        if hasattr(self, '_taskList'):
            tasks = [task.name for task in self._taskList.values()]
        if len(events) or len(tasks):
            estr = ('listening to events: %s' % events if len(events) else '')
            andStr = (' and ' if len(events) and len(tasks) else '')
            tstr = ('%srunning tasks: %s' % (andStr, tasks) if len(tasks) else '')
            notify = directNotify.newCategory('LeakDetect')
            crash = getattr(getRepository(), '_crashOnProactiveLeakDetect', False)
            func = (self.notify.error if crash else self.notify.warning)
            func('destroyed %s instance is still %s%s' % (self.__class__.__name__, estr, tstr))

    #snake_case alias:
    add_task = addTask
    do_method_later = doMethodLater
    detect_leaks = detectLeaks
    accept_once = acceptOnce
    ignore_all = ignoreAll
    get_all_accepting = getAllAccepting
    is_ignoring = isIgnoring
    remove_all_tasks = removeAllTasks
    remove_task = removeTask
    is_accepting = isAccepting
