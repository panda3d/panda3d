"""MultiTrack module: contains the MultiTrack class"""

import Interval
import Track

class MultiTrack(Interval.Interval):

    # special methods
    
    def __init__(self, name, trackList):
        """__init__(name, trackList)
        """
	self.name = name
	self.tlist = trackList
	self.getDuration()

    def getDuration(self):
	""" getDuration()
	"""
	if (len(self.tlist == 0):
	    Interval.notify.warning('MultiTrack.getDuration(): no Tracks')
	    return 0.0
	self.duration = self.tlist[0].getDuration()
	for t in self.tlist:
	    assert(self.duration = t.getDuration())
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
