"""MultiTrack module: contains the MultiTrack class"""

from Interval import *
from Track import *

class MultiTrack(Interval):

    multiTrackNum = 1

    # special methods
    
    def __init__(self, trackList, name=None):
        """__init__(trackList, name)
        """
	if (name == None):
	    n = 'MultiTrack-%d' % MultiTrack.multiTrackNum
	    MultiTrack.multiTrackNum = MultiTrack.multiTrackNum + 1
	else:
	    n = name
	self.tlist = trackList
	duration = self.__computeDuration()
	Interval.__init__(self, n, duration)

    def __computeDuration(self):
	""" __computeDuration()
	    Returns the duration of the longest Track 
	"""
	duration = 0.0
	for t in self.tlist:
	    dur = t.getDuration()
	    if (dur > duration):
		duration = dur
	return duration

    def setT(self, t, entry=0):
	""" setT(t, entry)
	    Go to time t
	"""
	if (t > self.duration):
            pass
	for track in self.tlist:
	    track.setT(t, entry)

    def printParams(self, indent=0):
	""" printParams(indent)
	"""
	Interval.printParams(self, indent)
	for t in self.tlist:
	    t.printParams(indent+1)
