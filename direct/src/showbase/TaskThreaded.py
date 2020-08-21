"""Contains the TaskThreaded and TaskThread classes."""

__all__ = ['TaskThreaded', 'TaskThread']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.task import Task

from .PythonUtil import SerialNumGen


class TaskThreaded:
    """ derive from this if you need to do a bunch of CPU-intensive
    processing and you don't want to hang up the show. Lets you break
    up the processing over multiple frames """
    notify = directNotify.newCategory("TaskThreaded")

    _Serial = SerialNumGen()

    def __init__(self, name, threaded=True, timeslice=None, callback=None):
        # timeslice is how long this thread should take every frame.
        self.__name = name
        self.__threaded=threaded
        if timeslice is None:
            timeslice = .01
        self.__timeslice = timeslice
        self.__taskNames = set()
        self._taskStartTime = None
        self.__threads = set()
        self._callback = callback

    def finished(self):
        if self._callback:
            self._callback()

    def destroy(self):
        for taskName in self.__taskNames:
            taskMgr.remove(taskName)
        del self.__taskNames
        for thread in self.__threads:
            thread.tearDown()
            thread._destroy()
        del self.__threads
        del self._callback
        self.ignoreAll()

    def getTimeslice(self):
        return self.___timeslice
    def setTimeslice(self, timeslice):
        self.__timeslice = timeslice

    def scheduleCallback(self, callback):
        assert self.notify.debugCall()
        if not self.__threaded:
            callback()
        else:
            taskName = ('%s-ThreadedTask-%s' %
                        (self.__name, TaskThreaded._Serial.next()))
            assert taskName not in self.__taskNames
            self.__taskNames.add(taskName)
            taskMgr.add(Functor(self.__doCallback, callback, taskName),
                        taskName)

    def scheduleThread(self, thread):
        assert self.notify.debugCall()
        # pass in a TaskThread. TaskThreaded will take over ownership and
        # cleanup responsibilities
        thread._init(self)
        thread.setUp()
        if thread.isFinished():
            thread._destroy()
        else:
            if not self.__threaded:
                while not thread.isFinished():
                    thread.run()
                thread._destroy()
            else:
                assert not thread in self.__threads
                self.__threads.add(thread)
                taskName = ('%s-ThreadedTask-%s-%s' %
                            (self.__name, thread.__class__.__name__,
                             TaskThreaded._Serial.next()))
                assert taskName not in self.__taskNames
                self.__taskNames.add(taskName)
                self.__threads.add(thread)
                taskMgr.add(Functor(self._doThreadCallback, thread, taskName),
                            taskName)

    def _doCallback(self, callback, taskName, task):
        assert self.notify.debugCall()
        self.__taskNames.remove(taskName)
        self._taskStartTime = globalClock.getRealTime()
        callback()
        self._taskStartTime = None
        return Task.done

    def _doThreadCallback(self, thread, taskName, task):
        assert self.notify.debugCall()
        self._taskStartTime = globalClock.getRealTime()
        thread.run()
        self._taskStartTime = None
        if thread.isFinished():
            thread._destroy()
            self.__taskNames.remove(taskName)
            self.__threads.remove(thread)
            return Task.done
        else:
            return Task.cont

    def taskTimeLeft(self):
        """returns True if there is time left for the current task callback
        to run without going over the allotted timeslice"""
        if self._taskStartTime is None:
            # we must not be in a task callback, we must be running in non-threaded
            # mode
            return True
        return (globalClock.getRealTime() - self._taskStartTime) < self.__timeslice

class TaskThread:
    # derive and override these four funcs
    # TaskThreaded obj is available as 'self.parent'
    # attributes of TaskThreaded obj are available directly as self.variable
    # call self.finished() when you're done
    def setUp(self):
        pass
    def run(self):
        pass
    def tearDown(self):
        # undo what you did in setUp()
        # this will be called if we get destroyed early
        pass
    def done(self):
        # override this if you want to do stuff after the thread finishes
        pass

    # call this when your task is complete
    def finished(self):
        self.tearDown()
        self._finished = True
        self.done()
    def isFinished(self):
        return self._finished

    # call this to find out if you've gone over your timeslice
    def timeLeft(self):
        return self.parent.taskTimeLeft()

    def _init(self, parent):
        self.parent = parent
        self._finished = False
    def _destroy(self):
        del self.parent
        del self._finished
