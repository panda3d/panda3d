"""Track module: contains the Track class"""

import Interval

class Track(Interval.Interval):

    trackNum = 1

    # special methods
    
    def __init__(self, intervalList, name = None):
        """__init__(intervalList, name)
        """
	if (name == None):
	    self.name = 'Track-%d' % self.trackNum
	    self.trackNum = self.trackNum + 1
	else:
	    self.name = name
	self.ilist = intervalList
	self.dlist = []
	self.computeDuration()

    def computeDuration(self):
	""" computeDuration()
	"""
	self.duration = 0.0
	for i in self.ilist:
	    dur = i.getDuration()
	    self.duration = self.duration + dur
	    self.dlist.append(dur)

    def getStartTimeOf(self, name):
	""" getStartTimeOf(name)
	"""
	t = 0.0
	for i in range(len(self.ilist)):
	    if (self.ilist[i].getName() == name):
		return t 
	    t = t + self.dlist[i]
	Interval.notify.warning('Track.getStartOf(): no Interval named: %s' %
					name)
	return 0.0

    def getEndTimeOf(self, name):
	""" getEndTimeOf(name)
	"""
	t = 0.0
	for i in range(len(self.ilist)):
	    t = t + self.dlist[i]
	    if (self.ilist[i].getName() == name):
		return t 
	Interval.notify.warning('Track.getStartOf(): no Interval named: %s' %
					name)
	return 0.0

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    Interval.notify.warning('Track.setT(): t = %f > duration' % t)
	    return
	for i in range(len(self.dlist)):
	    if (t <= self.dlist[i]):
		self.ilist[i].setT(t)
