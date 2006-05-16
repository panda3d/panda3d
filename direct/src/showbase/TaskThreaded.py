from direct.task import Task

class TaskThreaded:
    """ derive from this if you need to do a bunch of CPU-intensive
    processing and you don't want to hang up the show. Lets you break
    up the processing over multiple frames """
    _Serial = SerialNum()
    
    def __init__(self, name, threaded=True):
        self._name = name
        self._threaded=threaded
        self._taskNames = set()

    def destroy(self):
        for taskName in self._taskNames:
            taskMgr.remove(taskName)

    def scheduleNext(self, callback):
        if not self._threaded:
            callback()
        else:
            taskName = ('%s-ThreadedTask-%s' %
                        (self._name, TaskThreaded._Serial.next()))
            assert taskName not in self._taskNames
            self._taskNames.add(taskName)
            taskMgr.add(Functor(self._doCallback, callback, taskName),
                        taskName)

    def _doCallback(self, callback, taskName, task):
        self._taskNames.remove(taskName)
        callback()
        return Task.done
