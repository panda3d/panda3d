"""Track module: contains the Track class"""

import Interval
import types

PREVIOUS_END = 1
PREVIOUS_START = 2
TRACK_START = 3

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

	self.__buildIlist(intervalList)
	self.duration = self.__computeDuration(len(self.ilist))
	self.currentInterval = None

    def __buildIlist(self, intervalList):
	self.ilist = []
	for i in intervalList:
	    self.ilist.append((i, 0.0, PREVIOUS_END))

    def __computeDuration(self, length):
	""" __computeDuration(length)
	"""
	assert(length <= len(self.ilist))
	duration = 0.0
	prev = None
	for i in self.ilist[0:length]:
	    ival = i[0]
	    t0 = i[1]
	    type = i[2]
	    assert(t0 >= 0.0)
	    fillTime = t0 
	    if (type == PREVIOUS_END):
		pass
	    elif (type == PREVIOUS_START):
		if (prev != None):
		    fillTime = t0 - prev.getDuration()
	    elif (type == TRACK_START):
		fillTime = t0 - duration
	    else:
		Interval.notify.error(
			'Track.__computeDuration(): unknown type: %d' % type)
	    if (fillTime < 0.0):
		Interval.notify.error(
			'Track.__computeDuration(): overlap detected')
	    duration = duration + fillTime + ival.getDuration()
	    prev = ival
	return duration

    def setIntervalStartTime(self, name, t0, type=TRACK_START):
	""" setIntervalStartTime(name, t0, type)
	"""
	length = len(self.ilist)
	found = 0
	for i in range(length):
	    if (self.ilist[i][0].getName() == name):
		newi = (self.ilist[i][0], t0, type)	
		self.ilist[i] = newi
		found = 1
		break;
	if (found):
	    self.duration = self.__computeDuration(length)	
	else:
	    Interval.notify.warning(
		'Track.setIntervalStartTime(): no Interval named: %s' % name)

    def getIntervalStartTime(self, name):
	""" getIntervalStartTime(name)
	"""
	for i in range(len(self.ilist)):
	    if (self.ilist[i][0].getName() == name):	
		return (self.__computeDuration(i+1) - 
				self.ilist[i][0].getDuration())
	Interval.notify.warning(
		'Track.getIntervalStartTime(): no Interval named: %s' % name)
	return 0.0

    def __getIntervalStartTime(self, interval):
	""" __getIntervalStartTime(interval)
	"""
	for i in range(len(self.ilist)):
	    if (self.ilist[i][0] == interval):
		return (self.__computeDuration(i+1) - interval.getDuration())
	Interval.notify.warning(
		'Track.__getIntervalStartTime(): Interval not found')
	return 0.0

    def getIntervalEndTime(self, name):
	""" getIntervalEndTime(name)
	"""
	for i in range(len(self.ilist)):
	    if (self.ilist[i][0].getName() == name):	
		return self.__computeDuration(i+1)
	Interval.notify.warning(
		'Track.getIntervalEndTime(): no Interval named: %s' % name)
	return 0.0

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (entry == 1):
	    self.currentInterval = None
	if (len(self.ilist) == 0):
	    Interval.notify.warning('Track.setT(): track has no intervals')
	    return
	elif (entry == 1) and (t > self.duration):
	    # Anything beyond the end of the track is assumed to be the 
	    # final state of the last Interval on the track
	    self.ilist[len(self.ilist)-1][0].setT(t, entry=1)
	else:
	    # Find out which Interval applies
	    prev = None
	    for i in self.ilist:
		# Calculate the track relative start time for the interval
		ival = i[0]
		t0 = self.__getIntervalStartTime(ival)

		# Determine if the Interval is applicable
		if (t < t0):
		    if (prev != None):
			# Gaps between Intervals take the final state of
			# the previous Interval
			if (self.currentInterval != prev):
			    prev.setT(t, entry=1)
			    self.currentInterval = prev
			else:
			    prev.setT(t)
			return
		    else:
			#Interval.Interval.notify.warning(
			#	'Track.setT(): state undefined at t: %f' % t)
			return
		elif (t0 <= t) and (t <= t0 + ival.getDuration()):
		    if (self.currentInterval != ival):
		        ival.setT(t - t0, entry=1)
			self.currentInterval = ival
		    else:
		    	ival.setT(t - t0)
		    return
		prev = ival

    def printParams(self, indent=0):
	""" printParams(indent)
	"""
	Interval.Interval.printParams(self, indent)
	for i in self.ilist:	
	    i[0].printParams(indent+1)
