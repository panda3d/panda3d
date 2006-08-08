"""Undocumented Module"""

__all__ = ['Timer']

from pandac.PandaModules import *
import Task

class Timer:
    id = 0

    def __init__(self, name=None):
        self.finalT = 0.0
        self.currT = 0.0
        if (name == None):
            name = 'default-timer-%d' % Timer.id
            Timer.id += 1
        self.name = name
        self.started = 0
        self.callback = None

    def start(self, t, name):
        if (self.started):
            self.stop()
        self.callback = None
        self.finalT = t
        self.name = name
        self.startT = globalClock.getFrameTime()
        self.currT = 0.0
        taskMgr.add(self.__timerTask, self.name + '-run')
        self.started = 1

    def startCallback(self, t, callback):
        if (self.started):
            self.stop()
        self.callback = callback 
        self.finalT = t
        self.startT = globalClock.getFrameTime()
        self.currT = 0.0
        taskMgr.add(self.__timerTask, self.name + '-run')
        self.started = 1

    def stop(self):
        if (not self.started):
            return 0.0
        taskMgr.remove(self.name + '-run')
        self.started = 0
        return self.currT

    def resume(self):
        assert self.currT <= self.finalT
        assert self.started == 0
        self.start(self.finalT - self.currT, self.name)

    def restart(self):
        if (self.callback != None):
            self.startCallback(self.finalT, self.callback)
        else:
            self.start(self.finalT, self.name)

    def isStarted(self):
        return self.started

    def addT(self, t):
        self.finalT = self.finalT + t

    def setT(self, t):
        self.finalT = t

    def getT(self):
        return (self.finalT - self.currT)

    def __timerTask(self, task):
        t = globalClock.getFrameTime()
        te = t - self.startT 
        self.currT = te
        if (te >= self.finalT):
            if (self.callback != None):
                self.callback()
            else:
                messenger.send(self.name)
            return Task.done
        return Task.cont
