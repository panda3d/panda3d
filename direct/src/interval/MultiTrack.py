"""MultiTrack module: contains the MultiTrack class"""

import Interval
import Track

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
	self.type = Interval.Interval.PrevEndRelative

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
