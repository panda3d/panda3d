"""Track module: contains the Track class"""

import Interval
import types

class Track(Interval.Interval):

    trackNum = 1

    # special methods
    
    def __init__(self, intervalList, name = None):
        """__init__(intervalList, name)
        """
	if (name == None):
	    self.name = 'Track-%d' % Track.trackNum
	    Track.trackNum = Track.trackNum + 1
	else:
	    self.name = name

	self.ilist = intervalList
	self.duration = self.__computeDuration(len(self.ilist))
	self.startTime = 0.0
	self.type = Interval.PREVIOUS_END

    def __computeDuration(self, length):
	""" __computeDuration(length)
	"""
	assert(length <= len(self.ilist))
	duration = 0.0
	prev = None
	for i in self.ilist[0:length]:
	    type = i.getType()
	    t0 = i.getStartTime()
	    assert(t0 >= 0.0)
	    fillTime = t0 
	    if (type == Interval.PREVIOUS_END):
		pass
	    elif (type == Interval.PREVIOUS_START):
		if (prev != None):
		    fillTime = t0 - prev.getDuration()
	    elif (type == Interval.TRACK_START):
		fillTime = t0 - duration
	    else:
		Interval.notify.error(
			'Track.__computeDuration(): unknown type: %d' % type)
	    if (fillTime < 0.0):
		Interval.notify.error(
			'Track.__computeDuration(): overlap detected')
	    duration = duration + fillTime + i.getDuration()
	    prev = i
	return duration

    def getIntervalStartTime(self, name):
	""" getIntervalStartTime(name)
	"""
	for i in range(len(self.ilist)):
	    if (self.ilist[i].getName() == name):	
		return self.__computeDuration(i+1) - self.ilist[i].getDuration()
	Interval.notify.warning(
		'Track.getIntervalStartTime(): no Interval named: %s' % name)
	return 0.0

    def __getIntervalStartTime(self, interval):
	""" __getIntervalStartTime(interval)
	"""
	return (self.__computeDuration(self.ilist.index(interval)+1) -
			interval.getDuration())

    def getIntervalEndTime(self, name):
	""" getIntervalEndTime(name)
	"""
	for i in range(len(self.ilist)):
	    if (self.ilist[i].getName() == name):	
		return self.__computeDuration(i+1)
	Interval.notify.warning(
		'Track.getIntervalEndTime(): no Interval named: %s' % name)
	return 0.0

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (len(self.ilist) == 0):
	    Interval.notify.warning('Track.setT(): track has no intervals')
	    return
	elif (t > self.duration):
	    # Anything beyond the end of the track is assumed to be the 
	    # final state of the last Interval on the track
	    self.ilist[len(self.ilist)-1].setT(t)
	else:
	    # Find out which Interval applies
	    prev = None
	    for i in self.ilist:
		# Calculate the track relative start time for the interval
		t0 = self.__getIntervalStartTime(i)

		# Determine if the Interval is applicable
		if (t < t0):
		    if (prev != None):
			# Gaps between Intervals take the final state of
			# the previous Interval
			prev.setT(t)
			return
		    else:
			#Interval.Interval.notify.warning(
			#	'Track.setT(): state undefined at t: %f' % t)
			return
		elif (t0 <= t) and (t <= t0 + i.getDuration()):
		    i.setT(t - t0)
		    return
		prev = i
