import ClockObject
import Task

class Timer:

    def __init__(self):
	""" __init__()
	"""
	self.clock = ClockObject.ClockObject.getGlobalClock()
	self.finalT = 0.0
	self.currT = 0.0
	self.name = 'default-timer'
	self.started = 0

    def start(self, t, name):
	""" start(t, name)
	"""
	if (self.started):
	    self.stop()
	self.finalT = t
	self.name = name
	self.startT = self.clock.getFrameTime()
	self.currT = 0.0
	taskMgr.spawnMethodNamed(self.__timerTask, self.name + '-run')
	self.started = 1

    def stop(self):
	""" stop()
	"""
	if (not self.started):
	    return 0.0
	taskMgr.removeTasksNamed(self.name + '-run')
	self.started = 0
	return self.currT

    def resume(self):
	""" resume()
	"""
	assert(self.currT <= self.finalT)
	assert(self.started == 0)
	self.start(self.finalT - self.currT, self.name)

    def restart(self):
	""" restart()
	"""
	self.start(self.finalT, self.name)

    def isStarted(self):
	""" isStarted()
	"""
	return self.started

    def addT(self, t):
	""" addT(t)
	"""
	self.finalT = self.finalT + t

    def setT(self, t):
	""" setT(t)
	"""
	self.finalT = t

    def __timerTask(self, task):
	t = self.clock.getFrameTime()
 	te = t - self.startT 		
	self.currT = te
	if (te >= self.finalT):
	    messenger.send(self.name)
	    return Task.done
	return Task.cont
