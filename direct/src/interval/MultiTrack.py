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
	    self.name = 'MultiTrack-%d' % self.multiTrackNum
	    self.multiTrackNum = self.multiTrackNum + 1
	else:
	    self.name = name
	self.tlist = trackList
	self.getDuration()

    def getDuration(self):
	""" getDuration()
	"""
	#if (len(self.tlist == 0)):
	#    Interval.notify.warning('MultiTrack.getDuration(): no Tracks')
	#    return 0.0
	self.duration = self.tlist[0].getDuration()
	for t in self.tlist:
	    if (self.duration != t.getDuration()):
		Interval.Interval.notify.warning('MultiTrack.getDuration(): tracks not all same duration')
	return self.duration

    def setT(self, t):
	""" setT(t)
	    Go to time t
	"""
	if (t > self.duration):
	    Interval.notify.warning('MultiTrack.setT(): t = %f > duration' % t)
	    return
	for track in self.tlist:
	    track.setT(t)
