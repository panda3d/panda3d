"""MultiTrack module: contains the MultiTrack class"""

from Interval import *
from Track import *

class MultiTrack(Interval):
    # Name counter
    multiTrackNum = 1
    # Class methods
    def __init__(self, trackList, name=None):
        """__init__(trackList, name)
        """
        # Record track list
	self.tlist = trackList
	if (name == None):
	    name = 'MultiTrack-%d' % MultiTrack.multiTrackNum
	    MultiTrack.multiTrackNum = MultiTrack.multiTrackNum + 1
        # Duration is max of all track durations
	duration = self.__computeDuration()
        # Initialize superclass
	Interval.__init__(self, name, duration)
    
    # Access track at given index
    def __getitem__(self, item):
        return self.tlist[item]
    
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

    def updateFunc(self, t, event = IVAL_NONE):
	""" updateFunc(t, event)
	    Go to time t
	"""
	for track in self.tlist:
            tEnd = track.getDuration()
            # Compare time with track's end times
            if (t > tEnd):
                # If t > tEnd, only call if just crossing over
                # or this is an IVAL_INIT event
                if (self.prev_t < tEnd) or (event == IVAL_INIT):
                    track.setT(t, event)
            else:
                # Update track
                track.setT(t, event)

    # Print out representation of MultiTrack
    def __repr__(self, indent=0):
	""" __repr__(indent)
	"""
	str = Interval.__repr__(self, indent) + '\n'
	for t in self.tlist:
	    str = str + t.__repr__(indent+1)
        return str
