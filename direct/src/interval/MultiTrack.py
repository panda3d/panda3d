"""MultiTrack module: contains the MultiTrack class"""

import Interval
import Track
import ClockObject
import Task

class MultiTrack(Interval.Interval):

    multiTrackNum = 1

    # special methods
    
    def __init__(self, trackList, name = None):
        """__init__(trackList, name)
        """
	if (name == None):
	    self.name = 'MultiTrack-%d' % MultiTrack.multiTrackNum
	    MultiTrack.multiTrackNum = MultiTrack.multiTrackNum + 1
	else:
	    self.name = name
	self.tlist = trackList
	self.duration = self.getDuration()
	self.startTime = 0.0
	self.type = Interval.PREVIOUS_END
	self.clock = ClockObject.ClockObject.getGlobalClock()

    def getDuration(self):
	""" getDuration()
	    Returns the duration of the longest Track 
	"""
	duration = 0.0
	for t in self.tlist:
	    dur = t.getDuration()
	    if (dur > duration):
		duration = dur
	return duration

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    Interval.notify.warning(
		'MultiTrack.setT(): t = %f > duration' % t)
	for track in self.tlist:
	    track.setT(t)

    def play(self, t0=0.0, duration=0.0):
	""" play(t0, duration)
	"""
	self.startT = self.clock.getFrameTime() - t0
	if (duration == 0.0):
	    self.playDuration = self.duration
	else:
	    self.playDuration = duration
	taskMgr.spawnMethodNamed(self.__playTask, self.name + '-play')

    def stop(self):
	""" stop()
	"""
	taskMgr.removeTasksNamed(self.name + '-play')

    def __playTask(self, task):
	""" __playTask(task)
	"""
	t = self.clock.getFrameTime()
	te = t - self.startT
	if (te <= self.playDuration):
	    self.setT(te)
	    return Task.cont
	else:
	    return Task.done

    def printParams(self, indent=0):
	""" printParams(indent)
	"""
	Interval.Interval.printParams(self, indent)
	for t in self.tlist:
	    t.printParams(indent+1)
